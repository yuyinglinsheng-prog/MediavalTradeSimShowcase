// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Panels/TownPageBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "TownMarketPage.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UVerticalBox;
class UResourceEntryWidget01;

/**
 * 城镇市场页面
 * 显示城镇市场和贸易信息
 */
UCLASS()
class PULSATINGPRISM_API UTownMarketPage : public UTownPageBase
{
	GENERATED_BODY()

public:
	// 重写基类方法
	virtual void RefreshPage() override;

	// 刷新资源列表
	UFUNCTION(BlueprintCallable, Category = "Town Market")
	void RefreshResourceList();

	// 刷新玩家资金显示
	UFUNCTION(BlueprintCallable, Category = "Town Market")
	void RefreshPlayerMoney();

	// 设置商队ID
	UFUNCTION(BlueprintCallable, Category = "Town Market")
	void SetCaravanID(int32 InCaravanID);

	// 更新交易权限
	UFUNCTION(BlueprintCallable, Category = "Town Market")
	void UpdateTradePermission();

	// 获取当前商队ID
	UFUNCTION(BlueprintPure, Category = "Town Market")
	int32 GetCurrentCaravanID() const { return CurrentCaravanID; }

	// 重写城镇实例设置（切换城镇时清空Widget缓存，触发全量重建）
	virtual void SetTownInstance(class ATownActor01* InTownInstance) override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== UI组件绑定 =====

	// 页面标题
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PageTitleText = nullptr;

	// 玩家资金显示
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerMoneyText = nullptr;

	// 库存总价值显示
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InventoryValueText = nullptr;

	// 刷新按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RefreshButton = nullptr;

	// 资源列表滚动框
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ResourceScrollBox = nullptr;

	// 资源列表容器
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ResourceListBox = nullptr;

	// ===== 数据 =====

	// 当前关联的商队ID（用于交易）
	UPROPERTY()
	int32 CurrentCaravanID = -1;

	// 是否可以与商队交易
	UPROPERTY()
	bool bCanTradeWithCaravan = false;

	// ===== 事件处理 =====

	// 刷新按钮点击
	UFUNCTION()
	void OnRefreshClicked();

	// 资源交易完成
	UFUNCTION()
	void OnResourceTradeCompleted(const FString& ResourceID, int32 Quantity, float TotalValue, bool bWasPurchase);

	// PS 金钱变化事件处理
	UFUNCTION()
	void OnPlayerGoldChanged(float NewGold);

	// 是否已经订阅金钱变化事件
	bool bGoldDelegateSubscribed = false;

	// ===== 配置 =====

	// 资源条目Widget类
	UPROPERTY(EditAnywhere, Category = "Market Page")
	TSubclassOf<UResourceEntryWidget01> ResourceEntryWidgetClass;

	// 资源条目Widget缓存（ResourceID → Widget），避免重建导致调用方Widget失效
	UPROPERTY()
	TMap<FString, TObjectPtr<UResourceEntryWidget01>> ResourceWidgetCache;

	// ===== 辅助方法 =====

	// 创建资源条目Widget
	UResourceEntryWidget01* CreateResourceEntryWidget(const FString& ResourceID);

	// 更新统计信息
	void UpdateStatistics();

	// 计算库存总价值
	float CalculateInventoryValue() const;

	// 获取生产管理器
	class UProductionManager01* GetProductionManager() const;

	// 获取市场管理器
	class UMarketManager01* GetMarketManager() const;
};