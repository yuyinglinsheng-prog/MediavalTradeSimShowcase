#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PP_PlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

/*
当前都是在这里硬编码
1. 相机速度配置位置
在 PP_PlayerPawn.h 中找到了所有相机速度的硬编码配置：

WASD 移动速度：

BaseMoveSpeed = 1000.0f （第 52 行）
SpringArm 缩放速度：

ZoomSpeed = 300.0f （第 59 行）
其他相关速度参数：

PanSensitivity = 0.5f - 鼠标中键拖拽灵敏度
RotateSensitivity = 50.0f - Q/E 旋转速度
LookSensitivity = 0.25f - 鼠标右键旋转灵敏度
HeightAdjustSpeed = 500.0f - R/C 高度调整速度
MoveSmoothingFactor = 8.0f - 移动平滑系数
RotateSmoothingFactor = 6.0f - 旋转平滑系数
ZoomSmoothingFactor = 10.0f - 缩放平滑系数
这些都是 EditDefaultsOnly 属性，可以在蓝图中修改。
*/

UCLASS()
class PULSATINGPRISM_API APP_PlayerPawn : public APawn
{
	GENERATED_BODY()
public:
	APP_PlayerPawn();
protected:
    // 组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* SpringArm;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* Camera;
    // ===== 相机控制接口（由Controller调用）=====
public:
    // 平移控制
    UFUNCTION(BlueprintCallable, Category = "Camera|Movement")
    void MoveCamera(FVector2D MoveInput);
    UFUNCTION(BlueprintCallable, Category = "Camera|Movement")
    void MoveCameraForward(float ForwardInput);
    UFUNCTION(BlueprintCallable, Category = "Camera|Movement")
    void MoveCameraRight(float RightInput);
    UFUNCTION(BlueprintCallable, Category = "Camera|Movement")
    void PanCamera(FVector2D PanInput);
    // 旋转控制
    UFUNCTION(BlueprintCallable, Category = "Camera|Rotation")
    void RotateCamera(float RotateInput);  // QE旋转（围绕Z轴）
    UFUNCTION(BlueprintCallable, Category = "Camera|Rotation")
    void LookCamera(FVector2D LookInput);  // 鼠标右键旋转
    // 缩放与高度控制
    UFUNCTION(BlueprintCallable, Category = "Camera|Zoom")
    void ZoomCamera(float ZoomInput);
    UFUNCTION(BlueprintCallable, Category = "Camera|Zoom")
    void AdjustHeight(float HeightInput);
    // 功能控制
    UFUNCTION(BlueprintCallable, Category = "Camera|Functions")
    void FocusOnTarget(AActor* TargetActor);
    UFUNCTION(BlueprintCallable, Category = "Camera|Functions")
    void ResetCamera();
    UFUNCTION(BlueprintCallable, Category = "Camera|Functions")
    void SetMovementSpeed(float SpeedMultiplier);
protected:
    // ===== 可配置参数 =====
    // 平移参数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Movement")
    float BaseMoveSpeed = 1000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Movement")
    float PanSensitivity = 0.5f;
    // 旋转参数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Rotation")
    float RotateSensitivity = 50.0f;  // 度/秒
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Rotation")
    float LookSensitivity = 0.25f;
    // 缩放参数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Zoom")
    float ZoomSpeed = 1000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Zoom")
    float MinArmLength = 200.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Zoom")
    float MaxArmLength = 3000.0f;
    // 高度参数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Zoom")
    float HeightAdjustSpeed = 500.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Zoom")
    float MinHeight = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Zoom")
    float MaxHeight = 5000.0f;
    // 惯性平滑参数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Smooth")
    float MoveSmoothingFactor = 8.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Smooth")
    float RotateSmoothingFactor = 6.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Smooth")
    float ZoomSmoothingFactor = 10.0f;
    // ===== 运行时变量 =====
    float CurrentMoveSpeed;
    float CurrentMoveSpeedMultiplier;
    FVector TargetLocation;
    float TargetArmLength;
    FRotator TargetRotation;
    float TargetHeight;
    // 默认相机设置（用于重置）
    FRotator DefaultSpringArmRotation;
    float DefaultArmLength;
    float DefaultHeight;
    // 速度控制
    float CurrentSpeedMultiplier;
    bool bIsSpeedBoosted;
    bool bIsSpeedReduced;
    // ===== Tick函数 =====
public:
    virtual void Tick(float DeltaTime) override;
protected:
    virtual void BeginPlay() override;
    void ApplySmoothMovement(float DeltaTime);
    void UpdateCameraPosition();

    // ===== Getter函数 =====
public:
    UFUNCTION(BlueprintPure, Category = "Camera")
    USpringArmComponent* GetCameraSpringArm() const { return SpringArm; }
    UFUNCTION(BlueprintPure, Category = "Camera")
    UCameraComponent* GetCameraComponent() const { return Camera; }
    UFUNCTION(BlueprintPure, Category = "Camera")
    float GetCurrentSpeedMultiplier() const { return CurrentSpeedMultiplier; }
    UFUNCTION(BlueprintPure, Category = "Camera")
    bool GetIsSpeedBoosted() const { return bIsSpeedBoosted; }
    UFUNCTION(BlueprintPure, Category = "Camera")
    bool GetIsSpeedReduced() const { return bIsSpeedReduced; }
};
