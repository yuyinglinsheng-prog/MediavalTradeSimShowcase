// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "TownPageBase.generated.h"

class ADemo01_GM;

/**
 * 城镇页面基类
 * 为所有城镇子页面提供通用接口和功能
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class PULSATINGPRISM_API UTownPageBase : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 设置城镇数据
	UFUNCTION(BlueprintCallable, Category = "Town Page")
	virtual void SetTownData(const FTownData01& Data);
	
	// 设置城镇实例
	UFUNCTION(BlueprintCallable, Category = "Town Page")
	virtual void SetTownInstance(class ATownActor01* InTownInstance);
	
	// 刷新页面内容
	UFUNCTION(BlueprintCallable, Category = "Town Page")
	virtual void RefreshPage();
	
	// 初始化页面（由父面板调用）
	UFUNCTION(BlueprintCallable, Category = "Town Page")
	virtual void InitializePage(ADemo01_GM* InGameMode);

	// 重写样式类型
	virtual FString GetStyleType_Implementation() const override { return TEXT("TownPage"); }

protected:
	// 城镇数据
	UPROPERTY(BlueprintReadOnly, Category = "Town Page")
	FTownData01 TownData;
	
	// 城镇实例引用
	UPROPERTY(BlueprintReadOnly, Category = "Town Page")
	TObjectPtr<class ATownActor01> TownInstance;
};