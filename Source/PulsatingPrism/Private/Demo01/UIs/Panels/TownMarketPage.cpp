// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/TownMarketPage.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/UIs/Widgets/ResourceEntryWidget01.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/MarketManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"

void UTownMarketPage::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定按钮事件
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UTownMarketPage::OnRefreshClicked);
	}

	// 设置页面标题
	if (PageTitleText)
	{
		PageTitleText->SetText(FText::FromString(TEXT("城镇市场")));
	}
	
	// 设置滚动条可见并始终显示（避免样式问题导致滚动条不显示）
	if (ResourceScrollBox)
	{
		ResourceScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
		ResourceScrollBox->SetAlwaysShowScrollbar(true);
	}
}

void UTownMarketPage::RefreshPage()
{
	Super::RefreshPage();
	
	// 刷新资源列表
	RefreshResourceList();
	
	// 刷新玩家资金
	RefreshPlayerMoney();
	
	// 更新统计信息
	UpdateStatistics();
}

void UTownMarketPage::SetTownInstance(ATownActor01* InTownInstance)
{
	if (TownInstance != InTownInstance)
	{
		// 城镇切换：清空Widget缓存，下次RefreshResourceList执行全量重建
		if (ResourceListBox)
		{
			ResourceListBox->ClearChildren();
		}
		ResourceWidgetCache.Empty();
	}
	Super::SetTownInstance(InTownInstance);
}

void UTownMarketPage::RefreshResourceList()
{
	if (!ResourceListBox || !TownInstance)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 无法刷新资源列表 - ResourceListBox: %s, TownInstance: %s"), 
		// 	ResourceListBox ? TEXT("有效") : TEXT("空"), 
		// 	TownInstance ? TEXT("有效") : TEXT("空"));
		return;
	}

	// 构建完整资源ID列表（与原逻辑相同）
	TArray<FString> ResourceIDs = TownInstance->GetAllResourceIDs();
	UProductionManager01* ProductionManager = GetProductionManager();
	if (ProductionManager)
	{
		TArray<FString> AllProductIDs = ProductionManager->GetAllProductIDs();
		for (const FString& ProductID : AllProductIDs)
		{
			if (!ResourceIDs.Contains(ProductID))
			{
				ResourceIDs.Add(ProductID);
			}
		}
	}

	if (ResourceWidgetCache.Num() > 0)
	{
		// 增量更新：直接刷新现有Widget数据，不销毁，避免调用方Widget失效
		for (const FString& ResourceID : ResourceIDs)
		{
			if (TObjectPtr<UResourceEntryWidget01>* Found = ResourceWidgetCache.Find(ResourceID))
			{
				UResourceEntryWidget01* Widget = *Found;
				Widget->SetCaravanID(CurrentCaravanID);
				Widget->SetTradePermission(bCanTradeWithCaravan);
				Widget->RefreshDisplay();
			}
			else
			{
				// 新增资源（运行时新出现）：创建并缓存
				UResourceEntryWidget01* Widget = CreateResourceEntryWidget(ResourceID);
				if (Widget)
				{
					ResourceListBox->AddChild(Widget);
						UE_LOG(LogTemp, Log, TEXT("[TownMarketPage] 为 %s 应用样式"), *Widget->GetName());
						IDemo01Styleable::Execute_ApplyWidgetStyle(Widget);
					Widget->InitializeResourceEntry(TownInstance, ResourceID);
					Widget->SetCaravanID(CurrentCaravanID);
					Widget->SetTradePermission(bCanTradeWithCaravan);
					Widget->OnTradeCompleted.AddDynamic(this, &UTownMarketPage::OnResourceTradeCompleted);
					ResourceWidgetCache.Add(ResourceID, Widget);
				}
				else
				{
					// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 创建资源条目Widget失败: %s"), *ResourceID);
				}
			}
		}
	}
	else
	{
		// 全量构建（首次加载或城镇切换后）
		ResourceListBox->ClearChildren();
		for (const FString& ResourceID : ResourceIDs)
		{
			UResourceEntryWidget01* Widget = CreateResourceEntryWidget(ResourceID);
			if (Widget)
			{
				ResourceListBox->AddChild(Widget);
					UE_LOG(LogTemp, Log, TEXT("[TownMarketPage] 为 %s 应用样式"), *Widget->GetName());
					IDemo01Styleable::Execute_ApplyWidgetStyle(Widget);
				Widget->InitializeResourceEntry(TownInstance, ResourceID);
				Widget->SetCaravanID(CurrentCaravanID);
				Widget->SetTradePermission(bCanTradeWithCaravan);
				Widget->OnTradeCompleted.AddDynamic(this, &UTownMarketPage::OnResourceTradeCompleted);
				ResourceWidgetCache.Add(ResourceID, Widget);
			}
			else
			{
				// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 创建资源条目Widget失败: %s"), *ResourceID);
			}
		}
	}

	// UE_LOG(LogTemp, Log, TEXT("[TownMarketPage] 资源列表已刷新，共 %d 个资源"), ResourceIDs.Num());
}

