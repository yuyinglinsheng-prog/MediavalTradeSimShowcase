#include "Core/PP_PlayerController.h"
#include "Core/PP_PlayerPawn.h"
#include "Core/Input/PP_InputConfig.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"

APP_PlayerController::APP_PlayerController()
{
    // 初始化变量
    SpeedMultiplier = 1.0f;
    CachedEnhancedInput = nullptr;

    // 设置默认输入模式适合战略游戏
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    // 设置鼠标光标
    DefaultMouseCursor = EMouseCursor::Default;
   
}

void APP_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 输入模式 - 游戏和UI混合
    SetInputMode(FInputModeGameAndUI());

    // 确保在LocalController上设置输入
    if (IsLocalController())
    {
        SetupCameraInput();
    }
}

void APP_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // 获取EnhancedInput组件
    CachedEnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);

    // 如果还没有CachedEnhancedInput，创建它
    if (!CachedEnhancedInput && InputComponent)
    {
        CachedEnhancedInput = NewObject<UEnhancedInputComponent>(this);
        InputComponent = CachedEnhancedInput;
    }
}

void APP_PlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 确保输入系统在拥有Pawn后正确设置
    if (IsLocalController())
    {
        SetupCameraInput();
    }
}

void APP_PlayerController::SetupCameraInput()
{
    // 获取本地玩家子系统
    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get LocalPlayer"));
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!InputSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get EnhancedInputLocalPlayerSubsystem"));
        return;
    }

    // 清理现有映射
    InputSubsystem->ClearAllMappings();

    // 应用输入配置
    if (InputConfig && InputConfig->DefaultMappingContext)
    {
        InputSubsystem->AddMappingContext(InputConfig->DefaultMappingContext, 0);
        UE_LOG(LogTemp, Log, TEXT("Added default input mapping context"));
    }

    // 设置EnhancedInput绑定
    SetupEnhancedInputBindings();
}

void APP_PlayerController::SetupEnhancedInputBindings()
{
    if (!CachedEnhancedInput || !InputConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("CachedEnhancedInput or InputConfig is null"));
        return;
    }

    // 清理旧的绑定
    CachedEnhancedInput->ClearActionBindings();

    // ===== 绑定所有输入动作 =====

    // 1. WASD移动
    if (InputConfig->CameraMoveAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraMoveAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::MoveCamera
        );
        UE_LOG(LogTemp, Log, TEXT("Bound CameraMoveAction"));
    }

    // 2. 鼠标中键拖拽平移
    if (InputConfig->CameraPanAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraPanAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::PanCamera
        );
        UE_LOG(LogTemp, Log, TEXT("Bound CameraPanAction"));
    }

    // 3. QE旋转
    if (InputConfig->CameraRotateAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraRotateAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::RotateCamera
        );
    }

    // 4. 鼠标右键旋转视角
    if (InputConfig->CameraLookAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraLookAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::LookCamera
        );
    }

    // 5. 滚轮缩放
    if (InputConfig->CameraZoomAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraZoomAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::ZoomCamera
        );
    }

    // 6. 升高高度
    if (InputConfig->CameraAscendAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraAscendAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::AscendCamera
        );
    }

    // 7. 降低高度
    if (InputConfig->CameraDescendAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraDescendAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::DescendCamera
        );
    }

    // 8. 聚焦目标
    if (InputConfig->CameraFocusAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraFocusAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::FocusCamera
        );
    }

    // 9. 重置相机
    if (InputConfig->CameraResetAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->CameraResetAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::ResetCamera
        );
    }

    // 10. 加速（Shift）
    if (InputConfig->SpeedBoostAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->SpeedBoostAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::SpeedBoost
        );

        CachedEnhancedInput->BindAction(
            InputConfig->SpeedBoostAction,
            ETriggerEvent::Completed,
            this,
            &APP_PlayerController::SpeedBoost
        );
    }

    // 11. 减速（Alt）
    if (InputConfig->SpeedReduceAction)
    {
        CachedEnhancedInput->BindAction(
            InputConfig->SpeedReduceAction,
            ETriggerEvent::Triggered,
            this,
            &APP_PlayerController::SpeedReduce
        );

        CachedEnhancedInput->BindAction(
            InputConfig->SpeedReduceAction,
            ETriggerEvent::Completed,
            this,
            &APP_PlayerController::SpeedReduce
        );
    }
}

// ===== 输入处理函数的具体实现 =====

void APP_PlayerController::MoveCamera(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    CameraPawn->MoveCamera(Value.Get<FVector2D>());
}

void APP_PlayerController::PanCamera(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    CameraPawn->PanCamera(Value.Get<FVector2D>());
}

void APP_PlayerController::RotateCamera(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    CameraPawn->RotateCamera(Value.Get<float>());
}

void APP_PlayerController::LookCamera(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    CameraPawn->LookCamera(Value.Get<FVector2D>());
}

void APP_PlayerController::ZoomCamera(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    CameraPawn->ZoomCamera(Value.Get<float>());
}

void APP_PlayerController::AscendCamera(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    float AscendInput = Value.Get<float>();
    if (AscendInput > 0.0f)
    {
        CameraPawn->AdjustHeight(AscendInput);
    }
}

void APP_PlayerController::DescendCamera(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    float DescendInput = Value.Get<float>();
    if (DescendInput < 0.0f)
    {
        CameraPawn->AdjustHeight(DescendInput);
    }
}

void APP_PlayerController::FocusCamera()
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    // 这里获取当前选中的Actor，您需要实现自己的选择系统
    // 这是一个示例实现
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    if (HitResult.GetActor())
    {
        CameraPawn->FocusOnTarget(HitResult.GetActor());
    }
}

void APP_PlayerController::ResetCamera()
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    CameraPawn->ResetCamera();
}

void APP_PlayerController::SpeedBoost(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    bool bBoostActive = Value.Get<bool>();
    if (bBoostActive)
    {
        CameraPawn->SetMovementSpeed(SpeedBoostMultiplier);
    }
    else
    {
        CameraPawn->SetMovementSpeed(1.0f);
    }
}

void APP_PlayerController::SpeedReduce(const FInputActionValue& Value)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    bool bReduceActive = Value.Get<bool>();
    if (bReduceActive)
    {
        CameraPawn->SetMovementSpeed(SpeedReduceMultiplier);
    }
    else
    {
        CameraPawn->SetMovementSpeed(1.0f);
    }
}

// ===== 公共接口实现 =====

void APP_PlayerController::SetInputConfig(UPP_InputConfig* NewConfig)
{
    if (!NewConfig) return;

    InputConfig = NewConfig;

    // 重新设置输入
    if (IsLocalController())
    {
        SetupCameraInput();
    }
}

APP_PlayerPawn* APP_PlayerController::GetCameraPawn() const
{
    return Cast<APP_PlayerPawn>(GetPawn());
}

void APP_PlayerController::FocusOnActor(AActor* TargetActor)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn || !TargetActor) return;

    CameraPawn->FocusOnTarget(TargetActor);
}

void APP_PlayerController::SnapToLocation(FVector Location)
{
    APP_PlayerPawn* CameraPawn = GetCameraPawn();
    if (!CameraPawn) return;

    // 直接设置位置（无平滑）
    CameraPawn->SetActorLocation(Location);
}
