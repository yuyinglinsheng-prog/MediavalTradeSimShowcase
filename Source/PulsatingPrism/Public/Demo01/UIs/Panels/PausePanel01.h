// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "PausePanel01.generated.h"

class UButton;
class UTextBlock;
class ADemo01_GM;
class UTimeManager01;

/**
 * 暂停面板
 * 显示游戏暂停状态，提供暂停/恢复控制
 */
UCLASS()
class PULSATINGPRISM_API UPausePanel01 : public UDemo01PanelBase
{
	GENERATED_BODY()
	
public:
	// 初始化面板
	void InitializePanel(ADemo01_GM* InGameMode);
	
	// 刷新面板显示
	UFUNCTION(BlueprintCallable, Category = "Pause Panel")
	void RefreshPanel();
	
	// 切换暂停状态
	UFUNCTION(BlueprintCallable, Category = "Pause Panel")
	void TogglePause();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	// ===== UI组件绑定 =====
	
	// 暂停/恢复按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TogglePauseButton = nullptr;
	
	// 按钮文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TogglePauseButtonText = nullptr;
	
	// 状态文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText = nullptr;
	
	// 游戏时间文本（可选）
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameTimeText = nullptr;
	
	// 游戏日期文本（可选）
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameDayText = nullptr;

	// 当前速度文本（可选）
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameSpeedText = nullptr;

	// 速度按钮（可选）
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Speed1xButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Speed2xButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Speed4xButton = nullptr;

	// ===== 按钮事件 =====
	
	UFUNCTION()
	void OnTogglePauseClicked();

	UFUNCTION()
	void OnSpeed1xClicked();

	UFUNCTION()
	void OnSpeed2xClicked();

	UFUNCTION()
	void OnSpeed4xClicked();
	
	// ===== 蓝图事件 =====
	
	// 暂停状态改变时触发
	UFUNCTION(BlueprintImplementableEvent, Category = "Pause Panel")
	void OnPauseStateChanged(bool bIsPaused);

private:
	// 更新UI显示
	void UpdateUI();
	
	// 定时器句柄
	FTimerHandle UpdateTimerHandle;
	
	// 定时器回调 - 定期更新UI
	void OnUpdateTimer();
};
