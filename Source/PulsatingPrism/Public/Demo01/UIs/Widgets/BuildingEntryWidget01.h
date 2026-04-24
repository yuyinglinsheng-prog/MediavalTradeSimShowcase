// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "BuildingEntryWidget01.generated.h"

class UButton;
class UTextBlock;
class UImage;
class ATownActor01;

/**
 * 建筑条目Widget
 * 显示单个建筑的信息和操作按钮
 */
UCLASS()
class PULSATINGPRISM_API UBuildingEntryWidget01 : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 初始化建筑条目
	UFUNCTION(BlueprintCallable, Category = "Building Entry")
	void InitializeBuildingEntry(ATownActor01* InTown, const FTownBuildingData01& InBuildingData);

	// 禁用自动样式（动态创建时Slate层未就绪）
	virtual bool ShouldAutoApplyStyle_Implementation() const override { return false; }

	// 刷新显示数据
	UFUNCTION(BlueprintCallable, Category = "Building Entry")
	void RefreshDisplay();

	// 获取建筑ID
	UFUNCTION(BlueprintPure, Category = "Building Entry")
	int32 GetBuildingID() const { return BuildingData.BuildingID; }

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 建筑名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BuildingNameText = nullptr;

	// 配方名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RecipeNameText = nullptr;

	// 建筑等级文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText = nullptr;

	// 激活状态文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText = nullptr;

	// 激活/停用按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToggleActiveButton = nullptr;

	// 升级按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> UpgradeButton = nullptr;

	// 删除按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteButton = nullptr;

	// 更换配方按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChangeRecipeButton = nullptr;

	// ===== 事件处理 =====

	// 激活/停用按钮点击
	UFUNCTION()
	void OnToggleActiveClicked();

	// 升级按钮点击
	UFUNCTION()
	void OnUpgradeClicked();

	// 删除按钮点击
	UFUNCTION()
	void OnDeleteClicked();

	// 更换配方按钮点击
	UFUNCTION()
	void OnChangeRecipeClicked();

	// ===== 数据 =====

	// 关联的城镇
	UPROPERTY()
	TObjectPtr<ATownActor01> Town = nullptr;

	// 建筑数据
	UPROPERTY()
	FTownBuildingData01 BuildingData;

	// ===== 辅助方法 =====

	// 更新按钮文本和状态
	void UpdateButtonStates();

	// 获取配方显示名称
	FText GetRecipeDisplayName() const;

	// 获取生产管理器
	class UProductionManager01* GetProductionManager() const;

	// 通知父页面更新统计信息
	void NotifyParentPageUpdate();

public:
	// ===== 蓝图事件 =====

	// 建筑数据更新事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Building Entry")
	void OnBuildingDataUpdated();

	// 建筑降级通知事件（供蓝图重写，仅用于UI更新）
	UFUNCTION(BlueprintImplementableEvent, Category = "Building Entry")
	void OnBuildingDowngradeNotify(bool bWillDelete);

	// 更新删除按钮文本事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Building Entry")
	void OnUpdateDeleteButtonText(bool bWillDelete);

	// 建筑删除事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Building Entry")
	void OnBuildingDeleted();

	// 配方选择事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Building Entry")
	void OnRequestRecipeSelection();
};