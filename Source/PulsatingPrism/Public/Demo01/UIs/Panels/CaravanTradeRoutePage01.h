// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "CaravanTradeRoutePage01.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UComboBoxString;
class UPanelWidget;
class UCaravanTradeRouteTownEntryWidget01;

// 前向声明
class ADemo01_PS;

/**
 * 商队贸易路线页面
 * 显示和编辑商队的贸易路线配置
 */
UCLASS()
class PULSATINGPRISM_API UCaravanTradeRoutePage01 : public UDemo01UIBase
{
	GENERATED_BODY()
	
public:
	// 初始化页面
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void InitializePage(const FCaravanData01& InCaravanData);
	
	// 刷新页面显示
	virtual void UpdateUI() override;
	
	// 获取当前商队ID
	UFUNCTION(BlueprintPure, Category = "TradeRoute")
	int32 GetCaravanID() const { return CaravanData.EntityID; }
	
	// 获取贸易路线配置
	UFUNCTION(BlueprintPure, Category = "TradeRoute")
	FTradeRouteConfig GetTradeRouteConfig() const { return CaravanData.TradeRoute; }

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 自动贸易开关
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AutoTradeButton = nullptr;
	
	// 自动贸易状态文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AutoTradeStatusText = nullptr;
	
	// 金钱变化文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MoneyChangeText = nullptr;
	
	// 城镇列表滚动框
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> TownListBox = nullptr;
	
	// 添加城镇下拉框
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> AddTownComboBox = nullptr;
	
	// 地图选择按钮（Demo01留空）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MapSelectButton = nullptr;
	
	// 删除路线按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteRouteButton = nullptr;

	// ===== Widget类引用 =====
	
	// 城镇条目Widget类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Classes")
	TSubclassOf<UCaravanTradeRouteTownEntryWidget01> TownEntryWidgetClass;

	// ===== 数据 =====
	
	// 商队数据
	UPROPERTY()
	FCaravanData01 CaravanData;
	
	// ===== 事件处理 =====
	
	// 自动贸易按钮点击
	UFUNCTION()
	void OnAutoTradeClicked();
	
	// 添加城镇下拉框选择改变
	UFUNCTION()
	void OnAddTownSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	
	// 地图选择按钮点击（Demo01留空）
	UFUNCTION()
	void OnMapSelectClicked();
	
	// 删除路线按钮点击
	UFUNCTION()
	void OnDeleteRouteClicked();

public:
	// ===== 供条目调用的函数 =====
	
	// 交换城镇位置
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void SwapTownPosition(int32 TownID, int32 CurrentIndex, bool bMoveUp);
	
	// 从路线移除城镇
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void RemoveTownFromRoute(int32 TownID);
	
	// 添加城镇到路线
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void AddTownToRoute(int32 TownID);
	
	// 获取所有城镇名称列表（用于下拉框）
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	TArray<FString> GetAllTownOptions() const;

private:
	// 创建城镇条目
	void CreateTownEntries();
	
	// 更新自动贸易状态显示
	void UpdateAutoTradeStatus();
	
	// 更新金钱变化显示
	void UpdateMoneyChangeDisplay();
	
	// 从GameMode获取PlayerState
	ADemo01_PS* GetPlayerState() const;
	
};
