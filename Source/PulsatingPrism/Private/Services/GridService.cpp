// Services/GridService.cpp
#include "Services/GridService.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// 六边形方向定义
const TArray<FHexCoord> UGridService::HexDirectionVectors = {
    FHexCoord(1, 0),    // 东
    FHexCoord(1, -1),   // 东南
    FHexCoord(0, -1),   // 西南
    FHexCoord(-1, 0),   // 西
    FHexCoord(-1, 1),   // 西北
    FHexCoord(0, 1)     // 东北
};

// 正方形方向定义
const TArray<FIntVector2> UGridService::SquareDirectionVectors4 = {
    FIntVector2(1, 0),   // 右
    FIntVector2(0, 1),   // 上
    FIntVector2(-1, 0),  // 左
    FIntVector2(0, -1)   // 下
};

const TArray<FIntVector2> UGridService::SquareDirectionVectors8 = {
    FIntVector2(1, 0),    // 右
    FIntVector2(1, 1),    // 右上
    FIntVector2(0, 1),    // 上
    FIntVector2(-1, 1),   // 左上
    FIntVector2(-1, 0),   // 左
    FIntVector2(-1, -1),  // 左下
    FIntVector2(0, -1),   // 下
    FIntVector2(1, -1)    // 右下
};

// ==================== 初始化 ====================

void UGridService::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 设置默认布局
    ResetToDefaultLayout();

    LogInfo(FString::Printf(TEXT("Grid Service initialized with layout: %s"), *CurrentLayout.ToString()));
}

bool UGridService::SetGridLayout(const FGridLayoutParams& Params)
{
    if (!Params.IsValid())
    {
        LogError(FString::Printf(TEXT("Invalid grid layout parameters: %s"), *Params.ToString()));
        return false;
    }

    FGridLayoutParams OldLayout = CurrentLayout;
    CurrentLayout = Params;

    // 重新计算偏移量
    CurrentLayout.CalculateGridOffset();
    InternalGridOffset = CurrentLayout.GridOffset;

    LogInfo(FString::Printf(TEXT("Grid layout changed: %s -> %s"),
        *OldLayout.ToString(), *CurrentLayout.ToString()));

    return true;
}

void UGridService::ResetToDefaultLayout()
{
    CurrentLayout = FGridLayoutParams();
    CurrentLayout.CellSize = 100.0f;
    CurrentLayout.bUseHexGrid = true;
    CurrentLayout.bFlatTop = true;
}

// ==================== 坐标转换核心方法 ====================

FGridTransformResult UGridService::GridToWorld(const FIntVector2& GridCoord, const FVector& WorldCenter) const
{
    FGridTransformResult Result;
    Result.GridCoord = GridCoord;
    Result.bIsValid = true;

    if (CurrentLayout.bUseHexGrid)
    {
        FHexCoord HexCoord(GridCoord.X, GridCoord.Y);
        Result.WorldPosition = HexToWorld(HexCoord, WorldCenter);
    }
    else
    {
        Result.WorldPosition = SquareToWorld(GridCoord, WorldCenter);
    }

    return Result;
}

FIntVector2 UGridService::WorldToGrid(const FVector& WorldLocation, const FVector& WorldCenter) const
{
    if (CurrentLayout.bUseHexGrid)
    {
        FHexCoord HexCoord = WorldToHex(WorldLocation, WorldCenter);
        return HexCoord.ToIntVector2();
    }
    else
    {
        return WorldToSquare(WorldLocation, WorldCenter);
    }
}

void UGridService::BatchGridToWorld(const TArray<FIntVector2>& GridCoords, TArray<FVector>& WorldPositions,
    const FVector& WorldCenter) const
{
    WorldPositions.Empty(GridCoords.Num());

    for (const FIntVector2& Coord : GridCoords)
    {
        WorldPositions.Add(GridToWorld(Coord, WorldCenter).WorldPosition);
    }
}

// ==================== 六边形几何计算 ====================

TArray<FIntVector2> UGridService::GetHexNeighbors(const FIntVector2& CenterCoord) const
{
    TArray<FIntVector2> Neighbors;
    Neighbors.Reserve(6);

    FHexCoord Center(CenterCoord.X, CenterCoord.Y);

    for (const FHexCoord& Dir : HexDirectionVectors)
    {
        FHexCoord Neighbor = Center + Dir;
        Neighbors.Add(Neighbor.ToIntVector2());
    }

    return Neighbors;
}

