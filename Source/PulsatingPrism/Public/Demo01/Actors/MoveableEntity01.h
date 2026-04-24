// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/Actors/SelectableEntity01.h"
#include "MoveableEntity01.generated.h"


/**
 * 可移动实体基类
 * 提供移动系统和路径可视化功能
 * 所有可以在网格上移动的单位都应该继承此类
 */
UCLASS(Blueprintable)
class PULSATINGPRISM_API AMoveableEntity01 : public ASelectableEntity01
{
	GENERATED_BODY()
	
public:
	// ===== 移动控制 =====
	
	// 设置移动路径
	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void SetMovementPath(const TArray<FIntVector2>& Path);
	
	// 开始移动
	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void StartMovement();

	// 单位类型（商队 / 殖民者）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	EEntityType01 UnitType = EEntityType01::None;

	// 视觉移动速度（世界单位/秒），与游戏内天数无关，仅影响实例插值动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float VisualMoveSpeed = 300.0f;
	
	// 停止移动
	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void StopMovement();
	
	// 暂停移动
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void PauseMovement();
	
	// 恢复移动
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ResumeMovement();
	
	// 是否正在移动
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMoving() const { return bIsMoving; }

	// 设置视觉层的下一个目标格子（由数据层推进后通知）
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetVisualMoveTarget(FIntVector2 NewTargetCoord);

	// 视觉层是否已到达当前目标格子
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool HasVisualArrived() const { return bVisualArrived; }
	
	// 获取当前路径
	UFUNCTION(BlueprintPure, Category = "Movement")
	const TArray<FIntVector2>& GetCurrentPath() const { return CurrentPath; }
	
	
protected:
	AMoveableEntity01();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// ===== 子类必须实现的虚函数 =====
	
	// 获取移动速度（从数据层读取，根据UnitType dispatch）
	float GetMoveSpeed() const;
	
	// 到达目标时的回调（只清除路径可视化，游戏逻辑已迁移到UCM）
	virtual void OnArrived();
	
	// 调整Z轴位置以贴合地面
	FVector AdjustZPosition(const FVector& WorldPosition) const;
	
	// ===== 移动相关 =====
	
	// 当前路径
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TArray<FIntVector2> CurrentPath;
	
	// 当前路径索引
	int32 CurrentPathIndex = 0;
	
	// 是否正在移动
	bool bIsMoving = false;
	
	// 是否暂停移动
	bool bIsMovementPaused = false;
	
protected:
	// 获取移动数据（从数据层读取，根据UnitType dispatch）
	// 返回值：bIsMoving, CurrentGridCoord, NextGridCoord, MovementProgress(0-1)
	bool GetMovementData(bool& bOutIsMoving, FIntVector2& OutCurrentCoord, FIntVector2& OutNextCoord, float& OutProgress) const;

	// 点击事件（使用UnitType调用SelectMoveableUnit）
	virtual void OnClicked() override;
	
private:
	// ===== 移动逻辑 =====
	
	// 从数据层读取并更新视觉位置（插值显示）
	void UpdateVisualPosition(float DeltaTime);

	// 根据当前天长与速度档位计算视觉移动速度
	float CalculateVisualMoveSpeed(const FVector& CurrentPos, const FVector& TargetPos) const;
 	
 	// 更新移动（旧方法，保留用于兼容）
 	void UpdateMovement(float DeltaTime);
	
	// 移动插值参数
	float MoveProgress = 0.0f;
	FVector StartLocation;
	FVector TargetLocation;
	
 	// 上一帧数据层的路径索引（用于检测格子到达）
 	int32 LastDataPathIndex = 0;

	// 当前视觉层目标格子（由数据层推进后通知）
	FIntVector2 VisualTargetCoord = FIntVector2::ZeroValue;

	// 视觉层是否已到达目标格子
	bool bVisualArrived = true;

	// 视觉到达位置判断阈值（世界单位）
	float VisualArrivalTolerance = 5.0f;

	// 视觉层希望在半天内跑完一格，剩余时间可播放待机/原地动画
	float VisualTravelDayFraction = 0.5f;
	
 	// ===== 数据层写入辅助 =====
 	void WritePathToData(const TArray<FIntVector2>& Path);
 	void WriteStartToData();
	void WriteStopToData();
};
