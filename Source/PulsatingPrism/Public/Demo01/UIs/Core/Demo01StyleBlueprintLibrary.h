// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Demo01StyleBlueprintLibrary.generated.h"

class UDemo01StyleManager;
class UWidget;

/**
 * Demo01 样式系统蓝图函数库
 * 提供蓝图中使用样式系统的便捷方法
 */
UCLASS()
class PULSATINGPRISM_API UDemo01StyleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 重新加载样式配置
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	static bool ReloadStyleConfiguration();

	// 刷新所有样式
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	static void RefreshAllStyles();

	// 应用样式到指定组件
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	static void ApplyStyleToWidget(UWidget* Widget);

	// 检查样式系统是否已初始化
	UFUNCTION(BlueprintPure, Category = "Demo01 Style")
	static bool IsStyleSystemInitialized();

	// 测试按钮颜色配置（输出到日志）
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	static void TestButtonColors();

	// 测试UI组件样式应用
	UFUNCTION(BlueprintCallable, Category = "Demo01 Style")
	static void TestUIComponentStyles();

	// 获取样式管理器实例（仅供C++使用）
	static UDemo01StyleManager* GetStyleManager();
};