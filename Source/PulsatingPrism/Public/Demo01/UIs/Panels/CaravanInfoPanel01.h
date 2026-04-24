// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "CaravanInfoPanel01.generated.h"

class UTextBlock;
class UButton;
class UWidgetSwitcher;
class UCaravanInventoryPage01;
class UCaravanTradeRoutePage01;
class AMoveableEntity01;

// 前向声明
class UCaravanTradeRouteTownEntryWidget01;

/**
 * 商队标签页类型枚举
 */
UENUM(BlueprintType)
enum class ECaravanTabType : uint8
{
	Inventory   UMETA(DisplayName = "库存"),
	TradeRoute  UMETA(DisplayName = "贸易路线")
};

/**
 * 商队信息面板
 * 显示商队基本信息和库存
 */
UCLASS()
class PULSATINGPRISM_API UCaravanInfoPanel01 : public UDemo01PanelBase
{
	GENERATED_BODY()

public:
	// 初始化面板（兼容旧接口）
	UFUNCTION(BlueprintCallable, Category = "Caravan Panel")
	void InitializePanel(AMoveableEntity01* InCaravan);
	
	// 初始化UI（UIManager调用）
	virtual void InitializeUI(class ADemo01_GM* InGameMode) override;
	
	// 设置商队数据
	void SetCaravanData(const FCaravanData01& InCaravanData);
	
	// 设置商队实例
	void SetCaravanInstance(AMoveableEntity01* InCaravan);
	
	// 隐藏UI
	void HideUI();
	
	// 刷新面板数据
	UFUNCTION(BlueprintCallable, Category = "Caravan Panel")
	void RefreshPanel();

	// 获取当前商队
	UFUNCTION(BlueprintPure, Category = "Caravan Panel")
	AMoveableEntity01* GetCaravan() const { return Caravan; }

	// 获取当前商队ID（实例销毁后仍然有效）
	UFUNCTION(BlueprintPure, Category = "Caravan Panel")
	int32 GetCaravanID() const { return CaravanData.EntityID; }

	// 获取库存页面
	UFUNCTION(BlueprintPure, Category = "Caravan Panel")
	UCaravanInventoryPage01* GetInventoryPage() const { return InventoryPage; }
	
	// 切换标签页
	UFUNCTION(BlueprintCallable, Category = "Caravan Panel")
	void SwitchToTab(ECaravanTabType TabType);
	
	// 获取当前标签页
	UFUNCTION(BlueprintPure, Category = "Caravan Panel")
	ECaravanTabType GetCurrentTab() const { return CurrentTab; }

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 商队名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CaravanNameText = nullptr;
	
	// 商队状态文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CaravanStatusText = nullptr;
	
	// 商队位置文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CaravanLocationText = nullptr;
	
	// 库存页面
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCaravanInventoryPage01> InventoryPage = nullptr;
	
	// 贸易路线页面（通过WidgetSwitcher的子项获取）
	UPROPERTY(BlueprintReadOnly, Category = "Caravan Panel")
	TObjectPtr<UCaravanTradeRoutePage01> TradeRoutePage = nullptr;
	
	// 关闭按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton = nullptr;
	
	// 删除商队按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteCaravanButton = nullptr;
	
	// ===== 标签页控制 =====
	
	// 库存标签按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InventoryTabButton = nullptr;
	
	// 贸易路线标签按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TradeRouteTabButton = nullptr;
	
	// 内容切换器
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ContentSwitcher = nullptr;

	// ===== 数据 =====
	
	// 关联的商队（可能为空，如果商队在城镇中）
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> Caravan = nullptr;
	
	// 商队数据（始终有效）
	UPROPERTY()
	FCaravanData01 CaravanData;
	
	// 当前标签页
	UPROPERTY(BlueprintReadOnly, Category = "Caravan Panel")
	ECaravanTabType CurrentTab = ECaravanTabType::Inventory;
	
	// ===== 事件处理 =====
	
	// 关闭按钮点击
	UFUNCTION()
	void OnCloseClicked();
	
	// 删除商队按钮点击
	UFUNCTION()
	void OnDeleteCaravanClicked();
	
	// 标签页按钮点击事件
	UFUNCTION()
	void OnInventoryTabClicked();
	
	UFUNCTION()
	void OnTradeRouteTabClicked();

private:
	// 更新商队基本信息显示
	void UpdateCaravanInfo();
	
	// 获取商队状态文本
	FText GetCaravanStatusText() const;
	
	// 获取商队位置文本
	FText GetCaravanLocationText() const;
	
	// 更新标签按钮状态
	void UpdateTabButtonStates();
	
	// 初始化页面引用
	void InitializePageReferences();

public:
	// ===== 蓝图事件 =====
	
	// 面板关闭事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Caravan Panel")
	void OnPanelClosed();
	
	// 删除商队确认事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Caravan Panel")
	void OnDeleteCaravanConfirm();
};