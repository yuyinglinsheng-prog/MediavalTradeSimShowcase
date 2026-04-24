// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/MarketManager01.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/ProductionManager01.h"

float UMarketManager01::CalculateMarketPrice(float BasePrice, int32 BaseQuantity, int32 CurrentQuantity, float Elasticity) const
{
	if (BasePrice <= 0.0f)
	{
		return 0.0f;
	}

	// 幂律定价：Price = BasePrice × clamp((BaseQty / CurrentQty)^Elasticity, 0.25, 4.0)
	const float SafeCurrentQty = FMath::Max(1, CurrentQuantity);
	const float Ratio = static_cast<float>(BaseQuantity) / SafeCurrentQty;
	const float PriceFactor = FMath::Clamp(FMath::Pow(Ratio, Elasticity), 0.25f, 4.0f);
	const float FinalPrice = BasePrice * PriceFactor;

	UE_LOG(LogTemp, Verbose, TEXT("[MarketManager] Price: Base=%.1f Qty=%d/%d Elasticity=%.2f -> Factor=%.3f Price=%.1f"),
		BasePrice, CurrentQuantity, BaseQuantity, Elasticity, PriceFactor, FinalPrice);

	return FinalPrice;
}

// ==============================================
// 贸易优化计算
// ==============================================

FTradePlan UMarketManager01::CalculateOptimalTrade(int32 TownID, int32 CaravanID, float MinPriceRatio) const
{
	FTradePlan Plan;
	Plan.bIsExecutable = false;
	
	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		Plan.FailureReason = TEXT("GameMode未找到");
		return Plan;
	}
	
	ADemo01_PS* PlayerState = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PlayerState)
	{
		Plan.FailureReason = TEXT("PlayerState未找到");
		return Plan;
	}
	
	// 获取城镇和商队数据
	FTownData01* TownData = PlayerState->GetTownDataPtr(TownID);
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	
	if (!TownData || !CaravanData)
	{
		Plan.FailureReason = TEXT("城镇或商队数据不存在");
		return Plan;
	}
	
	// 获取生产管理器
	UProductionManager01* ProductionManager = GameMode->GetProductionManager();
	if (!ProductionManager || !ProductionManager->IsInitialized())
	{
		Plan.FailureReason = TEXT("生产管理器未初始化");
		return Plan;
	}
	
	// 遍历城镇所有商品
	float TotalMoneyChange = 0.0f;
	
	for (const auto& ResourcePair : TownData->Inventory.Resources)
	{
		const FString& ResourceID = ResourcePair.Key;
		int32 AvailableQuantity = ResourcePair.Value;
		
		if (AvailableQuantity <= 0) continue;
		
		// 获取产品数据
		FProductData01 ProductData = ProductionManager->GetProductData(ResourceID);
		if (ProductData.ProductID.IsEmpty()) continue;
		
		// 计算当前价格和偏差比例
		float CurrentPrice = GetCurrentPrice(ResourceID, TownID);
		float BasePrice = static_cast<float>(ProductData.BaseValue);
		
		if (BasePrice <= 0.0f) continue;
		
		float PriceRatio = (CurrentPrice - BasePrice) / BasePrice;
		
		// 检查是否满足阈值
		if (FMath::Abs(PriceRatio) < MinPriceRatio) continue;
		
		if (PriceRatio < 0) // 折扣 -> 买入
		{
			// 计算可购买数量（受限于金币）
			float PlayerGold = PlayerState->GetGold();
			int32 MaxAffordable = PlayerGold > 0 ? FMath::FloorToInt(PlayerGold / CurrentPrice) : 0;
			int32 BuyAmount = FMath::Min(AvailableQuantity, MaxAffordable);
			
			if (BuyAmount > 0)
			{
				Plan.BuyPlan.Add(ResourceID, BuyAmount);
				TotalMoneyChange -= BuyAmount * CurrentPrice;
			}
		}
		else // 溢价 -> 卖出
		{
			// 检查商队是否有库存
			int32 CaravanQuantity = CaravanData->Inventory.GetResourceAmount(ResourceID);
			
			if (CaravanQuantity > 0)
			{
				Plan.SellPlan.Add(ResourceID, CaravanQuantity);
				TotalMoneyChange += CaravanQuantity * CurrentPrice;
			}
		}
	}
	
	// 4. 验证计划是否可执行
	if (Plan.BuyPlan.Num() > 0 || Plan.SellPlan.Num() > 0)
	{
		Plan.bIsExecutable = true;
		Plan.MoneyChange = TotalMoneyChange;
	}
	else
	{
		Plan.FailureReason = TEXT("没有满足价格偏差阈值的商品");
	}
	
	return Plan;
}

