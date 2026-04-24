// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "CaravanTradeRouteTownEntryWidget01.generated.h"

class UTextBlock;
class UButton;

// 前向声明
class UCaravanTradeRoutePage01;

/**
 * 商队贸易路线城镇条目
 * 显示贸易路线中的一个城镇，并提供删除功能
 */
UCLASS()
class PULSATINGPRISM_API UCaravanTradeRouteTownEntryWidget01 : public UDemo01UIBase
{
	GENERATED_BODY()
	
public:
	// 初始化条目
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void InitializeEntry(int32 InTownID, int32 InRouteIndex, UCaravanTradeRoutePage01* InParentPage);
	
	// 获取城镇ID
	UFUNCTION(BlueprintPure, Category = "TradeRoute")
	int32 GetTownID() const { return TownID; }
	
	// 更新按钮状态
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void UpdateButtonStates(int32 TotalCount);

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 城镇名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TownNameText = nullptr;
	
	// 城镇ID文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TownIDText = nullptr;
	
	// 上移按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveUpButton = nullptr;
	
	// 下移按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveDownButton = nullptr;
	
	// 删除按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RemoveButton = nullptr;

	// ===== 数据 =====
	
	// 城镇ID
	UPROPERTY()
	int32 TownID = -1;
	
	// 在路线中的索引
	UPROPERTY()
	int32 RouteIndex = -1;
	
	// 父页面引用
	UPROPERTY()
	TObjectPtr<UCaravanTradeRoutePage01> ParentPage = nullptr;
	
	// ===== 事件处理 =====
	
	// 上移按钮点击
	UFUNCTION()
	void OnMoveUpClicked();
	
	// 下移按钮点击
	UFUNCTION()
	void OnMoveDownClicked();
	
	// 删除按钮点击
	UFUNCTION()
	void OnRemoveClicked();

public:
	// ===== 蓝图事件 =====
	
	// 请求从路线移除城镇（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "TradeRoute")
	void OnRequestRemoveFromRoute(int32 InTownID);
};
