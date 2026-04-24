// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Demo01StyleConfig.generated.h"

/**
 * Demo01 颜色配置
 * 定义所有UI组件使用的颜色
 */
USTRUCT(BlueprintType)
struct PULSATINGPRISM_API FDemo01ColorConfig
{
	GENERATED_BODY()

	// ===== 面板颜色 =====
	
	// 面板背景色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panel Colors")
	FLinearColor PanelBackground = FLinearColor(0.05f, 0.1f, 0.2f, 0.9f);
	
	// 面板边框色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panel Colors")
	FLinearColor PanelBorder = FLinearColor(0.8f, 0.6f, 0.2f, 1.0f);
	
	// 面板头部背景色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panel Colors")
	FLinearColor PanelHeader = FLinearColor(0.1f, 0.15f, 0.25f, 0.8f);

	// ===== 按钮颜色 =====
	
	// 主要按钮颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Colors")
	FLinearColor ButtonPrimary = FLinearColor(0.2f, 0.6f, 0.2f, 1.0f);
	
	// 主要按钮悬停色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Colors")
	FLinearColor ButtonPrimaryHover = FLinearColor(0.3f, 0.5f, 0.9f, 1.0f);
	
	// 次要按钮颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Colors")
	FLinearColor ButtonSecondary = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
	
	// 次要按钮悬停色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Colors")
	FLinearColor ButtonSecondaryHover = FLinearColor(0.3f, 0.7f, 0.3f, 1.0f);
	
	// 关闭按钮颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Colors")
	FLinearColor ButtonClose = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

	// ===== 文本颜色 =====
	
	// 主要文本色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Colors")
	FLinearColor TextPrimary = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	// 次要文本色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Colors")
	FLinearColor TextSecondary = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
	
	// 标题文本色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Colors")
	FLinearColor TextHeader = FLinearColor(0.8f, 0.6f, 0.2f, 1.0f);

	// ===== 列表项颜色 =====
	
	// 列表项背景色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List Colors")
	FLinearColor ListItemBackground = FLinearColor(0.1f, 0.15f, 0.2f, 0.8f);
	
	// 列表项悬停色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List Colors")
	FLinearColor ListItemHover = FLinearColor(0.2f, 0.2f, 0.2f, 0.9f);
	
	// 列表项选中色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List Colors")
	FLinearColor ListItemSelected = FLinearColor(0.8f, 0.6f, 0.2f, 0.3f);
	
	// 列表项边框色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List Colors")
	FLinearColor ListItemBorder = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

	// ===== 滚动框颜色 =====
	
	// 滚动框背景色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox Colors")
	FLinearColor ScrollBoxBackground = FLinearColor(0.05f, 0.1f, 0.15f, 0.9f);
	
	// 滚动条颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox Colors")
	FLinearColor ScrollBarColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	// 滚动条悬停色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox Colors")
	FLinearColor ScrollBarHover = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);

	// ===== 输入框颜色 =====
	
	// 输入框背景色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Colors")
	FLinearColor InputBackground = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
	
	// 输入框边框色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Colors")
	FLinearColor InputBorder = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
	
	// 输入框焦点边框色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Colors")
	FLinearColor InputFocusBorder = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);

	// ===== 图标和图像颜色 =====
	
	// 图标默认色调
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Colors")
	FLinearColor IconTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	// 图标禁用色调
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Colors")
	FLinearColor IconDisabledTint = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// ===== 进度条颜色 =====
	
	// 进度条背景色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Colors")
	FLinearColor ProgressBackground = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
	
	// 进度条填充色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Colors")
	FLinearColor ProgressFill = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
};

/**
 * Demo01 字体配置
 * 定义所有UI组件使用的字体大小和样式
 */
USTRUCT(BlueprintType)
struct PULSATINGPRISM_API FDemo01FontConfig
{
	GENERATED_BODY()

	// ===== 字体大小 =====
	
	// 标题字体大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Font Sizes", meta = (ClampMin = "12", ClampMax = "48"))
	int32 HeaderSize = 20;
	
