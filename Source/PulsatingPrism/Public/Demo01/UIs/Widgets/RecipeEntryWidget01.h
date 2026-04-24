// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "RecipeEntryWidget01.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeEntryClicked, const FString&, RecipeID);

/**
 * 配方条目Widget
 * 用于在配方选择器中显示单个配方信息
 */
UCLASS(BlueprintType, Blueprintable)
class PULSATINGPRISM_API URecipeEntryWidget01 : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 初始化配方条目
	UFUNCTION(BlueprintCallable, Category = "Recipe Entry")
	void InitializeRecipeEntry(const FRecipeData01& InRecipeData);

	// 禁用自动样式（动态创建时Slate层未就绪）
	virtual bool ShouldAutoApplyStyle_Implementation() const override { return false; }

	// 设置选中状态
	UFUNCTION(BlueprintCallable, Category = "Recipe Entry")
	void SetSelected(bool bInSelected);

	// 获取配方ID
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Recipe Entry")
	FString GetRecipeID() const { return RecipeData.RecipeID; }

	// 配方条目被点击事件
	UPROPERTY(BlueprintAssignable, Category = "Recipe Entry")
	FOnRecipeEntryClicked OnRecipeEntryClicked;

protected:
	virtual void NativeConstruct() override;

	// 按钮点击处理
	UFUNCTION()
	void OnButtonClicked();

	// UI组件
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RecipeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RecipeNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RecipeDescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FacilityText;

	// 配方数据
	UPROPERTY(BlueprintReadOnly, Category = "Recipe Entry")
	FRecipeData01 RecipeData;

	// 是否选中
	UPROPERTY(BlueprintReadOnly, Category = "Recipe Entry")
	bool bIsSelected = false;

	// 蓝图事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Recipe Entry")
	void OnSelectionChanged(bool bSelected);

	UFUNCTION(BlueprintImplementableEvent, Category = "Recipe Entry")
	void OnRecipeDataUpdated();
};