void UTownMarketPage::RefreshPlayerMoney()
{
	if (!PlayerMoneyText)
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;

	// 延迟订阅：第一次 RefreshPlayerMoney 时 PS 已就绪
	if (PS && !bGoldDelegateSubscribed)
	{
		PS->OnGoldChanged.AddDynamic(this, &UTownMarketPage::OnPlayerGoldChanged);
		bGoldDelegateSubscribed = true;
	}

	if (PS)
	{
		FText MoneyText = FText::Format(
			FText::FromString(TEXT("资金: {0}")),
			FText::AsNumber(PS->GetGold(), &FNumberFormattingOptions::DefaultWithGrouping())
		);
		PlayerMoneyText->SetText(MoneyText);
	}
	else
	{
		PlayerMoneyText->SetText(FText::FromString(TEXT("资金: N/A")));
	}
}

void UTownMarketPage::NativeDestruct()
{
	if (bGoldDelegateSubscribed)
	{
		UWorld* World = GetWorld();
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;
		if (PS)
		{
			PS->OnGoldChanged.RemoveDynamic(this, &UTownMarketPage::OnPlayerGoldChanged);
		}
		bGoldDelegateSubscribed = false;
	}
	Super::NativeDestruct();
}

void UTownMarketPage::OnPlayerGoldChanged(float NewGold)
{
	if (!PlayerMoneyText)
	{
		return;
	}
	FText MoneyText = FText::Format(
		FText::FromString(TEXT("资金: {0}")),
		FText::AsNumber(NewGold, &FNumberFormattingOptions::DefaultWithGrouping())
	);
	PlayerMoneyText->SetText(MoneyText);
}

void UTownMarketPage::OnRefreshClicked()
{
	RefreshPage();
	// UE_LOG(LogTemp, Log, TEXT("[TownMarketPage] 手动刷新页面"));
}

void UTownMarketPage::OnResourceTradeCompleted(const FString& ResourceID, int32 Quantity, float TotalValue, bool bWasPurchase)
{
	// 刷新相关显示
	RefreshPlayerMoney();
	UpdateStatistics();

	// UE_LOG(LogTemp, Log, TEXT("[TownMarketPage] 交易完成: %s %s %d 个，总价 %.2f"), 
	// 	*ResourceID, bWasPurchase ? TEXT("购买") : TEXT("出售"), Quantity, TotalValue);
}

UResourceEntryWidget01* UTownMarketPage::CreateResourceEntryWidget(const FString& ResourceID)
{
	if (!ResourceEntryWidgetClass)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] ResourceEntryWidgetClass 未设置"));
		return nullptr;
	}

	// 只创建widget，不在这里初始化（AddChild之前GetWorld()不稳定）
	UResourceEntryWidget01* ResourceEntryWidget = CreateWidget<UResourceEntryWidget01>(this, ResourceEntryWidgetClass);
	return ResourceEntryWidget;
}

void UTownMarketPage::UpdateStatistics()
{
	if (!InventoryValueText || !TownInstance)
	{
		return;
	}

	// 计算库存总价值
	float TotalValue = CalculateInventoryValue();

	// 更新显示文本
	FText ValueText = FText::Format(
		FText::FromString(TEXT("库存总价值: {0}")), 
		FText::AsNumber(TotalValue, &FNumberFormattingOptions::DefaultWithGrouping())
	);
	InventoryValueText->SetText(ValueText);
}

