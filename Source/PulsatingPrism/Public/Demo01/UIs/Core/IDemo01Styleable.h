// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDemo01Styleable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UDemo01Styleable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Demo01 样式接口
 * 所有需要应用样式的UI组件都应该实现这个接口
 */
class PULSATINGPRISM_API IDemo01Styleable
{
	GENERATED_BODY()

public:
	// 应用样式到组件
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Demo01 Style")
	void ApplyWidgetStyle();
	virtual void ApplyWidgetStyle_Implementation() {};

	// 获取样式类型（用于StyleManager识别组件类型）
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Demo01 Style")
	FString GetStyleType() const;
	virtual FString GetStyleType_Implementation() const { return TEXT("Default"); }

	// 是否需要自动应用样式
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Demo01 Style")
	bool ShouldAutoApplyStyle() const;
	virtual bool ShouldAutoApplyStyle_Implementation() const { return true; }
};