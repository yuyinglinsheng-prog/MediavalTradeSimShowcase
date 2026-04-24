#include "Core/PP_PlayerPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"

APP_PlayerPawn::APP_PlayerPawn()
{
    // 创建Root组件
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>("Root");
    RootComponent = Root;

    // 创建SpringArm（类似Editor的相机臂）
    SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));  // 45度俯角
    SpringArm->TargetArmLength = 1000.0f;  // 默认距离
    SpringArm->bDoCollisionTest = false;    // 不检测碰撞
    SpringArm->bEnableCameraLag = true;     // 启用相机延迟
    SpringArm->CameraLagSpeed = 3.0f;
    SpringArm->CameraRotationLagSpeed = 5.0f;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    // 创建相机组件
    Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->FieldOfView = 60.0f;
    Camera->bUsePawnControlRotation = false;

    // 初始化变量
    TargetLocation = GetActorLocation();
    TargetArmLength = SpringArm->TargetArmLength;
    TargetRotation = SpringArm->GetRelativeRotation();
    TargetHeight = GetActorLocation().Z;

    CurrentSpeedMultiplier = 1.0f;
    bIsSpeedBoosted = false;
    bIsSpeedReduced = false;

    // 记录默认设置
    DefaultSpringArmRotation = SpringArm->GetRelativeRotation();
    DefaultArmLength = SpringArm->TargetArmLength;
    DefaultHeight = GetActorLocation().Z;

    // 启用Tick
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void APP_PlayerPawn::BeginPlay()
{
    Super::BeginPlay();

    // 初始化目标位置
    TargetLocation = GetActorLocation();
    TargetRotation = SpringArm->GetRelativeRotation();
    TargetHeight = GetActorLocation().Z;
}

void APP_PlayerPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ApplySmoothMovement(DeltaTime);
}

// ===== 相机控制实现 =====

void APP_PlayerPawn::MoveCamera(FVector2D MoveInput)
{
    if (FMath::IsNearlyZero(MoveInput.SizeSquared()))
    {
        return;
    }

    // 计算移动方向（基于当前旋转）
    FRotator YawRotation(0.0f, SpringArm->GetComponentRotation().Yaw, 0.0f);
    FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // 计算移动偏移
    float BaseSpeed = BaseMoveSpeed * CurrentSpeedMultiplier;
    FVector MoveOffset = (ForwardVector * MoveInput.Y + RightVector * MoveInput.X) * BaseSpeed * GetWorld()->GetDeltaSeconds();

    TargetLocation += MoveOffset;
    TargetHeight = TargetLocation.Z;  // 同步高度
}

void APP_PlayerPawn::MoveCameraForward(float ForwardInput)
{
    if (FMath::IsNearlyZero(ForwardInput))
    {
        return;
    }

    // 计算前进方向（基于当前旋转）
    FRotator YawRotation(0.0f, SpringArm->GetComponentRotation().Yaw, 0.0f);
    FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // 计算移动偏移
    float BaseSpeed = BaseMoveSpeed * CurrentSpeedMultiplier;
    FVector MoveOffset = ForwardVector * ForwardInput * BaseSpeed * GetWorld()->GetDeltaSeconds();

    TargetLocation += MoveOffset;
    TargetHeight = TargetLocation.Z;
}

void APP_PlayerPawn::MoveCameraRight(float RightInput)
{
    if (FMath::IsNearlyZero(RightInput))
    {
        return;
    }

    // 计算右移方向（基于当前旋转）
    FRotator YawRotation(0.0f, SpringArm->GetComponentRotation().Yaw, 0.0f);
    FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // 计算移动偏移
    float BaseSpeed = BaseMoveSpeed * CurrentSpeedMultiplier;
    FVector MoveOffset = RightVector * RightInput * BaseSpeed * GetWorld()->GetDeltaSeconds();

    TargetLocation += MoveOffset;
    TargetHeight = TargetLocation.Z;
}

