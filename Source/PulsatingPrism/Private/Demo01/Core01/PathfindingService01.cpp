// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/PathfindingService01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "DataModels/GridTypes.h"
#include "Services/GridService.h"

namespace
{
	FHexCoord OffsetOddRToAxial(const FIntVector2& Coord)
	{
		return FHexOffsetCoord(Coord.X, Coord.Y).ToAxial();
	}

	int32 CalculateOddRHexDistance(const FIntVector2& From, const FIntVector2& To)
	{
		const FHexCoord AxialFrom = OffsetOddRToAxial(From);
		const FHexCoord AxialTo = OffsetOddRToAxial(To);
		const FHexCoord Diff = AxialFrom - AxialTo;
		return (FMath::Abs(Diff.Q) + FMath::Abs(Diff.Q + Diff.R) + FMath::Abs(Diff.R)) / 2;
	}

	TArray<FIntVector2> GetOddRNeighbors(const FIntVector2& Coord)
	{
		static const FIntVector2 EvenRowDirections[6] = {
			FIntVector2(1, 0),
			FIntVector2(0, -1),
			FIntVector2(-1, -1),
			FIntVector2(-1, 0),
			FIntVector2(-1, 1),
			FIntVector2(0, 1)
		};

		static const FIntVector2 OddRowDirections[6] = {
			FIntVector2(1, 0),
			FIntVector2(1, -1),
			FIntVector2(0, -1),
			FIntVector2(-1, 0),
			FIntVector2(0, 1),
			FIntVector2(1, 1)
		};

		const FIntVector2* Directions = ((Coord.Y & 1) == 0) ? EvenRowDirections : OddRowDirections;

		TArray<FIntVector2> Neighbors;
		Neighbors.Reserve(6);
		for (int32 Index = 0; Index < 6; ++Index)
		{
			Neighbors.Add(Coord + Directions[Index]);
		}

		return Neighbors;
	}
}

void UPathfindingService01::Initialize(ADemo01_GM* InGameMode, const FDemo01GridConfig& InGridConfig)
{
	GameMode = InGameMode;
	GridConfig = InGridConfig;
	
	// 获取 GridService
	if (GameMode && GameMode->GetWorld())
	{
		GridService = GameMode->GetWorld()->GetSubsystem<UGridService>();
	}
	
	UE_LOG(LogTemp, Log, TEXT("[寻路服务] 初始化完成，网格大小: %dx%d"), 
		GridConfig.GridWidth, GridConfig.GridHeight);
}

