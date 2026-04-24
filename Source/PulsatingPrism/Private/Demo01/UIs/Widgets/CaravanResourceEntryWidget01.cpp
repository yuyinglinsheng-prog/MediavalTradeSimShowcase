// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Widgets/CaravanResourceEntryWidget01.h"

#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCaravanResourceEntryWidget01::NativeConstruct()
{
	Super::NativeConstruct();
	

}

void UCaravanResourceEntryWidget01::InitializeEntryWithData(const FString& InResourceID, int32 InQuantity)
{
	Caravan = nullptr; // 不依赖实例
	ResourceID = InResourceID;
	Quantity = InQuantity;

	RefreshDisplay();
}

void UCaravanResourceEntryWidget01::InitializeEntry(AMoveableEntity01* InCaravan, const FString& InResourceID, int32 InQuantity)
{
	// 兼容性方法，实际上不需要 Caravan 实例
	Caravan = InCaravan; // 保存引用但不依赖
	ResourceID = InResourceID;
	Quantity = InQuantity;

	RefreshDisplay();
}

void UCaravanResourceEntryWidget01::RefreshDisplay()
{
	if (ResourceID.IsEmpty())
	{
		return;
	}

	// 获取产品数据
	FProductData01 ProductData = GetProductData();

	// 更新资源图标
	if (ResourceIcon)
	{
		if (ProductData.Icon && ProductData.Icon->IsValidLowLevel())
		{
			ResourceIcon->SetBrushFromTexture(ProductData.Icon);
		}
		else
		{
			// 如果图标无效，清除图标显示
			ResourceIcon->SetBrush(FSlateBrush());
		}
	}

	// 更新资源名称
	if (ResourceNameText)
	{
		FText DisplayName = ProductData.ProductName.IsEmpty() ? 
			FText::FromString(ResourceID) : ProductData.ProductName;
		ResourceNameText->SetText(DisplayName);
	}

	// 更新数量
	if (QuantityText)
	{
		FText QuantityDisplayText = FText::Format(
			FText::FromString(TEXT("数量: {0}")), 
			FText::AsNumber(Quantity)
		);
		QuantityText->SetText(QuantityDisplayText);
	}

	// 更新单位重量
	if (WeightText)
	{
		float TotalWeight = CalculateTotalWeight();
		FText WeightDisplayText = FText::Format(
			FText::FromString(TEXT("重量: {0} kg")), 
			FText::AsNumber(TotalWeight, &FNumberFormattingOptions::DefaultWithGrouping())
		);
		WeightText->SetText(WeightDisplayText);
	}

	// 更新单价
	if (UnitValueText)
	{
		FText UnitValueDisplayText = FText::Format(
			FText::FromString(TEXT("单价: {0}")), 
			FText::AsNumber(ProductData.BaseValue, &FNumberFormattingOptions::DefaultWithGrouping())
		);
		UnitValueText->SetText(UnitValueDisplayText);
	}

	// 更新总价值
	if (TotalValueText)
	{
		float TotalValue = CalculateTotalValue();
		FText TotalValueDisplayText = FText::Format(
			FText::FromString(TEXT("总值: {0}")), 
			FText::AsNumber(TotalValue, &FNumberFormattingOptions::DefaultWithGrouping())
		);
		TotalValueText->SetText(TotalValueDisplayText);
	}
}

FProductData01 UCaravanResourceEntryWidget01::GetProductData() const
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

UProductionManager01* UCaravanResourceEntryWidget01::GetProductionManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetProductionManager() : nullptr;
}

float UCaravanResourceEntryWidget01::CalculateTotalWeight() const
{
	// 简化处理：假设每个资源单位重量为1kg
	// 未来可以从ProductData中获取实际重量数据
	return static_cast<float>(Quantity);
}

float UCaravanResourceEntryWidget01::CalculateTotalValue() const
{
	FProductData01 ProductData = GetProductData();
	return ProductData.BaseValue * Quantity;
}