FIntVector2 UGridService::GetHexNeighborInDirection(const FIntVector2& CenterCoord, EHexDirection Direction) const
{
    if (Direction >= EHexDirection::Count)
    {
        LogWarning(FString::Printf(TEXT("Invalid hex direction: %d"), static_cast<int32>(Direction)));
        return CenterCoord;
    }

    FHexCoord Center(CenterCoord.X, CenterCoord.Y);
    FHexCoord Neighbor = Center + HexDirectionVectors[static_cast<int32>(Direction)];

    return Neighbor.ToIntVector2();
}

int32 UGridService::GetHexDistance(const FIntVector2& CoordA, const FIntVector2& CoordB) const
{
    FHexCoord A(CoordA.X, CoordA.Y);
    FHexCoord B(CoordB.X, CoordB.Y);

    // 六边形距离公式：(|dq| + |dq + dr| + |dr|) / 2
    FHexCoord Diff = A - B;
    return (FMath::Abs(Diff.Q) + FMath::Abs(Diff.Q + Diff.R) + FMath::Abs(Diff.R)) / 2;
}

TArray<FIntVector2> UGridService::GetHexCoordsInRange(const FIntVector2& CenterCoord, int32 Radius, bool bIncludeCenter) const
{
    TArray<FIntVector2> CoordsInRange;

    if (Radius < 0)
    {
        LogWarning(FString::Printf(TEXT("Invalid radius: %d"), Radius));
        return CoordsInRange;
    }

    // 预先分配大致大小（六边形范围公式）
    int32 EstimatedCount = 1 + 3 * Radius * (Radius + 1);
    CoordsInRange.Reserve(EstimatedCount);

    FHexCoord Center(CenterCoord.X, CenterCoord.Y);

    for (int32 q = -Radius; q <= Radius; ++q)
    {
        for (int32 r = -Radius; r <= Radius; ++r)
        {
            // 检查是否在范围内
            FHexCoord Candidate = Center + FHexCoord(q, r);

            // 计算立方体坐标的第三个分量
            int32 s = -q - r;

            // 检查距离
            if (FMath::Max3(FMath::Abs(q), FMath::Abs(r), FMath::Abs(s)) <= Radius)
            {
                // 转换为轴向坐标
                FIntVector2 GridCoord = Candidate.ToIntVector2();

                // 如果不包含中心点且距离为0，跳过
                if (!bIncludeCenter && q == 0 && r == 0 && s == 0)
                    continue;

                CoordsInRange.Add(GridCoord);
            }
        }
    }

    return CoordsInRange;
}

bool UGridService::AreHexCoordsAdjacent(const FIntVector2& CoordA, const FIntVector2& CoordB) const
{
    return GetHexDistance(CoordA, CoordB) == 1;
}

// ==================== 正方形网格支持 ====================

TArray<FIntVector2> UGridService::GetSquareNeighbors(const FIntVector2& CenterCoord, bool bIncludeDiagonals) const
{
    const TArray<FIntVector2>& DirectionVectors =
        bIncludeDiagonals ? SquareDirectionVectors8 : SquareDirectionVectors4;

    TArray<FIntVector2> Neighbors;
    Neighbors.Reserve(DirectionVectors.Num());

    for (const FIntVector2& Dir : DirectionVectors)
    {
        Neighbors.Add(CenterCoord + Dir);
    }

    return Neighbors;
}

int32 UGridService::GetManhattanDistance(const FIntVector2& CoordA, const FIntVector2& CoordB) const
{
    return FMath::Abs(CoordA.X - CoordB.X) + FMath::Abs(CoordA.Y - CoordB.Y);
}

// ==================== 内部计算方法 ====================

