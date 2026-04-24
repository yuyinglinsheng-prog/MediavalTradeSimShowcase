// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "TownInfoPanel.generated.h"

class UTextBlock;
class UButton;
class UWidgetSwitcher;
class UTownPageBase;
class UTownInfoPage;
class UTownBuildingPage;
class UTownMarketPage;
class UUnitEntryWidget01;

/**
 * 城镇标签页类型枚举
 */
UENUM(BlueprintType)
enum class ETownTabType : uint8
{
	Info        UMETA(DisplayName = "基本信息"),
	Building    UMETA(DisplayName = "建设"),
	Market      UMETA(DisplayName = "市场")
};

/**
 * 城镇信息面板
 * 主控制面板，管理多个标签页的切换
 */
UCLASS()
class PULSATINGPRISM_API UTownInfoPanel : public UDemo01PanelBase
{
	GENERATED_BODY()
	
public:
	// 重写基类方法
	virtual void InitializeUI(class ADemo01_GM* InGameMode) override;
	
	// 设置城镇数据
	UFUNCTION(BlueprintCallable, Category = "Town")
	void SetTownData(const FTownData01& Data);
	
	// 获取城镇数据
	UFUNCTION(BlueprintPure, Category = "Town")
	FTownData01 GetTownData() const { return TownData; }
	
	// 切换标签页
	UFUNCTION(BlueprintCallable, Category = "Town")
	void SwitchToTab(ETownTabType TabType);
	
	// 获取当前标签页
	UFUNCTION(BlueprintPure, Category = "Town")
	ETownTabType GetCurrentTab() const { return CurrentTab; }
	
	// ===== 兼容性方法（桥接到子页面） =====
	
	// 兼容旧版本的刷新方法
	UFUNCTION(BlueprintCallable, Category = "Town|Compatibility")
	void RefreshCaravanList();
	
	UFUNCTION(BlueprintCallable, Category = "Town|Compatibility")
	void RefreshColonistList();
	
	// 兼容旧版本的Entry点击方法
	void OnCaravanEntryClicked(UUnitEntryWidget01* Entry);
	void OnColonistEntryClicked(UUnitEntryWidget01* Entry);
	
protected:
	virtual void NativeConstruct() override;
	
	// 城镇数据
	UPROPERTY(BlueprintReadOnly, Category = "Town")
	FTownData01 TownData;
	
	// 当前标签页
	UPROPERTY(BlueprintReadOnly, Category = "Town")
	ETownTabType CurrentTab = ETownTabType::Info;
	
	// ===== UI 元素（在蓝图中绑定） =====
	
	// 城镇名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TownNameText;
	
	// 城镇 ID 文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TownIDText;
	
	// 网格坐标文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CoordText;
	
	// 人口文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PopulationText;
	
	// 关闭按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;
	
	// ===== 标签页控制 =====
	
	// 标签按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InfoTabButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuildingTabButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MarketTabButton;
	
	// 内容切换器
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ContentSwitcher;
	
	// 页面引用（通过WidgetSwitcher的子项获取）
	UPROPERTY(BlueprintReadOnly, Category = "Town")
	TObjectPtr<UTownInfoPage> InfoPage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Town")
	TObjectPtr<UTownBuildingPage> BuildingPage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Town")
	TObjectPtr<UTownMarketPage> MarketPage;
	
	// ===== 按钮事件 =====
	
	// 关闭按钮点击事件
	UFUNCTION()
	void OnCloseButtonClicked();
	
	// 标签按钮点击事件
	UFUNCTION()
	void OnInfoTabClicked();
	
	UFUNCTION()
	void OnBuildingTabClicked();
	
	UFUNCTION()
	void OnMarketTabClicked();
	
	// 更新标签按钮状态
	void UpdateTabButtonStates();
	
	// 初始化页面引用
	void InitializePageReferences();

public:

	// ===== 数据绑定函数（在蓝图中绑定） =====
	
	// 获取城镇名称
	UFUNCTION(BlueprintPure, Category = "Town")
	FText GetTownName() const;
	
	// 获取城镇 ID
	UFUNCTION(BlueprintPure, Category = "Town")
	FText GetTownID() const;
	
	// 获取网格坐标
	UFUNCTION(BlueprintPure, Category = "Town")
	FText GetCoord() const;
	
	// 获取人口
	UFUNCTION(BlueprintPure, Category = "Town")
	FText GetPopulation() const;
	
	// 设置城镇实例到所有页面
	UFUNCTION(BlueprintCallable, Category = "Town")
	void SetTownInstanceToPages(class ATownActor01* TownInstance);
	
	// 获取市场页面
	UFUNCTION(BlueprintPure, Category = "Town")
	UTownMarketPage* GetMarketPage() const { return MarketPage; }
};