	// 子标题字体大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Font Sizes", meta = (ClampMin = "10", ClampMax = "32"))
	int32 SubHeaderSize = 16;
	
	// 正文字体大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Font Sizes", meta = (ClampMin = "8", ClampMax = "24"))
	int32 BodySize = 13;
	
	// 小字体大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Font Sizes", meta = (ClampMin = "8", ClampMax = "16"))
	int32 CaptionSize = 11;
	
	// 按钮字体大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Font Sizes", meta = (ClampMin = "10", ClampMax = "20"))
	int32 ButtonSize = 12;
};

/**
 * Demo01 间距配置
 * 定义所有UI组件使用的间距和边距
 */
USTRUCT(BlueprintType)
struct PULSATINGPRISM_API FDemo01SpacingConfig
{
	GENERATED_BODY()

	// ===== 面板间距 =====
	
	// 面板内边距
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panel Spacing", meta = (ClampMin = "0", ClampMax = "50"))
	float PanelPadding = 16.0f;
	
	// 面板头部内边距
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panel Spacing", meta = (ClampMin = "0", ClampMax = "30"))
	float PanelHeaderPadding = 16.0f;
	
	// 面板边框宽度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panel Spacing", meta = (ClampMin = "0", ClampMax = "5"))
	float PanelBorderWidth = 1.5f;

	// ===== 按钮间距 =====
	
	// 按钮内边距（水平）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Spacing", meta = (ClampMin = "0", ClampMax = "30"))
	float ButtonPaddingHorizontal = 12.0f;
	
	// 按钮内边距（垂直）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Spacing", meta = (ClampMin = "0", ClampMax = "20"))
	float ButtonPaddingVertical = 6.0f;

	// ===== 列表项间距 =====
	
	// 列表项内边距（水平）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List Spacing", meta = (ClampMin = "0", ClampMax = "30"))
	float ListItemPaddingHorizontal = 10.0f;
	
	// 列表项内边距（垂直）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List Spacing", meta = (ClampMin = "0", ClampMax = "20"))
	float ListItemPaddingVertical = 6.0f;
	
	// 列表项之间的间距
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List Spacing", meta = (ClampMin = "0", ClampMax = "10"))
	float ListItemSpacing = 3.0f;

	// ===== 通用间距 =====
	
	// 圆角半径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Spacing", meta = (ClampMin = "0", ClampMax = "20"))
	float CornerRadius = 4.0f;
	
	// 标准边框宽度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Spacing", meta = (ClampMin = "0", ClampMax = "3"))
	float BorderWidth = 1.0f;

	// ===== 滚动框间距 =====
	
	// 滚动框内边距
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox Spacing", meta = (ClampMin = "0", ClampMax = "20"))
	float ScrollBoxPadding = 8.0f;
	
	// 滚动条宽度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox Spacing", meta = (ClampMin = "8", ClampMax = "20"))
	float ScrollBarWidth = 12.0f;

	// ===== 输入框间距 =====
	
	// 输入框内边距（水平）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Spacing", meta = (ClampMin = "0", ClampMax = "20"))
	float InputPaddingHorizontal = 12.0f;
	
	// 输入框内边距（垂直）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Spacing", meta = (ClampMin = "0", ClampMax = "15"))
	float InputPaddingVertical = 6.0f;

	// ===== 标签页间距 =====
	
	// 标签按钮间距
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Spacing", meta = (ClampMin = "0", ClampMax = "10"))
	float TabButtonSpacing = 2.0f;
	
	// 标签内容区域内边距
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Spacing", meta = (ClampMin = "0", ClampMax = "30"))
	float TabContentPadding = 16.0f;
};

/**
 * Demo01 完整样式配置
 * 包含所有样式设置的主结构
 */
USTRUCT(BlueprintType)
struct PULSATINGPRISM_API FDemo01StyleConfig
{
	GENERATED_BODY()

	// 颜色配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Config")
	FDemo01ColorConfig Colors;

	// 字体配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Config")
	FDemo01FontConfig Fonts;

	// 间距配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Config")
	FDemo01SpacingConfig Spacing;
};