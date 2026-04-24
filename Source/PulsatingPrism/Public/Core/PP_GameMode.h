// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GrandStrategySimulationCore/FrameworkBootstrap.h"
#include "CreationTools/WorldGenerationManager.h"
#include "PP_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class PULSATINGPRISM_API APP_GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	APP_GameMode();

protected:
	virtual void BeginPlay() override;

public:
	// 核心API：创建新游戏世界
	UFUNCTION(BlueprintCallable, Category = "World Generation")
	void CreateNewWorld();

	// 测试API：快速创建一个测试世界（用于开发环境）
	void CreateTestWorld();

private:
    // 子系统引用
    UPROPERTY()
    UWorldGenerationManager* WorldGenerator = nullptr;

    UPROPERTY()
    UFrameworkBootstrap* FrameworkBootstrap = nullptr;

    // 核心流程
    bool InitSubsystems();
    UFUNCTION()
    void OnWorldGenerationComplete(UWorldDataStore* GeneratedWorld);
    UFUNCTION()
    void OnWorldGenerationFailed(const FString& ErrorMessage);
    void StartFrameworkAfterWorldReady();

    // 日志辅助
    void Log(const FString& Message) const;
};
