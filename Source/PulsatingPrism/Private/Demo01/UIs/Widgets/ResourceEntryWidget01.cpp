// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Widgets/ResourceEntryWidget01.h"

#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/MarketManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"

void UResourceEntryWidget01::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮事件
	if (BuyButton)
	{
		BuyButton->OnClicked.AddDynamic(this, &UResourceEntryWidget01::OnBuyClicked);
	}

	if (SellButton)
	{
		SellButton->OnClicked.AddDynamic(this, &UResourceEntryWidget01::OnSellClicked);
	}

	// 绑定输入框事件
	if (TradeQuantityInput)
	{
		TradeQuantityInput->OnTextChanged.AddDynamic(this, &UResourceEntryWidget01::OnTradeQuantityChanged);
		TradeQuantityInput->SetText(FText::AsNumber(TradeQuantity));
	}


}

void UResourceEntryWidget01::InitializeResourceEntry(ATownActor01* InTown, const FString& InResourceID)
{
	Town = InTown;
	ResourceID = InResourceID;
	TradeQuantity = 1;

	RefreshDisplay();
}

void UResourceEntryWidget01::RefreshDisplay()
{
	if (!Town || ResourceID.IsEmpty())
	{
		// UE_LOG(LogTemp, Error, TEXT("[ResourceEntry] RefreshDisplay 失败 - Town或ResourceID无效"));
		return;
	}

	// 获取产品数据（只调用一次）
	FProductData01 ProductData = GetProductData();

	// 更新资源名称
	if (ResourceNameText)
	{
		FText DisplayName = ProductData.ProductName.IsEmpty() ? 
			FText::FromString(ResourceID) : ProductData.ProductName;
		ResourceNameText->SetText(DisplayName);
	}

	// 更新库存数量
	if (QuantityText)
	{
		int32 CurrentQuantity = Town->GetResourceAmount(ResourceID);
		FText QuantityDisplayText = FText::Format(
			FText::FromString(TEXT("库存: {0}")), 
			FText::AsNumber(CurrentQuantity)
		);
		QuantityText->SetText(QuantityDisplayText);
	}

	// 更新价格显示（双价格：基础价格 + 市场价格）
	float BasePrice = ProductData.BaseValue;
	float MarketPrice = BasePrice; // 默认使用基础价格

	// 通过定价服务计算市场价格，并写入缓存供 CalculateTotalValue 复用
	UMarketManager01* MarketManager = GetMarketManager();
	if (MarketManager)
	{
		int32 TownStock = Town->GetResourceAmount(ResourceID);
		MarketPrice = MarketManager->CalculateMarketPrice(BasePrice, 100, TownStock, ProductData.Elasticity);
	}
	CachedMarketPrice = MarketPrice;

	// 显示基础价格
	if (BasePriceText)
	{
		FText BasePriceDisplayText = FText::Format(
			FText::FromString(TEXT("基础: {0}")), 
			FText::AsNumber(BasePrice, &FNumberFormattingOptions::DefaultWithGrouping())
		);
		BasePriceText->SetText(BasePriceDisplayText);
	}

	// 显示市场价格
	if (MarketPriceText)
	{
		// 计算价格变化百分比
		float PriceChangePercent = 0.0f;
		if (BasePrice > 0.0f)
		{
			PriceChangePercent = ((MarketPrice - BasePrice) / BasePrice) * 100.0f;
		}
		
		FString PriceChangeIndicator;
		if (PriceChangePercent > 0.1f)
		{
			PriceChangeIndicator = FString::Printf(TEXT(" ↗ (+%.1f%%)"), PriceChangePercent);
		}
		else if (PriceChangePercent < -0.1f)
		{
			PriceChangeIndicator = FString::Printf(TEXT(" ↘ (%.1f%%)"), PriceChangePercent);
		}
		else
		{
			PriceChangeIndicator = TEXT(" →");
		}
		
		FText MarketPriceDisplayText = FText::Format(
			FText::FromString(TEXT("市场: {0}{1}")), 
			FText::AsNumber(MarketPrice, &FNumberFormattingOptions::DefaultWithGrouping()),
			FText::FromString(PriceChangeIndicator)
		);
		MarketPriceText->SetText(MarketPriceDisplayText);
	}

	// 更新总价显示
	UpdateTotalValueDisplay();

	// 更新按钮状态
	UpdateButtonStates();
}

