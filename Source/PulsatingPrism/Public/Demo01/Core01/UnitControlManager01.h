// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnitControlManager01.generated.h"

// 前向声明
class ADemo01_GM;
class AMoveableEntity01;
class ATownActor01;
enum class EEntityType01 : uint8;

/**
 * 单位控制管理器
 * 管理所有单位（商队、殖民者）的移动控制逻辑
 * 包括移动命令处理、实例创建等
 */
UCLASS()
class PULSATINGPRISM_API UUnitControlManager01 : public UObject
{
	GENERATED_BODY()
	
public:
	// 初始化
	void Initialize(ADemo01_GM* InGameMode);
	
	// ===== 每帧驱动（由 TimeManager 调用） =====
	
	// 更新所有移动中实体的数据层状态，并在需要时创建/销毁实体
	void UpdateAllMovements(float DeltaDays);
	
	// ===== 移动命令处理 =====
	
	// 处理移动命令（商队/殖民者统一接口，由 PlayerController 调用）
	void HandleMoveCommand(AMoveableEntity01* Entity, FIntVector2 TargetCoord, bool bAppendPath = false);

	// 无实例时直接基于数据层处理移动命令
	void HandleMoveCommandByData(int32 UnitID, EEntityType01 UnitType, FIntVector2 TargetCoord, bool bAppendPath = false);
	
	// ===== 商队实例管理 =====
	
	// 从城镇spawn商队实例（数据驱动）
	AMoveableEntity01* SpawnCaravanFromTown(int32 CaravanID, ATownActor01* Town);
	
	// 在指定坐标创建商队实例（用于离开城镇时）
	AMoveableEntity01* SpawnCaravanInstanceAtCoord(int32 CaravanID, FIntVector2 GridCoord);
	
	// ===== 殖民者实例管理 =====
	
	// 从城镇spawn殖民者实例（数据驱动）
	AMoveableEntity01* SpawnColonistFromTown(int32 ColonistID, ATownActor01* Town);
	
	// 在指定坐标创建殖民者实例（用于离开城镇时）
	AMoveableEntity01* SpawnColonistInstanceAtCoord(int32 ColonistID, FIntVector2 GridCoord);
	
private:
	UPROPERTY()
	ADemo01_GM* GameMode = nullptr;
};
