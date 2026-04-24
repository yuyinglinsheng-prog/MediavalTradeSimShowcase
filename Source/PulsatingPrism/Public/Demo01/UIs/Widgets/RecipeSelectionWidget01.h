// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "RecipeSelectionWidget01.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UVerticalBox;
class ATownActor01;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeSelected, const FString&, RecipeID);

/**
 * 配方选择Widget
 * 显示可用配方列表供玩家选择
 */
UCLASS()
class PULSATINGPRISM_API URecipeSelectionWidget01 : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 初始化配方选择器
	UFUNCTION(BlueprintCallable, Category = "Recipe Selection")
	void InitializeRecipeSelection(ATownActor01* InTown, const FString& InCurrentRecipeID = TEXT(""));

	// 刷新配方列表
	UFUNCTION(BlueprintCallable, Category = "Recipe Selection")
	void RefreshRecipeList();

	// 配方选择事件
	UPROPERTY(BlueprintAssignable, Category = "Recipe Selection")
	FOnRecipeSelected OnRecipeSelected;

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 标题文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText = nullptr;

	// 配方列表容器
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> RecipeScrollBox = nullptr;

	// 配方列表
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> RecipeListBox = nullptr;

	// 确认按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton = nullptr;

	// 取消按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton = nullptr;

	// ===== 事件处理 =====

	// 确认按钮点击
	UFUNCTION()
	void OnConfirmClicked();

	// 取消按钮点击
	UFUNCTION()
	void OnCancelClicked();

	// 配方条目点击
	UFUNCTION()
	void OnRecipeEntryClicked(const FString& RecipeID);

	// 通用按钮点击处理
	UFUNCTION()
	void OnGenericButtonClicked();

	// ===== 数据 =====

	// 关联的城镇
	UPROPERTY()
	TObjectPtr<ATownActor01> Town = nullptr;

	// 当前选中的配方ID
	UPROPERTY()
	FString SelectedRecipeID;

	// 原始配方ID（用于取消时恢复）
	UPROPERTY()
	FString OriginalRecipeID;

	// 配方条目Widget类
	UPROPERTY(EditAnywhere, Category = "Recipe Selection")
	TSubclassOf<UUserWidget> RecipeEntryWidgetClass;

	// ===== 辅助方法 =====

	// 创建配方条目Widget
	UUserWidget* CreateRecipeEntryWidget(const FRecipeData01& RecipeData);

	// 更新选中状态显示
	void UpdateSelectionDisplay();

	// 获取生产管理器
	class UProductionManager01* GetProductionManager() const;

public:
	// ===== 蓝图事件 =====

	// 配方选择确认事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Recipe Selection")
	void OnRecipeSelectionConfirmed(const FString& RecipeID);

	// 配方选择取消事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Recipe Selection")
	void OnRecipeSelectionCancelled();
};