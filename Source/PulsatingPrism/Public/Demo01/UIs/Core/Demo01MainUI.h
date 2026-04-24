// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "Demo01MainUI.generated.h"

class ADemo01_GM;
class ATownActor01;
class AMoveableEntity01;
class UTownInfoPanel;
class UCaravanInfoPanel01;
class UColonistInfoPanel01;
class UPausePanel01;
class UCanvasPanel;

/**
 * Demo01 主UI容器
 * 统一管理所有面板的显示和布局
 * 作为唯一添加到Viewport的Widget
 */
UCLASS()
class PULSATINGPRISM_API UDemo01MainUI : public UDemo01UIBase
{
	GENERATED_BODY()

public:
	// 初始化主UI（重命名以避免与UUserWidget::Initialize冲突）
	void InitializeMainUI(ADemo01_GM* InGameMode, const FDemo01UIConfig& InUIConfig);
	
	// ===== 城镇面板管理 =====
	
	// 显示城镇面板
	void ShowTownInfoPanel(ATownActor01* Town);
	
	// 隐藏城镇面板
	void HideTownInfoPanel();
	
	// ===== 商队面板管理 =====
	
	// 显示商队面板（通过实例）
	void ShowCaravanInfoPanel(AMoveableEntity01* Caravan);
	
	// 显示商队面板（通过ID）
	void ShowCaravanInfoPanelByID(int32 CaravanID);
	
	// 隐藏商队面板
	void HideCaravanInfoPanel();
	
	// ===== 殖民者面板管理 =====
	
	// 显示殖民者面板（通过实例）
	void ShowColonistInfoPanel(AMoveableEntity01* Colonist);
	
	// 显示殖民者面板（通过ID）
	void ShowColonistInfoPanelByID(int32 ColonistID);
	
	// 隐藏殖民者面板
	void HideColonistInfoPanel();
	
	// ===== 获取面板引用 =====
	
	UTownInfoPanel* GetTownInfoPanel() const { return TownInfoPanel; }
	UCaravanInfoPanel01* GetCaravanInfoPanel() const { return CaravanInfoPanel; }
	UColonistInfoPanel01* GetColonistInfoPanel() const { return ColonistInfoPanel; }
	UPausePanel01* GetPausePanel() const { return PausePanel; }

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 根Canvas（全屏，Hit Test Invisible）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;
	
	// 城镇面板（在蓝图中预先创建并绑定）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTownInfoPanel> TownInfoPanel = nullptr;
	
	// 商队面板（在蓝图中预先创建并绑定）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCaravanInfoPanel01> CaravanInfoPanel = nullptr;
	
	// 殖民者面板（在蓝图中预先创建并绑定）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UColonistInfoPanel01> ColonistInfoPanel = nullptr;
	
	// 暂停面板（在蓝图中预先创建并绑定）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPausePanel01> PausePanel = nullptr;
	
	// ===== 数据 =====
	
	FDemo01UIConfig UIConfig;
};
