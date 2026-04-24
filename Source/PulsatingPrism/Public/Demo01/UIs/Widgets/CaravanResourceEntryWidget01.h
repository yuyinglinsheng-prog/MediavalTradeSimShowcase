// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "CaravanResourceEntryWidget01.generated.h"

class UTextBlock;
class UImage;
class AMoveableEntity01;

/**
 * 商队资源条目Widget
 * 显示商队携带的单个资源信息
 */
UCLASS()
class PULSATINGPRISM_API UCaravanResourceEntryWidget01 : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 初始化资源条目（推荐使用，基于数据）
	UFUNCTION(BlueprintCallable, Category = "Caravan Resource")
	void InitializeEntryWithData(const FString& InResourceID, int32 InQuantity);

	// 禁用自动样式（动态创建时Slate层未就绪）
	virtual bool ShouldAutoApplyStyle_Implementation() const override { return false; }

	// 兼容性方法（保持向后兼容）
	UFUNCTION(BlueprintCallable, Category = "Caravan Resource")
	void InitializeEntry(AMoveableEntity01* InCaravan, const FString& InResourceID, int32 InQuantity);

	// 刷新显示
	UFUNCTION(BlueprintCallable, Category = "Caravan Resource")
	void RefreshDisplay();

	// 获取资源ID
	UFUNCTION(BlueprintPure, Category = "Caravan Resource")
	FString GetResourceID() const { return ResourceID; }

	// 获取数量
	UFUNCTION(BlueprintPure, Category = "Caravan Resource")
	int32 GetQuantity() const { return Quantity; }

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 资源名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResourceNameText = nullptr;
	
	// 数量文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText = nullptr;
	
	// 单位重量文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeightText = nullptr;
	
	// 单价文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> UnitValueText = nullptr;
	
	// 总价值文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalValueText = nullptr;
	
	// 资源图标（可选）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ResourceIcon = nullptr;

	// ===== 数据 =====
	
	// 关联的商队
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> Caravan = nullptr;
	
	// 资源ID
	UPROPERTY()
	FString ResourceID;
	
	// 数量
	UPROPERTY()
	int32 Quantity = 0;

private:
	// 获取产品数据
	FProductData01 GetProductData() const;
	
	// 获取生产管理器
	class UProductionManager01* GetProductionManager() const;
	
	// 计算总重量
	float CalculateTotalWeight() const;
	
	// 计算总价值
	float CalculateTotalValue() const;

public:
	// ===== 蓝图事件 =====
	
	// 资源条目点击事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Caravan Resource")
	void OnResourceEntryClicked();
};