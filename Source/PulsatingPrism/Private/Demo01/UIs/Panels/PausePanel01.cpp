// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/PausePanel01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/TimeManager01.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UPausePanel01::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定按钮事件
	if (TogglePauseButton)
	{
		TogglePauseButton->OnClicked.AddDynamic(this, &UPausePanel01::OnTogglePauseClicked);
	}
	if (Speed1xButton)
	{
		Speed1xButton->OnClicked.AddDynamic(this, &UPausePanel01::OnSpeed1xClicked);
	}
	if (Speed2xButton)
	{
		Speed2xButton->OnClicked.AddDynamic(this, &UPausePanel01::OnSpeed2xClicked);
	}
	if (Speed4xButton)
	{
		Speed4xButton->OnClicked.AddDynamic(this, &UPausePanel01::OnSpeed4xClicked);
	}
	
	// 启动定时器，每0.1秒更新一次UI
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			UpdateTimerHandle,
			this,
			&UPausePanel01::OnUpdateTimer,
			0.1f,  // 每0.1秒更新一次
			true   // 循环
		);
		UE_LOG(LogTemp, Log, TEXT("[PausePanel] 定时器已启动"));
	}
}

void UPausePanel01::NativeDestruct()
{
	// 清理定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("[PausePanel] 定时器已清理"));
	}
	
	Super::NativeDestruct();
}

void UPausePanel01::InitializePanel(ADemo01_GM* InGameMode)
{
	GameMode = InGameMode;
	
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[PausePanel] GameMode为空"));
		return;
	}
	
	// 初始化UI
	UpdateUI();
	
	UE_LOG(LogTemp, Log, TEXT("[PausePanel] 面板初始化完成"));
}

void UPausePanel01::RefreshPanel()
{
	UpdateUI();
}

void UPausePanel01::TogglePause()
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[PausePanel] GameMode为空，无法切换暂停状态"));
		return;
	}
	
	UTimeManager01* TimeManager = GameMode->GetTimeManager();
	if (!TimeManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[PausePanel] TimeManager为空"));
		return;
	}
	
	TimeManager->TogglePause();
	
	// 更新UI
	UpdateUI();
	
	// 触发蓝图事件
	OnPauseStateChanged(TimeManager->IsPaused());
	
	UE_LOG(LogTemp, Log, TEXT("[PausePanel] 暂停状态已切换: %s"), 
		TimeManager->IsPaused() ? TEXT("暂停") : TEXT("运行"));
}

void UPausePanel01::OnTogglePauseClicked()
{
	TogglePause();
}

void UPausePanel01::UpdateUI()
{
	if (!GameMode)
	{
		return;
	}
	
	UTimeManager01* TimeManager = GameMode->GetTimeManager();
	if (!TimeManager)
	{
		return;
	}
	
	bool bIsPaused = TimeManager->IsPaused();
	
	// 更新按钮文本
	if (TogglePauseButtonText)
	{
		FText ButtonText = bIsPaused ? FText::FromString(TEXT("恢复游戏")) : FText::FromString(TEXT("暂停游戏"));
		TogglePauseButtonText->SetText(ButtonText);
	}
	
	// 更新状态文本
	if (StatusText)
	{
		FText Status = bIsPaused ? FText::FromString(TEXT("游戏已暂停")) : FText::FromString(TEXT("游戏运行中"));
		StatusText->SetText(Status);
	}
	
	// 更新游戏时间（如果有）
	if (GameTimeText)
	{
		float GameTime = TimeManager->GetGameTime();
		FText TimeText = FText::FromString(FString::Printf(TEXT("游戏时间: %.1f 秒"), GameTime));
		GameTimeText->SetText(TimeText);
	}
	
	// 更新游戏日期（如果有）
	if (GameDayText)
	{
		int32 GameDay = TimeManager->GetGameDay();
		FText DayText = FText::FromString(FString::Printf(TEXT("第 %d 天"), GameDay));
		GameDayText->SetText(DayText);
	}

	// 更新当前速度显示（如果有）
	if (GameSpeedText)
	{
		float Speed = TimeManager->GetGameSpeed();
		FText SpeedText = FText::FromString(FString::Printf(TEXT("速度: %.0fx"), Speed));
		GameSpeedText->SetText(SpeedText);
	}
}

void UPausePanel01::OnSpeed1xClicked()
{
	if (UTimeManager01* TM = GameMode ? GameMode->GetTimeManager() : nullptr)
	{
		TM->SetGameSpeed(1.0f);
		UpdateUI();
	}
}

void UPausePanel01::OnSpeed2xClicked()
{
	if (UTimeManager01* TM = GameMode ? GameMode->GetTimeManager() : nullptr)
	{
		TM->SetGameSpeed(2.0f);
		UpdateUI();
	}
}

void UPausePanel01::OnSpeed4xClicked()
{
	if (UTimeManager01* TM = GameMode ? GameMode->GetTimeManager() : nullptr)
	{
		TM->SetGameSpeed(4.0f);
		UpdateUI();
	}
}

void UPausePanel01::OnUpdateTimer()
{
	// 定时器回调 - 更新UI显示
	UpdateUI();
}
