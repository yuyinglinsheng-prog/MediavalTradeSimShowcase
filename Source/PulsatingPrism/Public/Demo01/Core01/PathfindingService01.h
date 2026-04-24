// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "PathfindingService01.generated.h"

class ADemo01_GM;
class UGridService;

/**
 * 寻路服务（A* 算法）
 * 作为 UObject 在 GameMode 中使用
 */
UCLASS()
class PULSATINGPRISM_API UPathfindingService01 : public UObject
{
	GENERATED_BODY()
	
public:
	// 初始化
	void Initialize(ADemo01_GM* InGameMode, const FDemo01GridConfig& InGridConfig);
	
	// A* 寻路算法
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	TArray<FIntVector2> FindPath(FIntVector2 Start, FIntVector2 End);
	
	// 检查坐标是否有效
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	bool IsValidCoord(FIntVector2 Coord) const;
	
	// 获取移动成本（可被子类重写以支持地形）
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	virtual float GetMovementCost(FIntVector2 Coord) const;
	
	// 设置网格单元数据（可选，用于地形成本）
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	void SetGridCellData(FIntVector2 Coord, const FDemo01GridCell& CellData);
	
private:
	UPROPERTY()
	ADemo01_GM* GameMode = nullptr;
	
	UPROPERTY()
	TObjectPtr<UGridService> GridService = nullptr;
	
	FDemo01GridConfig GridConfig;
	
	// 网格单元数据（可选）
	TMap<FIntVector2, FDemo01GridCell> GridCells;
	
	// ===== A* 算法辅助函数 =====
	
	// 计算启发式成本（六边形网格距离）
	float CalculateHeuristic(FIntVector2 From, FIntVector2 To) const;
	
	// 获取邻居节点（六边形网格的 6 个邻居）
	TArray<FIntVector2> GetNeighbors(FIntVector2 Coord) const;
	
	// 回溯路径
	void ReconstructPath(const TMap<FIntVector2, FIntVector2>& CameFrom, 
	                     FIntVector2 Current, 
	                     TArray<FIntVector2>& OutPath) const;
};