FVector UGridService::HexToWorld(const FHexCoord& HexCoord, const FVector& WorldCenter) const
{
    const float CellSize = CurrentLayout.CellSize;
    FVector2D EffectiveOffset = GetEffectiveOffset(WorldCenter);

    if (CurrentLayout.bFlatTop)
    {
        // 平顶六边形（flat-top）转换
        const float HexWidth = CellSize * FMath::Sqrt(3.0f);
        const float HexHeight = CellSize * 2.0f;

        float WorldX = WorldCenter.X + EffectiveOffset.X + HexWidth * (HexCoord.Q + 0.5f * (HexCoord.R & 1));
        float WorldY = WorldCenter.Y + EffectiveOffset.Y + HexHeight * 0.75f * HexCoord.R;

        return FVector(WorldX, WorldY, WorldCenter.Z);
    }
    else
    {
        // 尖顶六边形（pointy-top）转换
        const float HexWidth = CellSize * 2.0f;
        const float HexHeight = CellSize * FMath::Sqrt(3.0f);

        float WorldX = WorldCenter.X + EffectiveOffset.X + HexWidth * 0.75f * HexCoord.Q;
        float WorldY = WorldCenter.Y + EffectiveOffset.Y + HexHeight * (HexCoord.R + 0.5f * (HexCoord.Q & 1));

        return FVector(WorldX, WorldY, WorldCenter.Z);
    }
}

// ==================== 修复说明 ====================
// 此函数已修复以正确处理平顶六边形的偏移行布局（offset row layout）
// 修复日期：2026-03-10
// 问题：原始实现使用立方体坐标转换，但没有正确处理偏移行，导致坐标转换不准确
// 解决方案：使用与 HexToWorld 完全对应的逆向转换公式
// ==================== 修复说明 ====================
FHexCoord UGridService::WorldToHex(const FVector& WorldLocation, const FVector& WorldCenter) const
{
    if (!CurrentLayout.bFlatTop)
    {
        // 尖顶六边形转换较复杂，这里简化处理
        LogWarning(TEXT("WorldToHex for pointy-top hexagons not fully implemented"));
        return FHexCoord(0, 0);
    }

    // 平顶六边形转换（逆运算）
    const float CellSize = CurrentLayout.CellSize;
    const float HexWidth = CellSize * FMath::Sqrt(3.0f);
    const float HexHeight = CellSize * 2.0f;
    FVector2D EffectiveOffset = GetEffectiveOffset(WorldCenter);

    // 转换为相对于世界中心的坐标
    float LocalX = WorldLocation.X - WorldCenter.X - EffectiveOffset.X;
    float LocalY = WorldLocation.Y - WorldCenter.Y - EffectiveOffset.Y;

    // 先计算 R（行）坐标
    // 从 HexToWorld: WorldY = HexHeight * 0.75f * R
    float r = LocalY / (HexHeight * 0.75f);
    int32 R = FMath::RoundToInt(r);
    
    // 计算 Q（列）坐标，考虑偏移行
    // 从 HexToWorld: WorldX = HexWidth * (Q + 0.5f * (R & 1))
    float q = (LocalX / HexWidth) - 0.5f * (R & 1);
    int32 Q = FMath::RoundToInt(q);
    
    return FHexCoord(Q, R);
}

FHexCoord UGridService::RoundHexCoord(const FVector2D& FractionalCoord) const
{
    float q = FractionalCoord.X;
    float r = FractionalCoord.Y;
    float s = -q - r;

    // 四舍五入
    int32 roundQ = FMath::RoundToInt(q);
    int32 roundR = FMath::RoundToInt(r);
    int32 roundS = FMath::RoundToInt(s);

    float qDiff = FMath::Abs(roundQ - q);
    float rDiff = FMath::Abs(roundR - r);
    float sDiff = FMath::Abs(roundS - s);

    // 调整四舍五入以保持 q + r + s = 0
    if (qDiff > rDiff && qDiff > sDiff)
    {
        roundQ = -roundR - roundS;
    }
    else if (rDiff > sDiff)
    {
        roundR = -roundQ - roundS;
    }
    else
    {
        roundS = -roundQ - roundR;
    }

    return FHexCoord(roundQ, roundR);
}

FVector UGridService::SquareToWorld(const FIntVector2& GridCoord, const FVector& WorldCenter) const
{
    const float CellSize = CurrentLayout.CellSize;

    float WorldX = WorldCenter.X + GridCoord.X * CellSize;
    float WorldY = WorldCenter.Y + GridCoord.Y * CellSize;

    return FVector(WorldX, WorldY, WorldCenter.Z);
}

