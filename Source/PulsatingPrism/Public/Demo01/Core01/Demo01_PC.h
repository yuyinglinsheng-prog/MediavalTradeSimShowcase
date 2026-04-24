// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Demo01DataTypes.h"
#include "Demo01_PC.generated.h"

class UPP_InputConfig;
class APP_PlayerPawn;
class ADemo01_GM;
class ASelectableEntity01;
class UInputAction;
class APathVisualizerActor01;

/**
 * Demo01 PlayerController
 * 处理输入、选中管理、相机控制
 */
UCLASS()
class PULSATINGPRISM_API ADemo01_PC : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADemo01_PC();
	
	// ===== 选中管理（分离城镇和可移动单位选择） =====
	
	// 城镇选择（独立于可移动单位）
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectTown(int32 TownID);
	
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearTownSelection();
	
	// 可移动单位选择（商队/殖民者互斥）
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectMoveableUnit(int32 UnitID, EEntityType01 UnitType);
	
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearMoveableUnitSelection();
	
	// 获取当前选中状态
	UFUNCTION(BlueprintPure, Category = "Selection")
	int32 GetSelectedTownID() const { return SelectedTownID; }
	
	UFUNCTION(BlueprintPure, Category = "Selection")
	int32 GetSelectedMoveableUnitID() const { return SelectedMoveableUnitID; }
	
	UFUNCTION(BlueprintPure, Category = "Selection")
	EEntityType01 GetSelectedMoveableUnitType() const { return SelectedMoveableUnitType; }
	
	// ===== 兼容性接口（保留原有方法） =====
	
	// 选中一个实体（通过引用，兼容旧代码）
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectEntity(FEntityReference01 EntityRef);
	
	// 选中一个实体（通过Actor指针，兼容旧代码）
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectEntityByActor(ASelectableEntity01* Entity);
	
	// 取消选中（兼容旧代码）
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearSelection();
	
	// 获取当前选中的实体引用（兼容旧代码，优先返回可移动单位）
	UFUNCTION(BlueprintPure, Category = "Selection")
	FEntityReference01 GetSelectedEntity() const;
	
	// 获取当前选中的实体实例（可能为nullptr）
	UFUNCTION(BlueprintPure, Category = "Selection")
	ASelectableEntity01* GetSelectedEntityInstance() const;
	
	// 是否有选中的实体
	UFUNCTION(BlueprintPure, Category = "Selection")
	bool HasSelection() const { return SelectedTownID != -1 || SelectedMoveableUnitID != -1; }
	
	// ===== 输入处理 =====
	
	// 左键点击
	void OnLeftMouseClick();
	
	// 右键点击
	void OnRightMouseClick();
	
	// 切换暂停
	void OnTogglePause();
	
	// 射线检测
	AActor* PerformRaycast(FHitResult& OutHit);
	
	// 处理商队移动命令
	void HandleCaravanMoveCommand(class ADemo01_PS* PS, FIntVector2 TargetCoord, bool bAppendPath);
	
	// 处理殖民者移动命令
	void HandleColonistMoveCommand(class ADemo01_PS* PS, FIntVector2 TargetCoord, bool bAppendPath);
	
	// ===== 相机控制（参考 PP_PlayerController） =====
	
	void MoveCamera(const FInputActionValue& Value);
	void MoveCameraForward(const FInputActionValue& Value);
	void MoveCameraRight(const FInputActionValue& Value);
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
	
	// ===== 公共接口 =====
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetupCameraInput();
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputConfig(UPP_InputConfig* NewConfig);
	
	UFUNCTION(BlueprintPure, Category = "Camera")
	APP_PlayerPawn* GetCameraPawn() const;

	// ===== 路径可视化（委托给PathVisualizerActor） =====

	// 显示手动路径（委托给PathVisualizer）
	void ShowManualPath(int32 UnitID, EEntityType01 UnitType);

	// 显示贸易路线（委托给PathVisualizer）
	void ShowTradeRoute(int32 CaravanID);

	// 清除路径（委托给PathVisualizer）
	void ClearPathVisualization();
	
	// ===== Debug =====
	
	// Debug开关：是否绘制射线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugDrawRaycast = false;
	
	// Debug开关：射线绘制持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugRaycastDuration = 2.0f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	void SetupEnhancedInputBindings();
	
	// ===== 输入配置 =====
	
	// 输入配置（复用 PP_InputConfig）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UPP_InputConfig> InputConfig;
	
	// 速度控制
	float SpeedMultiplier = 1.0f;
	float SpeedBoostMultiplier = 3.0f;
	float SpeedReduceMultiplier = 0.3f;

	// ===== 高亮管理 =====
	
	// 更新实体高亮状态
	void UpdateEntityHighlight(int32 EntityID, EEntityType01 EntityType, bool bHighlight);

private:

	// ===== 新的选中状态存储 =====
	
	// 当前选中的城镇ID
	int32 SelectedTownID = -1;
	
	// 当前选中的可移动单位ID
	int32 SelectedMoveableUnitID = -1;
	
	// 当前选中的可移动单位类型
	EEntityType01 SelectedMoveableUnitType = EEntityType01::None;
	
	// ===== 兼容性存储（逐步废弃） =====
	
	// 当前选中的实体引用（保留用于兼容性）
	UPROPERTY()
	FEntityReference01 SelectedEntityRef;
	
	// GameMode 引用
	UPROPERTY()
	ADemo01_GM* GameMode = nullptr;
	
	// Enhanced Input 组件缓存
	class UEnhancedInputComponent* CachedEnhancedInput = nullptr;
};
