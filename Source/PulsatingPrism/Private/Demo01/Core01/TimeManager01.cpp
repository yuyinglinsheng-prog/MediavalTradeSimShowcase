// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/TimeManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "Demo01/Core01/UnitControlManager01.h"
#include "Demo01/Core01/TradeRouteManager01.h"

void UTimeManager01::Initialize(ADemo01_GM* InGameMode)
{
	GameMode = InGameMode;
	TimeState = EGameTimeState01::Playing;
	GameTime = 0.0f;
	GameDay = 0;
	GameSpeedMultiplier = 1.0f;

	UE_LOG(LogTemp, Log, TEXT("[时间管理器] 初始化完成，SecondsPerDay: %.1f"), SecondsPerDay);
}

void UTimeManager01::Execute(float DeltaTime)
{
	if (TimeState == EGameTimeState01::Paused)
	{
		return;
	}

	GameTime += DeltaTime * GameSpeedMultiplier;
	while (GameTime >= SecondsPerDay)
	{
		GameTime -= SecondsPerDay;
		OnDayTick();
	}
}

void UTimeManager01::OnDayTick()
{
	GameDay++;

	UE_LOG(LogTemp, Log, TEXT("[时间管理器] 第 %d 天开始"), GameDay);

	// 驱动所有单位的数据层移动（整整一天）
	if (GameMode && GameMode->GetUnitControlManager())
	{
		GameMode->GetUnitControlManager()->UpdateAllMovements(1.0f);
	}
	
	// 驱动自动贸易（新增）
	if (GameMode && GameMode->GetTradeRouteManager())
	{
		GameMode->GetTradeRouteManager()->UpdateTradeRoutes(1.0f);
	}

	// 通知 GameMode 新的一天（城镇生产等）
	if (GameMode)
	{
		GameMode->OnGameDayPassed();
	}
}

void UTimeManager01::PauseGame()
{
	if (TimeState == EGameTimeState01::Paused)
	{
		return;
	}

	TimeState = EGameTimeState01::Paused;
	UE_LOG(LogTemp, Log, TEXT("[时间管理器] 游戏暂停"));
}

void UTimeManager01::ResumeGame()
{
	if (TimeState == EGameTimeState01::Playing)
	{
		return;
	}

	TimeState = EGameTimeState01::Playing;
	UE_LOG(LogTemp, Log, TEXT("[时间管理器] 游戏恢复"));
}

void UTimeManager01::SetGameSpeed(float Multiplier)
{
	GameSpeedMultiplier = FMath::Max(0.1f, Multiplier);
	UE_LOG(LogTemp, Log, TEXT("[时间管理器] 游戏速度设为 %.2fx"), GameSpeedMultiplier);
}

void UTimeManager01::TogglePause()
{
	if (TimeState == EGameTimeState01::Paused)
	{
		ResumeGame();
	}
	else
	{
		PauseGame();
	}
}

