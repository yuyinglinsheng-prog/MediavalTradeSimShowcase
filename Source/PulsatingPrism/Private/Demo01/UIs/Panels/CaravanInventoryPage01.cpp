// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/CaravanInventoryPage01.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/UIs/Widgets/CaravanResourceEntryWidget01.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UCaravanInventoryPage01::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置滚动条可见并始终显示（避免样式问题导致滚动条不显示）
	if (InventoryListBox)
	{
		InventoryListBox->SetScrollBarVisibility(ESlateVisibility::Visible);
		InventoryListBox->SetAlwaysShowScrollbar(true);
	}
}

void UCaravanInventoryPage01::InitializePage(AMoveableEntity01* InCaravan)
{
	if (!InCaravan)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInventoryPage] InitializePage: 商队为空"));
		return;
	}

	Caravan = InCaravan;
	if (GameMode)
	{
		ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (FCaravanData01* LiveData = PS ? PS->GetCaravanDataPtr(InCaravan->EntityID) : nullptr)
		{
			CaravanData = *LiveData;
		}
	}
	
	// 刷新库存列表
	RefreshInventoryList();
}

void UCaravanInventoryPage01::InitializePageWithData(const FCaravanData01& InCaravanData)
{
	// 无实例时，只使用数据
	Caravan = nullptr;
	CaravanData = InCaravanData;
	
	UE_LOG(LogTemp, Log, TEXT("[CaravanInventoryPage] 使用数据初始化（无实例）：%s"), *CaravanData.EntityName);
	
	// 刷新库存列表
	RefreshInventoryList();
}

void UCaravanInventoryPage01::RefreshInventoryList()
{
	// 始终从PS读最新数据，不依赖Actor实例（避免本地副本覆盖PS权威数据）
	ADemo01_PS* PS = nullptr;
	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (PC)
	{
		PS = PC->GetPlayerState<ADemo01_PS>();
	}
	if (PS && CaravanData.EntityID > 0)
	{
		CaravanData = PS->GetCaravanData(CaravanData.EntityID);
	}

	// 创建资源条目
	CreateResourceEntries();
	
	// 更新载重显示
	UpdateCapacityDisplay();
	
	// 更新总价值显示
	UpdateTotalValueDisplay();
}

void UCaravanInventoryPage01::CreateResourceEntries()
{
	if (!InventoryListBox)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInventoryPage] InventoryListBox 组件未绑定"));
		return;
	}

	if (!ResourceEntryWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInventoryPage] ResourceEntryWidgetClass 未设置"));
		return;
	}

	// 清空现有条目
	InventoryListBox->ClearChildren();

	// 获取商队库存（使用数据）
	const FTownInventoryData01& Inventory = CaravanData.Inventory;
	
	if (Inventory.Resources.Num() == 0)
	{
		// 显示空库存提示
		// 这里可以添加一个提示文本Widget
		return;
	}

	// 为每个资源创建条目
	for (const auto& ResourcePair : Inventory.Resources)
	{
		const FString& ResourceID = ResourcePair.Key;
		int32 Quantity = ResourcePair.Value;

		if (Quantity <= 0)
		{
			continue; // 跳过数量为0的资源
		}

		// 创建资源条目Widget
		UCaravanResourceEntryWidget01* ResourceEntry = CreateWidget<UCaravanResourceEntryWidget01>(
			this, ResourceEntryWidgetClass);

		if (ResourceEntry)
		{
			// 1. AddChild，Slate层就绪
			InventoryListBox->AddChild(ResourceEntry);
			// 2. 应用样式
			UE_LOG(LogTemp, Log, TEXT("[CaravanInventoryPage] 为 %s 应用样式"), *ResourceEntry->GetName());
			IDemo01Styleable::Execute_ApplyWidgetStyle(ResourceEntry);
			// 3. 初始化数据
			ResourceEntry->InitializeEntryWithData(ResourceID, Quantity);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[CaravanInventoryPage] 无法创建资源条目Widget: %s"), *ResourceID);
		}
	}
}

void UCaravanInventoryPage01::UpdateCapacityDisplay()
{
	// 计算当前重量
	float CurrentWeight = CalculateCurrentWeight();
	float MaxCapacity = CaravanData.CarryCapacity;

	// 更新总重量显示
	if (TotalWeightText)
	{
		FText WeightText = FText::Format(
			FText::FromString(TEXT("当前重量: {0} / {1}")),
			FText::AsNumber(CurrentWeight, &FNumberFormattingOptions::DefaultWithGrouping()),
			FText::AsNumber(MaxCapacity, &FNumberFormattingOptions::DefaultWithGrouping())
		);
		TotalWeightText->SetText(WeightText);
	}

	// 更新载重能力显示
	if (CarryCapacityText)
	{
		float UsagePercent = MaxCapacity > 0 ? (CurrentWeight / MaxCapacity) * 100.0f : 0.0f;
		FText CapacityText = FText::Format(
			FText::FromString(TEXT("载重使用率: {0}%")),
			FText::AsNumber(UsagePercent, &FNumberFormattingOptions::DefaultNoGrouping())
		);
		CarryCapacityText->SetText(CapacityText);
	}
}

