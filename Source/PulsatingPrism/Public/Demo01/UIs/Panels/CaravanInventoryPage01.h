// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "CaravanInventoryPage01.generated.h"

class UScrollBox;
class UTextBlock;
class UCaravanResourceEntryWidget01;
class AMoveableEntity01;

/**
 * 商队库存页面
 * 显示商队携带的所有资源
 */
UCLASS()
class PULSATINGPRISM_API UCaravanInventoryPage01 : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 初始化页面（通过实例）
	UFUNCTION(BlueprintCallable, Category = "Caravan Inventory")
	void InitializePage(AMoveableEntity01* InCaravan);
	
	// 初始化页面（通过数据，无实例）
	UFUNCTION(BlueprintCallable, Category = "Caravan Inventory")
	void InitializePageWithData(const FCaravanData01& InCaravanData);
	
	// 刷新库存列表
	UFUNCTION(BlueprintCallable, Category = "Caravan Inventory")
	void RefreshInventoryList();

	// 获取当前商队
	UFUNCTION(BlueprintPure, Category = "Caravan Inventory")
	AMoveableEntity01* GetCaravan() const { return Caravan; }

	// 设置城镇引用
	UFUNCTION(BlueprintCallable, Category = "Caravan Inventory")
	void SetTown(ATownActor01* InTown);

	// 更新交易权限
	UFUNCTION(BlueprintCallable, Category = "Caravan Inventory")
	void UpdateTradePermission();

	// 获取当前城镇
	UFUNCTION(BlueprintPure, Category = "Caravan Inventory")
	ATownActor01* GetCurrentTown() const { return CurrentTown; }

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 库存列表滚动框
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> InventoryListBox = nullptr;
	
	// 总重量文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalWeightText = nullptr;
	
	// 载重能力文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CarryCapacityText = nullptr;
	
	// 库存总价值文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalValueText = nullptr;

	// ===== Widget类引用 =====
	
	// 资源条目Widget类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Classes")
	TSubclassOf<UCaravanResourceEntryWidget01> ResourceEntryWidgetClass;

	// ===== 数据 =====
	
	// 关联的商队（可能为空，如果商队在城镇中）
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> Caravan = nullptr;
	
	// 商队数据（始终有效）
	UPROPERTY()
	FCaravanData01 CaravanData;

	// 当前关联的城镇（用于交易）
	UPROPERTY()
	TObjectPtr<ATownActor01> CurrentTown = nullptr;

	// 是否可以与城镇交易
	UPROPERTY()
	bool bCanTradeWithTown = false;

private:
	// 创建资源条目
	void CreateResourceEntries();
	
	// 更新载重显示
	void UpdateCapacityDisplay();
	
	// 更新总价值显示
	void UpdateTotalValueDisplay();
	
	// 计算库存总价值
	float CalculateTotalInventoryValue() const;
	
	// 计算当前总重量
	float CalculateCurrentWeight() const;
	
	// 获取生产管理器
	class UProductionManager01* GetProductionManager() const;
};