void UResourceEntryWidget01::OnBuyClicked()
{
	// 检查交易权限
	if (!CanTrade())
	{
		OnTradeFailed(TEXT("无法交易：商队不在此城镇中"));
		return;
	}
	
	if (!Town || CaravanID < 0 || TradeQuantity <= 0)
	{
		OnTradeFailed(TEXT("无效的交易数量"));
		return;
	}
	
	// 获取PlayerState
	UWorld* World = GetWorld();
	if (!World)
	{
		OnTradeFailed(TEXT("无法获取World"));
		return;
	}
	
	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	if (!GM)
	{
		OnTradeFailed(TEXT("无法获取GameMode"));
		return;
	}
	
	ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		OnTradeFailed(TEXT("无法获取PlayerState"));
		return;
	}
	
	// 检查城镇库存是否足够
	int32 TownStock = Town->GetResourceAmount(ResourceID);
	if (TownStock < TradeQuantity)
	{
		OnTradeFailed(TEXT("城镇库存不足"));
		return;
	}
	
	// 获取商队数据
	FCaravanData01 CaravanData = PS->GetCaravanData(CaravanID);
	if (CaravanData.EntityID != CaravanID)
	{
		OnTradeFailed(TEXT("无法找到商队数据"));
		return;
	}
	
	// 计算总价（使用市场价格）
	float TotalCost = CalculateTotalValue();

	// 检查并扣除玩家资金
	if (!PS->HasEnoughGold(TotalCost))
	{
		OnTradeFailed(TEXT("资金不足"));
		return;
	}
	PS->UpdateGold(-TotalCost);
	
	// 从城镇库存转移到商队库存
	if (Town->ConsumeResource(ResourceID, TradeQuantity))
	{
		// 更新商队数据
		CaravanData.Inventory.AddResource(ResourceID, TradeQuantity);
		PS->UpdateCaravanData(CaravanID, CaravanData);
		
		// 刷新显示
		RefreshDisplay();
		
		// 触发完成事件
		OnTradeCompleted.Broadcast(ResourceID, TradeQuantity, TotalCost, true);
		OnTradeCompletedBP(ResourceID, TradeQuantity, TotalCost, true);
		
		// 刷新商队面板库存显示
		RefreshCaravanPanelIfOpen();
	}
	else
	{
		OnTradeFailed(TEXT("城镇资源消耗失败"));
	}
}

void UResourceEntryWidget01::OnSellClicked()
{
	// 检查交易权限
	if (!CanTrade())
	{
		OnTradeFailed(TEXT("无法交易：商队不在此城镇中"));
		return;
	}
	
	if (!Town || CaravanID < 0 || TradeQuantity <= 0)
	{
		OnTradeFailed(TEXT("无效的交易数量"));
		return;
	}
	
	// 获取PlayerState
	UWorld* World = GetWorld();
	if (!World)
	{
		OnTradeFailed(TEXT("无法获取World"));
		return;
	}
	
	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	if (!GM)
	{
		OnTradeFailed(TEXT("无法获取GameMode"));
		return;
	}
	
	ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		OnTradeFailed(TEXT("无法获取PlayerState"));
		return;
	}
	
	// 获取商队数据
	FCaravanData01 CaravanData = PS->GetCaravanData(CaravanID);
	if (CaravanData.EntityID != CaravanID)
	{
		OnTradeFailed(TEXT("无法找到商队数据"));
		return;
	}
	
	// 检查商队库存是否足够
	int32 CaravanStock = CaravanData.Inventory.GetResourceAmount(ResourceID);
	if (CaravanStock < TradeQuantity)
	{
		OnTradeFailed(TEXT("商队库存不足"));
		return;
	}
	
	// 计算总价
	float TotalValue = CalculateTotalValue();
	
	// 从商队库存转移到城镇库存
	if (CaravanData.Inventory.ConsumeResource(ResourceID, TradeQuantity))
	{
		Town->AddResource(ResourceID, TradeQuantity);
		
		// 更新商队数据到PS
		PS->UpdateCaravanData(CaravanID, CaravanData);
		
		// 增加玩家资金
		PS->AddGold(TotalValue);
		
		// 刷新当前条目显示（更新城镇库存数量）
		RefreshDisplay();
		
		// 触发完成事件（通知TownMarketPage刷新资金和统计）
		OnTradeCompleted.Broadcast(ResourceID, TradeQuantity, TotalValue, false);
		OnTradeCompletedBP(ResourceID, TradeQuantity, TotalValue, false);
		
		// 刷新商队库存面板
		RefreshCaravanPanelIfOpen();
	}
	else
	{
		OnTradeFailed(TEXT("商队资源消耗失败"));
	}
}

