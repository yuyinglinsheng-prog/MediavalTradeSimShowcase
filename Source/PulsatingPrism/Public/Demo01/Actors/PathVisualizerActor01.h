// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "PathVisualizerActor01.generated.h"

class USplineComponent;
class USplineMeshComponent;
class ADemo01_GM;
class AMoveableEntity01;

/**
 * 路径类型枚举
 */
UENUM(BlueprintType)
enum class EPathType01 : uint8
{
	None UMETA(DisplayName = "无"),
	Manual UMETA(DisplayName = "手动路径"),
	TradeRoute UMETA(DisplayName = "贸易路线")
};

/**
 * 路径可视化Actor - 专门用于显示单位移动路径
 * 由GameMode创建和管理
 */
UCLASS(Blueprintable)
class PULSATINGPRISM_API APathVisualizerActor01 : public AActor
{
	GENERATED_BODY()
	
public:
	APathVisualizerActor01();

	// 显示手动移动路径（带动画）
	UFUNCTION(BlueprintCallable, Category = "PathVisualization")
	void ShowManualPath(int32 UnitID, EEntityType01 UnitType);

	// 显示贸易路线（闭环，无动画）
	UFUNCTION(BlueprintCallable, Category = "PathVisualization")
	void ShowTradeRoute(int32 CaravanID);

	// 清除所有路径
	UFUNCTION(BlueprintCallable, Category = "PathVisualization")
	void ClearPathVisualization();

	// 清除贸易路线
	UFUNCTION(BlueprintCallable, Category = "PathVisualization")
	void ClearTradeRoute();

	// 清除手动路径
	UFUNCTION(BlueprintCallable, Category = "PathVisualization")
	void ClearManualPath();

	// 更新路径起点（跟随单位移动）
	UFUNCTION(BlueprintCallable, Category = "PathVisualization")
	void UpdatePathStartPoint();

	// ===== 手动路径配置 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Manual Path")
	TObjectPtr<UStaticMesh> PathArrowMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Manual Path")
	TObjectPtr<UMaterialInterface> PathArrowMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Manual Path")
	float PathExtensionSpeed = 10.0f;

	// ===== 贸易路线配置 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trade Route")
	TObjectPtr<UStaticMesh> TradeRouteMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trade Route")
	TObjectPtr<UMaterialInterface> TradeRouteMaterial;

	// 获取当前路径类型
	UFUNCTION(BlueprintPure, Category = "PathVisualization")
	EPathType01 GetCurrentPathType() const { return CurrentPathType; }

protected:
	virtual void Tick(float DeltaTime) override;

private:
	// 初始化手动路径显示
	void InitializeManualPathDisplay(const TArray<FVector>& PathPoints, AMoveableEntity01* TrackedEntity, int32 StartIndex);

	// 路径延伸动画
	void UpdatePathExtension(float RealDeltaTime);

	// 删除已走过的路径节点
	void TrimPassedWaypoints();

	// 重建手动路径SplineMesh组件
	void RebuildManualPathSplineMeshes();

	// 重建贸易路线SplineMesh组件
	void RebuildTradeRouteSplineMeshes(const TArray<FVector>& PathPoints);

	// ===== 手动路径组件 =====
	UPROPERTY()
	TObjectPtr<USplineComponent> ManualPathSpline;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> ManualPathSplineMeshes;

	// ===== 贸易路线组件 =====
	UPROPERTY()
	TObjectPtr<USplineComponent> TradeRouteSpline;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> TradeRouteSplineMeshes;

	// 完整路径点缓存（手动路径用）
	TArray<FVector> ManualPathPoints;

	// 当前追踪的实体实例
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> TrackedInstance;

	// 手动路径动画状态
	bool bIsExtendingPath = false;
	float ExtensionProgress = 0.0f;
	float LastRealTime = 0.0f;

	// 上一帧记录的数据层路径索引
	int32 LastPathIndex = 0;

	// 当前显示的路径类型
	EPathType01 CurrentPathType = EPathType01::None;

	// 当前显示的单位ID和类型
	int32 CurrentUnitID = -1;
	EEntityType01 CurrentUnitType = EEntityType01::None;

	// 延迟销毁的组件列表（避免迭代器失效）
	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> PendingDestroyMeshes;
};