FIntVector2 UGridService::WorldToSquare(const FVector& WorldLocation, const FVector& WorldCenter) const
{
    const float CellSize = CurrentLayout.CellSize;

    int32 GridX = FMath::FloorToInt((WorldLocation.X - WorldCenter.X) / CellSize);
    int32 GridY = FMath::FloorToInt((WorldLocation.Y - WorldCenter.Y) / CellSize);

    return FIntVector2(GridX, GridY);
}

// ==================== 验证和查询 ====================

bool UGridService::ValidateCoord(const FIntVector2& Coord, const UWorldDataStore* WorldDataStore) const
{
    // 基本验证
    if (Coord.X == TNumericLimits<int32>::Min() || Coord.Y == TNumericLimits<int32>::Min())
    {
        return false;
    }

    // 如果有WorldDataStore，进行更严格的验证
    if (WorldDataStore)
    {
        return WorldDataStore->ContainsCoord(Coord);
    }

    return true;
}

FBox UGridService::GetGridBounds(const UWorldDataStore* WorldDataStore) const
{
    if (!WorldDataStore)
    {
        LogWarning(TEXT("Cannot calculate grid bounds: WorldDataStore is null"));
        return FBox(ForceInit);
    }

    // 直接使用WorldDataStore的边界计算方法
    return WorldDataStore->CalculateWorldBounds();
}

FVector UGridService::CalculateCenteringOffset(const FBox& DesiredBounds, const FIntVector2& GridSize) const
{
    if (!DesiredBounds.IsValid)
    {
        LogWarning(TEXT("Invalid desired bounds for centering"));
        return FVector::ZeroVector;
    }

    // 计算网格的总尺寸
    FVector2D GridExtent;
    if (CurrentLayout.bUseHexGrid)
    {
        // 六边形网格的近似尺寸
        float HexWidth = GetHexWidth();
        float HexHeight = GetHexHeight();
        GridExtent = FVector2D(HexWidth * GridSize.X, HexHeight * GridSize.Y) * 0.5f;
    }
    else
    {
        // 正方形网格尺寸
        float CellSize = GetEffectiveCellSize();
        GridExtent = FVector2D(CellSize * GridSize.X, CellSize * GridSize.Y) * 0.5f;
    }

    // 计算需要的偏移，使网格在边界内居中
    FVector DesiredCenter = DesiredBounds.GetCenter();
    FVector DesiredExtent = DesiredBounds.GetExtent();

    // 确保网格不会超出边界
    float ScaleX = DesiredExtent.X / GridExtent.X;
    float ScaleY = DesiredExtent.Y / GridExtent.Y;
    float Scale = FMath::Min(ScaleX, ScaleY);

    if (Scale < 1.0f)
    {
        LogWarning(FString::Printf(TEXT("Grid too large for bounds, scaling by %.2f"), Scale));
    }

    // 计算偏移，使网格中心与边界中心对齐
    return DesiredCenter;
}

// ==================== 调试和可视化 ====================

void UGridService::DrawDebugCell(const FIntVector2& GridCoord, const FVector& WorldCenter,
    FColor Color, float Duration, float Thickness) const
{
    UWorld* World = GetWorld();
    if (!World) return;

    FGridTransformResult Transform = GridToWorld(GridCoord, WorldCenter);
    if (!Transform.bIsValid) return;

    const FVector& Center = Transform.WorldPosition;
    float CellSize = GetEffectiveCellSize();

    if (CurrentLayout.bUseHexGrid)
    {
        DrawHexagon(Center, CellSize, Color, Duration, Thickness);

        if (DebugText)
        {
            // 绘制坐标标签
            FString CoordText = FString::Printf(TEXT("(%d, %d)"), GridCoord.X, GridCoord.Y);
            DrawDebugString(World, Center + FVector(0, 0, 50), CoordText, nullptr, Color, Duration, false, 1.0f);
        }

    }
    else
    {
        DrawSquare(Center, CellSize * 0.5f, Color, Duration, Thickness);

        if (DebugText)
        {
            // 绘制坐标标签
            FString CoordText = FString::Printf(TEXT("(%d, %d)"), GridCoord.X, GridCoord.Y);
            DrawDebugString(World, Center + FVector(0, 0, 50), CoordText, nullptr, Color, Duration, false, 1.0f);
        }

    }

    // 绘制中心点
    //DrawDebugPoint(World, Center, 10.0f, Color, Duration, 0);
}