void APP_PlayerPawn::PanCamera(FVector2D PanInput)
{
    // 鼠标中键拖拽（屏幕空间平移）
    if (FMath::IsNearlyZero(PanInput.SizeSquared())) return;

    // 基于屏幕平面计算平移
    FVector ScreenRight = Camera->GetRightVector();
    FVector ScreenUp = Camera->GetUpVector();

    float PanSpeed = BaseMoveSpeed * PanSensitivity * CurrentSpeedMultiplier * GetWorld()->GetDeltaSeconds();
    FVector PanOffset = (ScreenRight * PanInput.X + ScreenUp * PanInput.Y) * PanSpeed;

    TargetLocation += PanOffset;
    TargetHeight = TargetLocation.Z;
}

void APP_PlayerPawn::RotateCamera(float RotateInput)
{
    // Q/E旋转（围绕Z轴）
    if (FMath::IsNearlyZero(RotateInput)) return;

    float RotateAmount = RotateInput * RotateSensitivity * GetWorld()->GetDeltaSeconds();
    TargetRotation.Yaw += RotateAmount;
}

void APP_PlayerPawn::LookCamera(FVector2D LookInput)
{
    // 鼠标右键旋转（直接控制旋转）
    if (FMath::IsNearlyZero(LookInput.SizeSquared())) return;

    float LookScale = LookSensitivity * 100.0f;
    TargetRotation.Yaw += LookInput.X * LookScale * GetWorld()->GetDeltaSeconds();
    TargetRotation.Pitch = FMath::Clamp(
        TargetRotation.Pitch + LookInput.Y * LookScale * GetWorld()->GetDeltaSeconds(),
        -89.0f, 0.0f  // 限制俯仰角
    );
}

void APP_PlayerPawn::ZoomCamera(float ZoomInput)
{
    if (FMath::IsNearlyZero(ZoomInput)) return;

    float ZoomAmount = ZoomInput * ZoomSpeed * GetWorld()->GetDeltaSeconds();
    TargetArmLength = FMath::Clamp(TargetArmLength + ZoomAmount, MinArmLength, MaxArmLength);
}

void APP_PlayerPawn::AdjustHeight(float HeightInput)
{
    if (FMath::IsNearlyZero(HeightInput)) return;

    float HeightAmount = HeightInput * HeightAdjustSpeed * GetWorld()->GetDeltaSeconds();
    TargetHeight = FMath::Clamp(TargetHeight + HeightAmount, MinHeight, MaxHeight);
    TargetLocation.Z = TargetHeight;
}

void APP_PlayerPawn::FocusOnTarget(AActor* TargetActor)
{
    if (!TargetActor) return;

    // 平滑移动到目标
    TargetLocation = TargetActor->GetActorLocation();
    TargetLocation.Z = TargetHeight;  // 保持当前高度
}

void APP_PlayerPawn::ResetCamera()
{
    // 重置到默认位置
    TargetLocation.Z = DefaultHeight;
    TargetRotation = DefaultSpringArmRotation;
    TargetArmLength = DefaultArmLength;
}

void APP_PlayerPawn::SetMovementSpeed(float SpeedMultiplier)
{
    CurrentSpeedMultiplier = FMath::Clamp(SpeedMultiplier, 0.1f, 10.0f);
}

void APP_PlayerPawn::ApplySmoothMovement(float DeltaTime)
{
    // 位置平滑
    FVector CurrentLocation = GetActorLocation();
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, MoveSmoothingFactor);
    SetActorLocation(NewLocation);

    // 旋转平滑
    FRotator CurrentRotation = SpringArm->GetRelativeRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotateSmoothingFactor);
    SpringArm->SetRelativeRotation(NewRotation);

    // 缩放平滑
    float CurrentLength = SpringArm->TargetArmLength;
    float NewLength = FMath::FInterpTo(CurrentLength, TargetArmLength, DeltaTime, ZoomSmoothingFactor);
    SpringArm->TargetArmLength = NewLength;

    // 高度约束
    if (NewLocation.Z < MinHeight)
    {
        NewLocation.Z = MinHeight;
        SetActorLocation(NewLocation);
        TargetLocation.Z = MinHeight;
    }
    else if (NewLocation.Z > MaxHeight)
    {
        NewLocation.Z = MaxHeight;
        SetActorLocation(NewLocation);
        TargetLocation.Z = MaxHeight;
    }

    TargetHeight = TargetLocation.Z;
}

void APP_PlayerPawn::UpdateCameraPosition()
{
}