void UCaravanInventoryPage01::UpdateTotalValueDisplay()
{
	if (!TotalValueText)
	{
		return;
	}

	float TotalValue = CalculateTotalInventoryValue();
	
	FText ValueText = FText::Format(
		FText::FromString(TEXT("库存总价值: {0}")),
		FText::AsNumber(TotalValue, &FNumberFormattingOptions::DefaultWithGrouping())
	);
	
	TotalValueText->SetText(ValueText);
}

float UCaravanInventoryPage01::CalculateTotalInventoryValue() const
{
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		return 0.0f;
	}

	float TotalValue = 0.0f;
	const FTownInventoryData01& Inventory = CaravanData.Inventory;

	for (const auto& ResourcePair : Inventory.Resources)
	{
		const FString& ResourceID = ResourcePair.Key;
		int32 Quantity = ResourcePair.Value;

		if (Quantity <= 0 || !ProductionManager->IsValidProduct(ResourceID))
		{
			continue;
		}

		FProductData01 ProductData = ProductionManager->GetProductData(ResourceID);
		TotalValue += ProductData.BaseValue * Quantity;
	}

	return TotalValue;
}

float UCaravanInventoryPage01::CalculateCurrentWeight() const
{
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		return 0.0f;
	}

	float TotalWeight = 0.0f;
	const FTownInventoryData01& Inventory = CaravanData.Inventory;

	for (const auto& ResourcePair : Inventory.Resources)
	{
		const FString& ResourceID = ResourcePair.Key;
		int32 Quantity = ResourcePair.Value;

		if (Quantity <= 0 || !ProductionManager->IsValidProduct(ResourceID))
		{
			continue;
		}

		// 简化处理：假设每个资源单位重量为1.0f
		// 实际项目中可以在ProductData中添加Weight字段
		TotalWeight += 1.0f * Quantity;
	}

	return TotalWeight;
}

UProductionManager01* UCaravanInventoryPage01::GetProductionManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetProductionManager() : nullptr;
}


void UCaravanInventoryPage01::SetTown(ATownActor01* InTown)
{
	CurrentTown = InTown;
	UpdateTradePermission();
}

void UCaravanInventoryPage01::UpdateTradePermission()
{
	bCanTradeWithTown = false;

	if (!CurrentTown)
	{
		UE_LOG(LogTemp, Log, TEXT("[CaravanInventoryPage] 交易权限更新: 禁止 (城镇为空)"));
		RefreshInventoryList();
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	ADemo01_PC* DemoPC = Cast<ADemo01_PC>(PC);
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;

	if (!DemoPC || !PS)
	{
		UE_LOG(LogTemp, Log, TEXT("[CaravanInventoryPage] 交易权限更新: 禁止 (PC或PS为空)"));
		RefreshInventoryList();
		return;
	}

	// 从PS重新读取最新数据，不用本地缓存
	const FCaravanData01 FreshData = PS->GetCaravanData(CaravanData.EntityID);

	// 条件1：该商队必须是玩家当前选中的商队
	const bool bIsSelected = (DemoPC->GetSelectedMoveableUnitID() == FreshData.EntityID)
		&& (DemoPC->GetSelectedMoveableUnitType() == EEntityType01::Caravan);

	// 条件2：商队必须处于 InTown 状态
	const bool bInTown = (FreshData.State == ECaravanState01::InTown);

	// 条件3：商队所在城镇必须是当前打开的城镇
	const bool bInThisTown = (FreshData.CurrentTownID == CurrentTown->TownData.EntityID);

	bCanTradeWithTown = bIsSelected && bInTown && bInThisTown;

	UE_LOG(LogTemp, Log, TEXT("[CaravanInventoryPage] 交易权限检查: 已选中=%s, InTown=%s, 在本城镇=%s => %s"),
		bIsSelected ? TEXT("是") : TEXT("否"),
		bInTown ? TEXT("是") : TEXT("否"),
		bInThisTown ? TEXT("是") : TEXT("否"),
		bCanTradeWithTown ? TEXT("允许") : TEXT("禁止"));

	RefreshInventoryList();
}