float UMarketManager01::ExecutePurchase(int32 TownID, int32 CaravanID, const FString& ResourceID, int32 Amount)
{
	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return 0.0f;
	
	ADemo01_PS* PlayerState = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PlayerState) return 0.0f;
	
	FTownData01* TownData = PlayerState->GetTownDataPtr(TownID);
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	
	if (!TownData || !CaravanData) return 0.0f;
	
	// 检查城镇库存
	int32 TownQuantity = TownData->Inventory.GetResourceAmount(ResourceID);
	if (TownQuantity < Amount) return 0.0f;
	
	// 计算价格
	float Price = GetCurrentPrice(ResourceID, TownID);
	float TotalCost = Price * Amount;
	
	// 检查金币
	if (PlayerState->GetGold() < TotalCost) return 0.0f;
	
	// 执行交易
	TownData->Inventory.ConsumeResource(ResourceID, Amount);
	CaravanData->Inventory.AddResource(ResourceID, Amount);
	PlayerState->UpdateGold(-TotalCost);
	
	UE_LOG(LogTemp, Log, TEXT("[市场] 商队 %d 在城镇 %d 购买 %d 个 %s，花费 %.2f 金币"),
		CaravanID, TownID, Amount, *ResourceID, TotalCost);
	
	return TotalCost;
}

float UMarketManager01::ExecuteSale(int32 TownID, int32 CaravanID, const FString& ResourceID, int32 Amount)
{
	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return 0.0f;
	
	ADemo01_PS* PlayerState = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PlayerState) return 0.0f;
	
	FTownData01* TownData = PlayerState->GetTownDataPtr(TownID);
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	
	if (!TownData || !CaravanData) return 0.0f;
	
	// 检查商队库存
	int32 CaravanQuantity = CaravanData->Inventory.GetResourceAmount(ResourceID);
	if (CaravanQuantity < Amount) return 0.0f;
	
	// 计算价格
	float Price = GetCurrentPrice(ResourceID, TownID);
	float TotalRevenue = Price * Amount;
	
	// 执行交易
	CaravanData->Inventory.ConsumeResource(ResourceID, Amount);
	TownData->Inventory.AddResource(ResourceID, Amount);
	PlayerState->UpdateGold(TotalRevenue);
	
	UE_LOG(LogTemp, Log, TEXT("[市场] 商队 %d 在城镇 %d 卖出 %d 个 %s，获得 %.2f 金币"),
		CaravanID, TownID, Amount, *ResourceID, TotalRevenue);
	
	return TotalRevenue;
}

float UMarketManager01::GetCurrentPrice(const FString& ResourceID, int32 TownID) const
{
	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return 0.0f;
	
	ADemo01_PS* PlayerState = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PlayerState) return 0.0f;
	
	// 获取城镇数据
	FTownData01* TownData = PlayerState->GetTownDataPtr(TownID);
	if (!TownData) return 0.0f;
	
	// 获取商品基础价格
	float BasePrice = GetProductBasePrice(ResourceID);
	
	// 获取商品数据（用于弹性系数）
	UProductionManager01* ProductionManager = GameMode->GetProductionManager();
	if (!ProductionManager || !ProductionManager->IsInitialized()) return BasePrice;
	
	FProductData01 ProductData = ProductionManager->GetProductData(ResourceID);
	if (ProductData.ProductID.IsEmpty()) return BasePrice;
	
	// 获取城镇当前库存
	int32 TownStock = TownData->Inventory.GetResourceAmount(ResourceID);
	
	// 使用CalculateMarketPrice计算实际价格（基准数量使用100，与UI保持一致）
	return CalculateMarketPrice(BasePrice, 100, TownStock, ProductData.Elasticity);
}

float UMarketManager01::GetProductBasePrice(const FString& ResourceID) const
{
	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return 0.0f;
	
	UProductionManager01* ProductionManager = GameMode->GetProductionManager();
	if (!ProductionManager || !ProductionManager->IsInitialized()) return 0.0f;
	
	FProductData01 ProductData = ProductionManager->GetProductData(ResourceID);
	return static_cast<float>(ProductData.BaseValue);
}

float UMarketManager01::GetProductWeight(const FString& ResourceID) const
{
	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return 1.0f;
	
	UProductionManager01* ProductionManager = GameMode->GetProductionManager();
	if (!ProductionManager || !ProductionManager->IsInitialized()) return 1.0f;
	
	FProductData01 ProductData = ProductionManager->GetProductData(ResourceID);
	return ProductData.WeightPerUnit > 0 ? ProductData.WeightPerUnit : 1.0f;
}

