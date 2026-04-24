// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PP_GameMode.h"

APP_GameMode::APP_GameMode()
{

}

void APP_GameMode::BeginPlay() 
{
	Super::BeginPlay();

    Log(TEXT("GameMode BeginPlay: Ready to create world"));

    // 如果是开发环境，自动创建测试世界
#if WITH_EDITOR
    CreateTestWorld();
#endif

    //无论是生成世界还是启动运行时框架的函数，都需要订阅事件调用，不要忘了
}

void APP_GameMode::CreateNewWorld()
{
    if (!GetWorld())
    {
        Log(TEXT("ERROR: No valid World context"));
        return;
    }
    Log(TEXT("== Starting World Creation Process =="));

    // 第一步：获取子系统
    if (!InitSubsystems())
    {
        Log(TEXT("ERROR: Failed to initialize subsystems"));
        return;
    }

    WorldGenerator->OnGenerationComplete.AddUniqueDynamic(this, &APP_GameMode::OnWorldGenerationComplete);
    WorldGenerator->OnGenerationFailed.AddUniqueDynamic(this, &APP_GameMode::OnWorldGenerationFailed);

    // 第二步：启动世界生成（同步）
    if (!WorldGenerator->Initialize(EWorldGeneratorState::Idle))
    {
        Log(TEXT("ERROR: Failed to initialize WorldGenerator"));
        return;
    }
    // 第三步：开始生成流程（状态机驱动）
    Log(TEXT("Starting world generation state machine..."));
    WorldGenerator->ChangeState(EWorldGeneratorState::CreatingDataStore);
}

void APP_GameMode::CreateTestWorld()
{
    Log(TEXT("Creating test world for development..."));
    CreateNewWorld();
}

bool APP_GameMode::InitSubsystems()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    // 1. 获取FrameworkBootstrap
    FrameworkBootstrap = World->GetSubsystem<UFrameworkBootstrap>();
    if (!FrameworkBootstrap)
    {
        Log(TEXT("ERROR: Cannot get FrameworkBootstrap subsystem"));
        return false;
    }
    Log(TEXT("FrameworkBootstrap available"));
    // 2. 创建世界生成器（需要指定Outer）
    WorldGenerator = NewObject<UWorldGenerationManager>(this);
    if (!WorldGenerator)
    {
        Log(TEXT("ERROR: Cannot create WorldGenerationManager"));
        return false;
    }

    Log(TEXT("WorldGenerationManager created"));
    return true;
}

void APP_GameMode::OnWorldGenerationComplete(UWorldDataStore* GeneratedWorld)
{
    if (!GeneratedWorld)
    {
        Log(TEXT("ERROR: Received null world data from generator"));
        return;
    }

    Log(TEXT("World generation complete!"));
	StartFrameworkAfterWorldReady();
    //TODO:输出世界生成完成信息,或者还有些别的业务
}

void APP_GameMode::OnWorldGenerationFailed(const FString& ErrorMessage)
{
    //TODO:输出错误信息，可能还要重新启动生成之类的
	Log(FString::Printf(TEXT("World generation failed: %s"), *ErrorMessage));
}

void APP_GameMode::StartFrameworkAfterWorldReady()
{
    if (!FrameworkBootstrap)
    {
        Log(TEXT("ERROR: FrameworkBootstrap not available"));
        return;
    }
    Log(TEXT("Starting game framework with generated world data..."));

    // 获取DataStoreManager验证世界数据
    if (UWorld* World = GetWorld())
    {
        if (UDataStoreManager* DataStoreManager = World->GetSubsystem<UDataStoreManager>())
        {
            // 尝试获取生成的世界数据
            UWorldDataStore* WorldData = DataStoreManager->GetDataStore<UWorldDataStore>(FName("WorldDataStore"));
            if (WorldData)
            {
                Log(FString::Printf(TEXT("WorldDataStore available with %d tiles"), WorldData->GetTileCount()));

                // 现在启动框架
                if (FrameworkBootstrap)
                {
                    FrameworkBootstrap->InitializeFramework();
                    Log(TEXT("Framework initialized and running"));
                    Log(TEXT("=== GAME IS READY TO PLAY ==="));
                }
            }
            else
            {
                Log(TEXT("ERROR: WorldDataStore not found after generation"));
            }
        }
    }
}

void APP_GameMode::Log(const FString& Message) const
{

}