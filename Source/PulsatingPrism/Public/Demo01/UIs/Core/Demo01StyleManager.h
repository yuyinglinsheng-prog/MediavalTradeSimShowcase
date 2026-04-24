// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Demo01/UIs/Core/Demo01StyleConfig.h"
#include "Demo01StyleManager.generated.h"

class UWidget;
class UButton;
class UTextBlock;
class UBorder;
class UCanvasPanel;
class UScrollBox;
class UEditableTextBox;
class UImage;
class UProgressBar;
class UPanelWidget;
class UDemo01PanelBase;

/**
 * Demo01 样式管理器
 * 负责管理和应用UI样式配置
 */
UCLASS(BlueprintType)
class PULSATINGPRISM_API UDemo01StyleManager : public UObject
{
	GENERATED_BODY()

public:
	// 获取单例实例
	static UDemo01StyleManager* GetInstance();

	// 初始化样式管理器
	void Initialize();

	// 从文件加载样式配置
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	bool LoadStyleFromFile(const FString& FilePath);

	// 重新加载样式配置
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	bool ReloadStyleConfiguration();

	// 刷新所有已应用样式的组件
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void RefreshAllStyles();

	// ===== 基础样式应用方法 =====

	// 应用面板样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyPanelStyle(class UDemo01PanelBase* Panel);

	// 应用按钮样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyButtonStyle(class UButton* Button, bool bIsPrimary = false, bool bIsCloseButton = false);

	// 添加标签按钮样式方法
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyTabButtonStyle(UButton* Button, bool bIsSelected);

	// 添加资源进度条样式方法
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyResourceProgressStyle(UProgressBar* ProgressBar);
	// 应用文本样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyTextStyle(class UTextBlock* TextBlock, bool bIsHeader = false, bool bIsSecondary = false);

	// 应用边框样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyBorderStyle(class UBorder* Border, bool bIsListItem = false, bool bIsSelected = false);

	// 应用列表项样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyListItemStyle(class UWidget* ListItem, bool bIsSelected = false, bool bIsHovered = false);

	// 应用滚动框样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyScrollBoxStyle(class UScrollBox* ScrollBox);

	// 应用输入框样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyInputBoxStyle(class UEditableTextBox* InputBox);

	// 应用图像样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyImageStyle(class UImage* Image, bool bIsIcon = false, bool bIsDisabled = false);

	// 应用进度条样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyProgressBarStyle(class UProgressBar* ProgressBar);

	// 应用容器样式（VerticalBox, HorizontalBox等）
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyContainerStyle(class UPanelWidget* Container);

	// ===== 专用Widget样式应用方法 =====

	// 应用资源条目样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyResourceEntryStyle(class UUserWidget* ResourceEntry);

	// 应用建筑条目样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyBuildingEntryStyle(class UUserWidget* BuildingEntry);

	// 应用配方条目样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyRecipeEntryStyle(class UUserWidget* RecipeEntry, bool bIsSelected = false);

	// 应用单位条目样式（商队、殖民者等）
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	void ApplyUnitEntryStyle(class UUserWidget* UnitEntry, bool bIsSelected = false);

	// 应用样式到指定组件
	void ApplyStyleToWidget(UWidget* Widget);

	// 获取当前样式配置（只读）
	UFUNCTION(BlueprintPure, Category = "Demo01 Style")
	const FDemo01StyleConfig& GetStyleConfig() const { return StyleConfig; }

protected:
	// 样式配置
	UPROPERTY()
	FDemo01StyleConfig StyleConfig;

	// 单例实例
	static UDemo01StyleManager* Instance;

	// 已应用样式的组件列表（用于刷新）
	UPROPERTY()
	TArray<TWeakObjectPtr<UWidget>> StyledWidgets;

	// ===== 内部辅助方法 =====

	// 从JSON解析样式配置
	bool ParseStyleFromJson(const FString& JsonString);

	// 递归应用样式到子组件
	void ApplyStyleToChildren(UWidget* ParentWidget);

	// 设置组件的基础视觉属性
	void SetWidgetVisualProperties(UWidget* Widget, const FLinearColor& BackgroundColor, 
		const FLinearColor& BorderColor = FLinearColor::Transparent, float BorderWidth = 0.0f);

	// 创建默认样式配置
	void CreateDefaultStyleConfig();
};