float UTownMarketPage::CalculateInventoryValue() const
{
	if (!TownInstance)
	{
		return 0.0f;
	}

	float TotalValue = 0.0f;
	TArray<FString> ResourceIDs = TownInstance->GetAllResourceIDs();
	UProductionManager01* ProductionManager = GetProductionManager();

	for (const FString& ResourceID : ResourceIDs)
	{
		int32 Quantity = TownInstance->GetResourceAmount(ResourceID);
		if (Quantity > 0 && ProductionManager)
		{
			FProductData01 ProductData = ProductionManager->GetProductData(ResourceID);
			TotalValue += ProductData.BaseValue * Quantity;
		}
	}

	return TotalValue;
}

UProductionManager01* UTownMarketPage::GetProductionManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetProductionManager() : nullptr;
}

UMarketManager01* UTownMarketPage::GetMarketManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetMarketManager() : nullptr;
}


void UTownMarketPage::SetCaravanID(int32 InCaravanID)
{
	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] ========== SetCaravanID =========="));
	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 设置CaravanID: %d -> %d"), CurrentCaravanID, InCaravanID);
	
	CurrentCaravanID = InCaravanID;
	UpdateTradePermission();
	
	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] ========== SetCaravanID 结束 =========="));
}

void UTownMarketPage::UpdateTradePermission()
{
	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] ========== UpdateTradePermission =========="));
	
	UWorld* World = GetWorld();
	if (!World || !TownInstance)
	{
		bCanTradeWithCaravan = false;
		// UE_LOG(LogTemp, Error, TEXT("[TownMarketPage] 交易权限更新: 禁止 (World或城镇为空)"));
		return;
	}
	
	APlayerController* PC = World->GetFirstPlayerController();
	ADemo01_PC* DemoPC = Cast<ADemo01_PC>(PC);
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;

	if (!DemoPC || !PS)
	{
		bCanTradeWithCaravan = false;
		// UE_LOG(LogTemp, Error, TEXT("[TownMarketPage] 交易权限更新: 禁止 (PC或PS为空)"));
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] CurrentCaravanID: %d"), CurrentCaravanID);
	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] TownInstance->TownData.EntityID: %d"), TownInstance->TownData.EntityID);

	bool bOldPermission = bCanTradeWithCaravan;
	bCanTradeWithCaravan = false;

	if (CurrentCaravanID >= 0)
	{
		// 条件1：该商队必须是玩家当前选中的商队
		const bool bIsSelected = (DemoPC->GetSelectedMoveableUnitID() == CurrentCaravanID)
			&& (DemoPC->GetSelectedMoveableUnitType() == EEntityType01::Caravan);

		if (!bIsSelected)
		{
			// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 交易权限更新: 禁止 (商队ID=%d 未被玩家选中，当前选中=%d)"),
			// 	CurrentCaravanID, DemoPC->GetSelectedMoveableUnitID());
		}
		else
		{
			FCaravanData01 CaravanData = PS->GetCaravanData(CurrentCaravanID);
			// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 商队数据：Name=%s, State=%d, CurrentTownID=%d"),
			// 	*CaravanData.EntityName, (int32)CaravanData.State, CaravanData.CurrentTownID);

			// 条件2：商队必须处于 InTown 状态（无实例，在城镇中）
			const bool bInTown = (CaravanData.State == ECaravanState01::InTown);

			// 条件3：商队所在城镇必须是当前打开的城镇
			const bool bInThisTown = (CaravanData.CurrentTownID == TownInstance->TownData.EntityID);

			bCanTradeWithCaravan = bIsSelected && bInTown && bInThisTown;

			// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 交易权限检查: 已选中=%s, InTown=%s, 在本城镇=%s => %s"),
			// 	bIsSelected ? TEXT("是") : TEXT("否"),
			// 	bInTown ? TEXT("是") : TEXT("否"),
			// 	bInThisTown ? TEXT("是") : TEXT("否"),
			// 	bCanTradeWithCaravan ? TEXT("允许") : TEXT("禁止"));
		}
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 交易权限更新: 禁止 (无商队选中)"));
	}

	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] 交易权限: %s -> %s"),
	// 	bOldPermission ? TEXT("允许") : TEXT("禁止"),
	// 	bCanTradeWithCaravan ? TEXT("允许") : TEXT("禁止"));

	// 更新所有ResourceEntryWidget的交易权限
	RefreshResourceList();
	
	// UE_LOG(LogTemp, Warning, TEXT("[TownMarketPage] ========== UpdateTradePermission 结束 =========="));
}