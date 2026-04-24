// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "Demo01UIManager.generated.h"

class ADemo01_GM;
class ASelectableEntity01;
class ATownActor01;
class AMoveableEntity01;
class UTownInfoPanel;
class UCaravanInfoPanel01;
class UColonistInfoPanel01;
class UUnitEntryWidget01;
class UDemo01MainUI;
class UDemo01StyleManager;

/**
 * 输入模式枚举
 */
UENUM(BlueprintType)
enum class EDemo01InputMode : uint8
{
	GameOnly      UMETA(DisplayName = "游戏输入"),
	UIOnly        UMETA(DisplayName = "UI输入"),
	GameAndUI     UMETA(DisplayName = "游戏和UI")
};

/**
 * Demo01 UI 管理器
 * 统一管理所有 UI 的创建、显示、隐藏、销毁
 * 作为 UObject 存在于 GameMode 中
 */
UCLASS()
class PULSATINGPRISM_API UDemo01UIManager : public UObject
{
	GENERATED_BODY()
	
public:
	// 初始化 UI 系统
	void Initialize(ADemo01_GM* InGameMode, const FDemo01UIConfig& InUIConfig);
	
	// 初始化样式系统
	void InitializeStyleSystem();
	
	// ===== 选择变化监听（新增） =====
	
	// 监听城镇选择变化
	void OnTownSelectionChanged(int32 NewTownID, int32 OldTownID);
	
	// 监听可移动单位选择变化
	void OnMoveableUnitSelectionChanged(int32 NewUnitID, EEntityType01 NewType, 
	                                   int32 OldUnitID, EEntityType01 OldType);
	
	// 用户主动关闭面板的回调
	void OnTownPanelClosedByUser();
	void OnMoveableUnitPanelClosedByUser();
	
	// ===== 面板 UI 管理 =====
	
	// 打开城镇信息面板
	UFUNCTION(BlueprintCallable, Category = "UI|Panels")
	void OpenTownInfoPanel(ATownActor01* Town);
	
	// 关闭城镇信息面板
	UFUNCTION(BlueprintCallable, Category = "UI|Panels")
	void CloseTownInfoPanel();
	
	// 打开商队信息面板（通过实例）
	void OpenCaravanInfoPanel(AMoveableEntity01* Caravan);
	
	// 打开商队信息面板（通过ID）
	UFUNCTION(BlueprintCallable, Category = "UI|Panels")
	void OpenCaravanInfoPanelByID(int32 CaravanID);
	
	// 关闭商队信息面板
	UFUNCTION(BlueprintCallable, Category = "UI|Panels")
	void CloseCaravanInfoPanel();
	
	// 打开殖民者信息面板（通过实例）
	void OpenColonistInfoPanel(AMoveableEntity01* Colonist);
	
	// 打开殖民者信息面板（通过ID）
	UFUNCTION(BlueprintCallable, Category = "UI|Panels")
	void OpenColonistInfoPanelByID(int32 ColonistID);
	
	// 关闭殖民者信息面板
	UFUNCTION(BlueprintCallable, Category = "UI|Panels")
	void CloseColonistInfoPanel();
	
	// 获取当前打开的城镇
	UFUNCTION(BlueprintPure, Category = "UI|Panels")
	ATownActor01* GetCurrentTown() const { return CurrentTown; }
	
	// 获取当前打开的商队
	AMoveableEntity01* GetCurrentCaravan() const { return CurrentCaravan; }

	// 获取当前打开的殖民者
	AMoveableEntity01* GetCurrentColonist() const { return CurrentColonist; }
	
	// ===== 输入模式管理 =====
	
	// 设置输入模式
	UFUNCTION(BlueprintCallable, Category = "UI|Input")
	void SetInputMode(EDemo01InputMode Mode);
	
	// 检查是否有任何面板打开
	UFUNCTION(BlueprintPure, Category = "UI|Panels")
	bool HasAnyPanelOpen() const;
	
	// 刷新城镇信息面板（如果打开）
	void RefreshTownInfoPanelIfOpen(ATownActor01* Town);

	// 当前打开城镇的库存变化回调（由 OnInventoryChanged delegate 触发）
	void OnActiveTownInventoryChanged();

	// 刷新商队信息面板（如果打开）
	void RefreshCaravanInfoPanelIfOpen(AMoveableEntity01* Caravan);
	
	// 刷新商队信息面板（通过ID）
	void RefreshCaravanInfoPanelIfOpen(int32 CaravanID);
	
	// 刷新殖民者信息面板（如果打开）
	void RefreshColonistInfoPanelIfOpen(AMoveableEntity01* Colonist);
	
	// 刷新殖民者信息面板（通过ID）
	void RefreshColonistInfoPanelIfOpen(int32 ColonistID);
	
	// 更新殖民者建城按钮状态（如果面板已打开）
	void UpdateColonistFoundCityButtonState(int32 ColonistID);
	
	// 更新当前殖民者引用（如果面板已打开且是同一个殖民者）
	void UpdateCurrentColonistIfOpen(AMoveableEntity01* Colonist);
	
	// 获取 UnitEntryWidget01 类
	TSubclassOf<UUnitEntryWidget01> GetUnitEntryWidgetClass() const;
	
	// ===== 面板状态通知方法 =====
	
	// 面板状态通知方法
	void NotifyTownPanelOpened(ATownActor01* Town);
	void NotifyCaravanPanelOpened(AMoveableEntity01* Caravan);
	void NotifyColonistPanelOpened(AMoveableEntity01* Colonist);
	void NotifyTownPanelClosed();
	void NotifyCaravanPanelClosed();
	void NotifyColonistPanelClosed();
	
	// 商队状态变化通知（进入/离开城镇）
	void NotifyCaravanStateChanged(int32 CaravanID);
	
	// 商队面板打开通知（通过ID，不依赖实例）
	void NotifyCaravanPanelOpenedByID(int32 CaravanID);
	
	// 殖民者面板打开通知（通过ID，不依赖实例）
	void NotifyColonistPanelOpenedByID(int32 ColonistID);
	
	// 殖民者面板关闭（通过ID检查，避免错误关闭其他殖民者UI）
	void CloseColonistInfoPanelByID(int32 ColonistID);
	
	// 获取当前选中的商队ID（保持架构解耦）
	int32 GetCurrentSelectedCaravanID();
	
protected:
	// GameMode 引用
	UPROPERTY()
	ADemo01_GM* GameMode = nullptr;
	
	// UI 配置（从GameMode传入）
	FDemo01UIConfig UIConfig;
	
	// ===== 主UI容器 =====
	
	UPROPERTY()
	TObjectPtr<UDemo01MainUI> MainUIWidget = nullptr;
	
	// ===== 选择状态跟踪（新增） =====
	
	// 当前选中的城镇ID和可移动单位ID（用于同步选择状态）
	int32 CurrentTownID = -1;
	int32 CurrentMoveableUnitID = -1;
	EEntityType01 CurrentMoveableUnitType = EEntityType01::None;
	
	// ===== 面板状态跟踪（保留兼容性） =====
	
	// 当前打开的城镇
	UPROPERTY()
	TObjectPtr<ATownActor01> CurrentTown = nullptr;
	
	// 当前打开的商队
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> CurrentCaravan = nullptr;

	// 当前打开的殖民者
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> CurrentColonist = nullptr;
};