void UResourceEntryWidget01::OnTradeQuantityChanged(const FText& Text)
{
	FString QuantityString = Text.ToString();
	int32 NewQuantity = FCString::Atoi(*QuantityString);
	
	// 确保数量为正数
	if (NewQuantity <= 0)
	{
		NewQuantity = 1;
		if (TradeQuantityInput)
		{
			TradeQuantityInput->SetText(FText::AsNumber(NewQuantity));
		}
	}
	
	TradeQuantity = NewQuantity;

	// 更新总价显示
	UpdateTotalValueDisplay();

	// 更新按钮状态
	UpdateButtonStates();
}

FProductData01 UResourceEntryWidget01::GetProductData() const
{
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		// 返回默认数据
		FProductData01 DefaultData;
		DefaultData.ProductID = ResourceID;
		DefaultData.ProductName = FText::FromString(ResourceID);
		DefaultData.BaseValue = 10; // 默认价格
		return DefaultData;
	}
	
	if (!ProductionManager->IsValidProduct(ResourceID))
	{
		// 返回默认数据
		FProductData01 DefaultData;
		DefaultData.ProductID = ResourceID;
		DefaultData.ProductName = FText::FromString(ResourceID);
		DefaultData.BaseValue = 10; // 默认价格
		return DefaultData;
	}

	return ProductionManager->GetProductData(ResourceID);
}

float UResourceEntryWidget01::CalculateTotalValue() const
{
	// 使用 RefreshDisplay 缓存的价格，避免重复调用定价服务
	float UnitPrice = CachedMarketPrice > 0.0f ? CachedMarketPrice : static_cast<float>(GetProductData().BaseValue);
	return UnitPrice * static_cast<float>(TradeQuantity);
}

void UResourceEntryWidget01::UpdateTotalValueDisplay()
{
	if (TotalValueText)
	{
		float TotalValue = CalculateTotalValue();
		
		FText TotalDisplayText = FText::Format(
			FText::FromString(TEXT("总价: {0}")), 
			FText::AsNumber(TotalValue, &FNumberFormattingOptions::DefaultWithGrouping())
		);
		
		TotalValueText->SetText(TotalDisplayText);
	}
}

void UResourceEntryWidget01::UpdateButtonStates()
{
	if (!bCanTradeWithCaravan)
	{
		// 没有交易权限时，禁用所有按钮
		if (BuyButton) BuyButton->SetIsEnabled(false);
		if (SellButton) SellButton->SetIsEnabled(false);
		return;
	}

	// 更新购买按钮状态
	if (BuyButton && Town)
	{
		int32 TownStock = Town->GetResourceAmount(ResourceID);
		bool bCanBuy = TownStock >= TradeQuantity; // 城镇库存足够才能购买
		BuyButton->SetIsEnabled(bCanBuy);
	}

	// 更新出售按钮状态
	if (SellButton && CaravanID >= 0)
	{
		// 获取商队库存
		UWorld* World = GetWorld();
		if (World)
		{
			ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
			if (PS)
			{
				FCaravanData01 CaravanData = PS->GetCaravanData(CaravanID);
				if (CaravanData.EntityID == CaravanID)
				{
					int32 CaravanStock = CaravanData.Inventory.GetResourceAmount(ResourceID);
					bool bCanSell = CaravanStock >= TradeQuantity; // 商队库存足够才能出售
					SellButton->SetIsEnabled(bCanSell);
				}
				else
				{
					SellButton->SetIsEnabled(false);
				}
			}
			else
			{
				SellButton->SetIsEnabled(false);
			}
		}
		else
		{
			SellButton->SetIsEnabled(false);
		}
	}
}

UProductionManager01* UResourceEntryWidget01::GetProductionManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetProductionManager() : nullptr;
}

UMarketManager01* UResourceEntryWidget01::GetMarketManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetMarketManager() : nullptr;
}

void UResourceEntryWidget01::RefreshCaravanPanelIfOpen()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	if (GM && GM->GetUIManager())
	{
		GM->GetUIManager()->RefreshCaravanInfoPanelIfOpen(CaravanID);
	}
}


void UResourceEntryWidget01::SetCaravanID(int32 InCaravanID)
{
	CaravanID = InCaravanID;
}

void UResourceEntryWidget01::SetTradePermission(bool bCanTrade)
{
	bCanTradeWithCaravan = bCanTrade;
	
	// 更新按钮状态
	UpdateButtonStates();
}


