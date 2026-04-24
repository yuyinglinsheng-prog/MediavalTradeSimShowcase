// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "Demo01/Core01/MarketManager01.h"
#include "ResourceEntryWidget01.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;
class UImage;
class ATownActor01;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnResourceTradeCompleted, const FString&, ResourceID, int32, Quantity, float, TotalValue, bool, bWasPurchase);

/**
 * 资源条目Widget
 * 显示单个资源的信息和交易操作
 */
UCLASS()
class PULSATINGPRISM_API UResourceEntryWidget01 : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 初始化资源条目
	UFUNCTION(BlueprintCallable, Category = "Resource Entry")
	void InitializeResourceEntry(ATownActor01* InTown, const FString& InResourceID);

	// 禁用自动样式应用（在AddChild之前widget的Slate层未就绪，延迟到InitializeResourceEntry）
	virtual bool ShouldAutoApplyStyle_Implementation() const override { return false; }

	// 刷新显示数据
	UFUNCTION(BlueprintCallable, Category = "Resource Entry")
	void RefreshDisplay();

	// 获取资源ID
	UFUNCTION(BlueprintPure, Category = "Resource Entry")
	FString GetResourceID() const { return ResourceID; }

	// 设置商队ID
	UFUNCTION(BlueprintCallable, Category = "Resource Entry")
	void SetCaravanID(int32 InCaravanID);

	// 设置交易权限
	UFUNCTION(BlueprintCallable, Category = "Resource Entry")
	void SetTradePermission(bool bCanTrade);

	// 检查是否可以交易
	UFUNCTION(BlueprintPure, Category = "Resource Entry")
	bool CanTrade() const { return bCanTradeWithCaravan && CaravanID >= 0; }

	// 交易完成事件
	UPROPERTY(BlueprintAssignable, Category = "Resource Entry")
	FOnResourceTradeCompleted OnTradeCompleted;

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 资源名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResourceNameText = nullptr;

	// 库存数量文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText = nullptr;

	// 基础价格文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BasePriceText = nullptr;

	// 市场价格文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MarketPriceText = nullptr;

	// 交易数量输入框
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TradeQuantityInput = nullptr;

	// 购买按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyButton = nullptr;

	// 出售按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SellButton = nullptr;

	// 总价显示文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalValueText = nullptr;

	// ===== 事件处理 =====

	// 购买按钮点击
	UFUNCTION()
	void OnBuyClicked();

	// 出售按钮点击
	UFUNCTION()
	void OnSellClicked();

	// 交易数量输入变化
	UFUNCTION()
	void OnTradeQuantityChanged(const FText& Text);

	// ===== 数据 =====

	// 关联的城镇
	UPROPERTY()
	TObjectPtr<ATownActor01> Town = nullptr;

	// 关联的商队ID（用于交易）
	UPROPERTY()
	int32 CaravanID = -1;

	// 是否可以与商队交易
	UPROPERTY()
	bool bCanTradeWithCaravan = false;

	// 资源ID
	UPROPERTY()
	FString ResourceID;

	// 当前交易数量
	UPROPERTY()
	int32 TradeQuantity = 1;

	// 缓存的市场价格（由 RefreshDisplay 写入，供 CalculateTotalValue 直接使用）
	UPROPERTY()
	float CachedMarketPrice = 0.0f;

	// ===== 辅助方法 =====

	// 获取产品数据
	FProductData01 GetProductData() const;

	// 计算交易总价
	float CalculateTotalValue() const;

	// 更新总价显示
	void UpdateTotalValueDisplay();

	// 更新按钮状态
	void UpdateButtonStates();

	// 获取生产管理器
	class UProductionManager01* GetProductionManager() const;

	// 获取市场管理器
	class UMarketManager01* GetMarketManager() const;
	
	// 刷新商队面板库存显示
	void RefreshCaravanPanelIfOpen();

public:
	// ===== 蓝图事件 =====

	// 交易完成事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Resource Entry")
	void OnTradeCompletedBP(const FString& InResourceID, int32 Quantity, float TotalValue, bool bWasPurchase);

	// 交易失败事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Resource Entry")
	void OnTradeFailed(const FString& Reason);
};