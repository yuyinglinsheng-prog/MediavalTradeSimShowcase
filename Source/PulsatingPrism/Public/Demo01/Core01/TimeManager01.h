// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "TimeManager01.generated.h"

class ADemo01_GM;

/**
 * 游戏时间状态枚举
 */
UENUM(BlueprintType)
enum class EGameTimeState01 : uint8
{
	Paused      UMETA(DisplayName = "暂停"),
	Playing     UMETA(DisplayName = "播放")
};

/**
 * 时间管理器
 * 管理游戏时间和暂停状态，作为 UObject 在 GameMode 中使用
 */
UCLASS()
class PULSATINGPRISM_API UTimeManager01 : public UObject
{
	GENERATED_BODY()
	
public:
	// 初始化
	void Initialize(ADemo01_GM* InGameMode);
	
	// Execute 函数（在 GameMode 的 Tick 中调用）
	UFUNCTION(BlueprintCallable, Category = "Time")
	void Execute(float DeltaTime);
	
	// ===== 时间控制 =====
	
	// 暂停游戏
	UFUNCTION(BlueprintCallable, Category = "Time")
	void PauseGame();
	
	// 恢复游戏
	UFUNCTION(BlueprintCallable, Category = "Time")
	void ResumeGame();
	
	// 切换暂停状态
	UFUNCTION(BlueprintCallable, Category = "Time")
	void TogglePause();

	// 设置游戏速度倍数（1.0=正常，2.0=两倍速，0.5=半速）
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetGameSpeed(float Multiplier);

	// 获取当前游戏速度倍数
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetGameSpeed() const { return GameSpeedMultiplier; }

	// 获取一天的基准秒数
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetSecondsPerDay() const { return SecondsPerDay; }
	
	// 是否暂停
	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsPaused() const { return TimeState == EGameTimeState01::Paused; }
	
	// 获取当前时间状态
	UFUNCTION(BlueprintPure, Category = "Time")
	EGameTimeState01 GetTimeState() const { return TimeState; }
	
	// ===== 游戏时间 =====
	
	// 获取游戏时间（秒）
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetGameTime() const { return GameTime; }
	
	// 获取游戏日期（天数）
	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetGameDay() const { return GameDay; }
	
private:
	UPROPERTY()
	ADemo01_GM* GameMode = nullptr;
	
	// 时间状态
	EGameTimeState01 TimeState = EGameTimeState01::Playing;
	
	// 游戏时间（秒）
	float GameTime = 0.0f;
	
	// 游戏日期（天数）
	int32 GameDay = 0;
	
	// 一天的秒数（固定基准，不随速度变化）
	float SecondsPerDay = 10.0f;

	// 游戏速度倍数（乘在 DeltaTime 上，不修改 SecondsPerDay）
	float GameSpeedMultiplier = 1.0f;

	// 每天结束时触发：推进天数、驱动数据层移动
	void OnDayTick();
};
