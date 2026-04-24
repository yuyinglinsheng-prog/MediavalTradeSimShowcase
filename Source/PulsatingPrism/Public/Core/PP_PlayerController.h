#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "PP_PlayerController.generated.h"

class UPP_InputConfig;
class APP_PlayerPawn;

UCLASS()
class PULSATINGPRISM_API APP_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    APP_PlayerController();

protected:
    // 输入配置
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UPP_InputConfig> InputConfig;

    // 速度控制变量
    float SpeedMultiplier;
    float SpeedBoostMultiplier = 3.0f;
    float SpeedReduceMultiplier = 0.3f;

    // ===== 输入处理函数 =====
    void MoveCamera(const FInputActionValue& Value);
    void PanCamera(const FInputActionValue& Value);
    void RotateCamera(const FInputActionValue& Value);
    void LookCamera(const FInputActionValue& Value);
    void ZoomCamera(const FInputActionValue& Value);
    void AscendCamera(const FInputActionValue& Value);
    void DescendCamera(const FInputActionValue& Value);
    void FocusCamera();
    void ResetCamera();
    void SpeedBoost(const FInputActionValue& Value);
    void SpeedReduce(const FInputActionValue& Value);

public:
    // 蓝图接口
    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetupCameraInput();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetInputConfig(UPP_InputConfig* NewConfig);

    UFUNCTION(BlueprintPure, Category = "Camera")
    APP_PlayerPawn* GetCameraPawn() const;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void FocusOnActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SnapToLocation(FVector Location);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn* InPawn) override;

    void SetupEnhancedInputBindings();

private:
    class UEnhancedInputComponent* CachedEnhancedInput;
};