TArray<FIntVector2> UPathfindingService01::FindPath(FIntVector2 Start, FIntVector2 End)
{
	TArray<FIntVector2> Path;
	
	// 验证起点和终点
	if (!IsValidCoord(Start) || !IsValidCoord(End))
	{
		UE_LOG(LogTemp, Error, TEXT("[寻路服务] 无效的起点或终点: (%d,%d) -> (%d,%d)，有效范围: [0,%d] x [0,%d]"), 
			Start.X, Start.Y, End.X, End.Y, GridConfig.GridWidth-1, GridConfig.GridHeight-1);
		return Path;
	}
	
	// 如果起点和终点相同
	if (Start == End)
	{
		Path.Add(Start);
		return Path;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[寻路服务] 开始寻路: (%d,%d) -> (%d,%d)"), 
		Start.X, Start.Y, End.X, End.Y);
	
	// A* 算法
	TArray<FPathNode01> OpenList;
	TSet<FIntVector2> ClosedSet;
	TMap<FIntVector2, FIntVector2> CameFrom;
	TMap<FIntVector2, float> GScore;
	
	// 初始化起点
	FPathNode01 StartNode(Start);
	StartNode.GCost = 0.0f;
	StartNode.HCost = CalculateHeuristic(Start, End);
	StartNode.CalculateFCost();
	OpenList.Add(StartNode);
	GScore.Add(Start, 0.0f);
	
	int32 IterationCount = 0;
	const int32 MaxIterations = 10000; // 防止无限循环
	
	while (OpenList.Num() > 0 && IterationCount < MaxIterations)
	{
		IterationCount++;
		
		// 找到 FCost 最小的节点
		int32 CurrentIndex = 0;
		for (int32 i = 1; i < OpenList.Num(); ++i)
		{
			if (OpenList[i].FCost < OpenList[CurrentIndex].FCost)
			{
				CurrentIndex = i;
			}
		}
		
		FPathNode01 Current = OpenList[CurrentIndex];
		OpenList.RemoveAt(CurrentIndex);
		
		// 到达终点
		if (Current.Coord == End)
		{
			ReconstructPath(CameFrom, End, Path);
			UE_LOG(LogTemp, Log, TEXT("[寻路服务] 找到路径，长度: %d，迭代次数: %d"), 
				Path.Num(), IterationCount);
			return Path;
		}
		
		ClosedSet.Add(Current.Coord);
		
		// 遍历邻居
		TArray<FIntVector2> Neighbors = GetNeighbors(Current.Coord);
		for (const FIntVector2& Neighbor : Neighbors)
		{
			if (ClosedSet.Contains(Neighbor) || !IsValidCoord(Neighbor))
			{
				continue;
			}
			
			float TentativeGScore = GScore[Current.Coord] + GetMovementCost(Neighbor);
			
			if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
			{
				CameFrom.Add(Neighbor, Current.Coord);
				GScore.Add(Neighbor, TentativeGScore);
				
				FPathNode01 NeighborNode(Neighbor);
				NeighborNode.GCost = TentativeGScore;
				NeighborNode.HCost = CalculateHeuristic(Neighbor, End);
				NeighborNode.CalculateFCost();
				
				// 检查是否已在开放列表中
				bool bFound = false;
				for (int32 i = 0; i < OpenList.Num(); ++i)
				{
					if (OpenList[i].Coord == Neighbor)
					{
						if (NeighborNode.FCost < OpenList[i].FCost)
						{
							OpenList[i] = NeighborNode;
						}
						bFound = true;
						break;
					}
				}
				
				if (!bFound)
				{
					OpenList.Add(NeighborNode);
				}
			}
		}
	}
	
	// 未找到路径
	UE_LOG(LogTemp, Warning, TEXT("[寻路服务] 未找到路径: (%d,%d) -> (%d,%d)，迭代次数: %d"), 
		Start.X, Start.Y, End.X, End.Y, IterationCount);
	return Path;
}

bool UPathfindingService01::IsValidCoord(FIntVector2 Coord) const
{
	// 检查是否在网格范围内
	if (Coord.X < 0 || Coord.X >= GridConfig.GridWidth ||
		Coord.Y < 0 || Coord.Y >= GridConfig.GridHeight)
	{
		return false;
	}
	
	// 检查是否可通行（如果有网格单元数据）
	if (const FDemo01GridCell* Cell = GridCells.Find(Coord))
	{
		return Cell->bIsWalkable;
	}
	
	// 默认可通行
	return true;
}

float UPathfindingService01::GetMovementCost(FIntVector2 Coord) const
{
	// 如果有网格单元数据，返回其移动成本
	if (const FDemo01GridCell* Cell = GridCells.Find(Coord))
	{
		return Cell->MovementCost;
	}
	
	// 默认成本为 1.0
	return 1.0f;
}

void UPathfindingService01::SetGridCellData(FIntVector2 Coord, const FDemo01GridCell& CellData)
{
	GridCells.Add(Coord, CellData);
}

float UPathfindingService01::CalculateHeuristic(FIntVector2 From, FIntVector2 To) const
{
	return static_cast<float>(CalculateOddRHexDistance(From, To));
}

TArray<FIntVector2> UPathfindingService01::GetNeighbors(FIntVector2 Coord) const
{
	return GetOddRNeighbors(Coord);
}

void UPathfindingService01::ReconstructPath(const TMap<FIntVector2, FIntVector2>& CameFrom, 
                                             FIntVector2 Current, 
                                             TArray<FIntVector2>& OutPath) const
{
	OutPath.Empty();
	OutPath.Add(Current);
	
	while (CameFrom.Contains(Current))
	{
		Current = CameFrom[Current];
		OutPath.Insert(Current, 0);
	}
}