void UGridService::DrawDebugGrid(const UWorldDataStore* WorldDataStore, const FVector& WorldCenter,
    FColor Color, float Duration) const
{
    if (!WorldDataStore)
    {
        LogWarning(TEXT("Cannot draw debug grid: WorldDataStore is null"));
        return;
    }

    // 绘制所有可见的网格单元
    for (int32 i = 0; i < WorldDataStore->GetTileCount(); ++i)
    {
        const FIntVector2* Coord = WorldDataStore->GetCoordinateByIndex(i);
        if (Coord)
        {
            DrawDebugCell(*Coord, WorldCenter, Color, Duration, 1.0f);
        }
    }

    LogInfo(FString::Printf(TEXT("Drawn debug grid with %d cells"), WorldDataStore->GetTileCount()));
}

FString UGridService::GetServiceStatus() const
{
    FString Status;

    Status += TEXT("=== Grid Service Status ===\n");
    Status += FString::Printf(TEXT("Layout: %s\n"), *CurrentLayout.ToString());
    Status += FString::Printf(TEXT("Effective Cell Size: %.1f\n"), GetEffectiveCellSize());

    if (CurrentLayout.bUseHexGrid)
    {
        Status += FString::Printf(TEXT("Hex Width: %.1f, Hex Height: %.1f\n"), GetHexWidth(), GetHexHeight());
    }

    return Status;
}

// ==================== 辅助方法 ====================

float UGridService::GetHexWidth() const
{
    return CurrentLayout.CellSize * FMath::Sqrt(3.0f);
}

float UGridService::GetHexHeight() const
{
    return CurrentLayout.CellSize * 2.0f;
}

float UGridService::GetEffectiveCellSize() const
{
    return CurrentLayout.CellSize;
}

FVector2D UGridService::GetEffectiveOffset(const FVector& WorldCenter) const
{
    // 如果 WorldCenter 不是零，可能需要调整偏移量
   // 目前简单返回内部偏移
    return InternalGridOffset;
}

void UGridService::DrawHexagon(const FVector& Center, float Radius, FColor Color, float Duration, float Thickness) const
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 平顶六边形从30度开始绘制
    FVector PrevPoint;
    for (int32 i = 0; i < 6; ++i)
    {
        float Angle1 = (60.0f * i + 30.0f) * (PI / 180.0f);
        float Angle2 = (60.0f * (i + 1) + 30.0f) * (PI / 180.0f);

        FVector Point1 = Center + FVector(Radius * FMath::Cos(Angle1), Radius * FMath::Sin(Angle1), 0.0f);
        FVector Point2 = Center + FVector(Radius * FMath::Cos(Angle2), Radius * FMath::Sin(Angle2), 0.0f);

        DrawDebugLine(World, Point1, Point2, Color, false, Duration, 0, Thickness);

        if (i == 0) PrevPoint = Point1;
    }
}

void UGridService::DrawSquare(const FVector& Center, float HalfSize, FColor Color, float Duration, float Thickness) const
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 四个角
    FVector TopLeft = Center + FVector(-HalfSize, -HalfSize, 0.0f);
    FVector TopRight = Center + FVector(HalfSize, -HalfSize, 0.0f);
    FVector BottomRight = Center + FVector(HalfSize, HalfSize, 0.0f);
    FVector BottomLeft = Center + FVector(-HalfSize, HalfSize, 0.0f);

    // 绘制四条边
    DrawDebugLine(World, TopLeft, TopRight, Color, false, Duration, 0, Thickness);
    DrawDebugLine(World, TopRight, BottomRight, Color, false, Duration, 0, Thickness);
    DrawDebugLine(World, BottomRight, BottomLeft, Color, false, Duration, 0, Thickness);
    DrawDebugLine(World, BottomLeft, TopLeft, Color, false, Duration, 0, Thickness);
}

// ==================== 日志方法 ====================

void UGridService::LogInfo(const FString& Message) const
{
    UE_LOG(LogTemp, Log, TEXT("[GridService] %s"), *Message);
}

void UGridService::LogWarning(const FString& Message) const
{
    UE_LOG(LogTemp, Warning, TEXT("[GridService] WARNING: %s"), *Message);
}

void UGridService::LogError(const FString& Message) const
{
    UE_LOG(LogTemp, Error, TEXT("[GridService] ERROR: %s"), *Message);
}
