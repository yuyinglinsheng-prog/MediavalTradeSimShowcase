// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Demo01UIBase.generated.h"

class ADemo01_GM;

/**
 * Demo01 UI 基类
 * 所有 UI Widget 的基类，提供通用功能
 */
UCLASS(Abstract)
class PULSATINGPRISM_API UDemo01UIBase : public UUserWidget, public IDemo01Styleable
{
	GENERATED_BODY()
	
public:
	// 初始化 UI（传入 GameMode 引用）
	UFUNCTION(BlueprintCallable, Category = "Demo01|UI")
	virtual void InitializeUI(ADemo01_GM* InGameMode);
	
	// 显示 UI
	UFUNCTION(BlueprintCallable, Category = "Demo01|UI")
	virtual void ShowUI();
	
	// 隐藏 UI
	UFUNCTION(BlueprintCallable, Category = "Demo01|UI")
	virtual void HideUI();
	
	// 更新 UI 数据
	UFUNCTION(BlueprintCallable, Category = "Demo01|UI")
	virtual void UpdateUI();

	// ===== IDemo01Styleable 接口实现 =====

	// 应用样式到组件（基类实现，子类可重写）
	UFUNCTION(BlueprintCallable, Category = "Demo01|UI")
	virtual void ApplyWidgetStyle_Implementation() override;

	// 递归应用样式到子Widget
	void ApplyStyleToChildren(UWidget* ParentWidget);

	// 获取样式类型
	virtual FString GetStyleType_Implementation() const override;

	// 是否需要自动应用样式
	virtual bool ShouldAutoApplyStyle_Implementation() const override { return true; }

protected:
	virtual void NativeConstruct() override;
	// GameMode 引用
	UPROPERTY(BlueprintReadOnly, Category = "Demo01")
	ADemo01_GM* GameMode = nullptr;
	
	// 是否已初始化
	UPROPERTY(BlueprintReadOnly, Category = "Demo01")
	bool bIsInitialized = false;

public:
	// 检查是否已初始化
	UFUNCTION(BlueprintPure, Category = "Demo01")
	bool IsInitialized() const { return bIsInitialized; }

	// ===== 样式应用辅助方法 =====

	// 自动应用样式到所有BindWidget组件
	UFUNCTION(BlueprintCallable, Category = "Demo01|UI")
	void AutoApplyStylesToBindWidgets();

	// 应用样式到指定组件
	UFUNCTION(BlueprintCallable, Category = "Demo01|UI")
	void ApplyStyleToComponent(UWidget* Widget, const FString& ComponentType = TEXT(""));
};
