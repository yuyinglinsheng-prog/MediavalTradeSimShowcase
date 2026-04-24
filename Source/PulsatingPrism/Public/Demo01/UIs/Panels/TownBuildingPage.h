// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Panels/TownPageBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "TownBuildingPage.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UVerticalBox;
class UBuildingEntryWidget01;
class URecipeSelectionWidget01;

/**
 * 城镇建设页面
 * 显示城镇建筑和生产设施
 */
UCLASS()
class PULSATINGPRISM_API UTownBuildingPage : public UTownPageBase
{
	GENERATED_BODY()

public:
	// 重写基类方法
	virtual void RefreshPage() override;

	// 刷新建筑列表
	UFUNCTION(BlueprintCallable, Category = "Town Building")
	void RefreshBuildingList();

	// 添加新建筑
	UFUNCTION(BlueprintCallable, Category = "Town Building")
	void ShowAddBuildingDialog();

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====

	// 页面标题
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PageTitleText = nullptr;

	// 建筑数量统计
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BuildingCountText = nullptr;

	// 添加建筑按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AddBuildingButton = nullptr;

	// 建筑列表滚动框
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> BuildingScrollBox = nullptr;

	// 建筑列表容器
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> BuildingListBox = nullptr;

	// 配方选择Widget（弹窗）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URecipeSelectionWidget01> RecipeSelectionWidget = nullptr;

	// ===== 事件处理 =====

	// 添加建筑按钮点击
	UFUNCTION()
	void OnAddBuildingClicked();

	// 配方选择完成
	UFUNCTION()
	void OnRecipeSelected(const FString& RecipeID);

	// ===== 配置 =====

	// 建筑条目Widget类
	UPROPERTY(EditAnywhere, Category = "Building Page")
	TSubclassOf<UBuildingEntryWidget01> BuildingEntryWidgetClass;

	// ===== 辅助方法 =====

	// 创建建筑条目Widget
	UBuildingEntryWidget01* CreateBuildingEntryWidget(const FTownBuildingData01& BuildingData);

	// 更新统计信息
	void UpdateStatistics();

	// 获取生产管理器
	class UProductionManager01* GetProductionManager() const;
};