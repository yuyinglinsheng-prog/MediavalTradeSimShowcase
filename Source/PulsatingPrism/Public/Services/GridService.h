// Services/GridService.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DataModels/GridTypes.h"
#include "DataModels/WorldDataStore.h"  // 为了获取数据验证
#include "GridService.generated.h"

/**
 * 网格几何计算服务 - 纯计算服务，不持有游戏数据
 *
 * 职责：
 * 1. 坐标转换（网格↔世界）
 * 2. 六边形几何计算（邻居、距离、范围）
 * 3. 网格布局验证和调试
 *
 * 特点：
 * - 无状态：不存储任何游戏数据
 * - 纯函数：输入→计算→输出
 * - 可重用：任何系统都可以使用
 */
UCLASS(meta = (DisplayName = "网格服务"))
class PULSATINGPRISM_API UGridService : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // ==================== 初始化 ====================
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ==================== 配置管理 ====================

    /**
     * 设置网格布局参数
     * @param Params 布局参数（单元格大小、网格类型等）
     * @return 是否设置成功
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Configuration")
    bool SetGridLayout(const FGridLayoutParams& Params);

    /**
     * 获取当前网格布局参数
     */
    UFUNCTION(BlueprintPure, Category = "Grid Service|Configuration")
    const FGridLayoutParams& GetGridLayout() const { return CurrentLayout; }

    /**
     * 重置为默认布局
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Configuration")
    void ResetToDefaultLayout();

    // ==================== 坐标转换核心方法 ====================

    /**
     * 网格坐标转世界坐标
     * @param GridCoord 网格坐标
     * @param WorldCenter 世界中心偏移（可选）
     * @return 包含结果和有效性的结构
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Coordinate Conversion")
    FGridTransformResult GridToWorld(const FIntVector2& GridCoord, const FVector& WorldCenter = FVector::ZeroVector) const;

    /**
     * 世界坐标转网格坐标
     * @param WorldLocation 世界位置
     * @param WorldCenter 世界中心偏移（必须与GridToWorld一致）
     * @return 网格坐标
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Coordinate Conversion")
    FIntVector2 WorldToGrid(const FVector& WorldLocation, const FVector& WorldCenter = FVector::ZeroVector) const;

    /**
     * 批量坐标转换（性能优化）
     * @param GridCoords 网格坐标数组
     * @param WorldPositions 输出的世界位置数组
     * @param WorldCenter 世界中心偏移
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Coordinate Conversion")
    void BatchGridToWorld(const TArray<FIntVector2>& GridCoords, TArray<FVector>& WorldPositions,
        const FVector& WorldCenter = FVector::ZeroVector) const;

    // ==================== 六边形几何计算 ====================

    /**
     * 获取六边形邻居坐标（6个方向）
     * @param CenterCoord 中心坐标
     * @return 邻居坐标数组（顺时针顺序：东、东南、西南、西、西北、东北）
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Hex Geometry")
    TArray<FIntVector2> GetHexNeighbors(const FIntVector2& CenterCoord) const;

    /**
     * 获取特定方向的邻居
     * @param CenterCoord 中心坐标
     * @param Direction 方向枚举
     * @return 邻居坐标
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Hex Geometry")
    FIntVector2 GetHexNeighborInDirection(const FIntVector2& CenterCoord, EHexDirection Direction) const;

    /**
     * 计算六边形距离
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Hex Geometry")
    int32 GetHexDistance(const FIntVector2& CoordA, const FIntVector2& CoordB) const;

    /**
     * 获取一定半径范围内的所有六边形坐标
     * @param CenterCoord 中心坐标
     * @param Radius 半径（格数）
     * @param bIncludeCenter 是否包含中心点
     * @return 范围内的坐标数组
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Hex Geometry")
    TArray<FIntVector2> GetHexCoordsInRange(const FIntVector2& CenterCoord, int32 Radius, bool bIncludeCenter = false) const;

    /**
     * 检查两个坐标是否相邻
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Hex Geometry")
    bool AreHexCoordsAdjacent(const FIntVector2& CoordA, const FIntVector2& CoordB) const;

    // ==================== 正方形网格支持 ====================

    /**
     * 获取正方形邻居（4或8方向）
     * @param CenterCoord 中心坐标
     * @param bIncludeDiagonals 是否包含对角方向
     * @return 邻居坐标数组
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Square Geometry")
    TArray<FIntVector2> GetSquareNeighbors(const FIntVector2& CenterCoord, bool bIncludeDiagonals = false) const;

    /**
     * 获取曼哈顿距离
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Square Geometry")
    int32 GetManhattanDistance(const FIntVector2& CoordA, const FIntVector2& CoordB) const;

    // ==================== 数据验证和查询 ====================

    /**
     * 验证坐标是否在有效范围内（需要WorldDataStore）
     * @param Coord 要验证的坐标
     * @param WorldDataStore 世界数据存储（用于获取边界）
     * @return 是否有效
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Validation")
    bool ValidateCoord(const FIntVector2& Coord, const UWorldDataStore* WorldDataStore = nullptr) const;

    /**
     * 获取网格边界（需要WorldDataStore）
     * @param WorldDataStore 世界数据存储
     * @return 边界框（世界空间）
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Validation")
    FBox GetGridBounds(const UWorldDataStore* WorldDataStore) const;

    /**
     * 计算世界中心偏移，使网格在指定区域内居中
     * @param DesiredBounds 期望的边界区域
     * @param GridSize 网格尺寸（宽×高）
     * @return 世界中心偏移
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Layout")
    FVector CalculateCenteringOffset(const FBox& DesiredBounds, const FIntVector2& GridSize) const;

    // ==================== 调试和可视化 ====================

    /**
     * 调试绘制单个网格单元
     * @param GridCoord 网格坐标
     * @param WorldCenter 世界中心
     * @param Color 绘制颜色
     * @param Duration 显示时间
     * @param Thickness 线粗
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Debug")
    void DrawDebugCell(const FIntVector2& GridCoord, const FVector& WorldCenter = FVector::ZeroVector,
        FColor Color = FColor::White, float Duration = 0.0f, float Thickness = 2.0f) const;

    /**
     * 调试绘制整个网格
     * @param WorldDataStore 世界数据存储（用于获取所有坐标）
     * @param WorldCenter 世界中心
     * @param Color 绘制颜色
     * @param Duration 显示时间
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Debug")
    void DrawDebugGrid(const UWorldDataStore* WorldDataStore, const FVector& WorldCenter = FVector::ZeroVector,
        FColor Color = FColor::White, float Duration = 0.0f) const;

    /**
     * 获取服务状态信息
     */
    UFUNCTION(BlueprintCallable, Category = "Grid Service|Debug")
    FString GetServiceStatus() const;

private:
    // ==================== 内部状态 ====================

    // 当前布局参数
    FGridLayoutParams CurrentLayout;

    // 六边形方向向量（轴向坐标）
    static const TArray<FHexCoord> HexDirectionVectors;

    // 正方形方向向量（4方向）
    static const TArray<FIntVector2> SquareDirectionVectors4;

    // 正方形方向向量（8方向，含对角线）
    static const TArray<FIntVector2> SquareDirectionVectors8;

    // ==================== 内部计算方法 ====================

    // 六边形坐标转换
    FVector HexToWorld(const FHexCoord& HexCoord, const FVector& WorldCenter) const;
    FHexCoord WorldToHex(const FVector& WorldLocation, const FVector& WorldCenter) const;
    FHexCoord RoundHexCoord(const FVector2D& FractionalCoord) const;

    // 正方形坐标转换
    FVector SquareToWorld(const FIntVector2& GridCoord, const FVector& WorldCenter) const;
    FIntVector2 WorldToSquare(const FVector& WorldLocation, const FVector& WorldCenter) const;

    // 辅助计算
    float GetHexWidth() const;
    float GetHexHeight() const;
    float GetEffectiveCellSize() const;

    // ==================== 偏移量计算相关 ====================

    // 内部偏移量（在 SetGridLayout 中计算）
    FVector2D InternalGridOffset = FVector2D::ZeroVector;

    // 获取有效偏移量（考虑 WorldCenter）
    FVector2D GetEffectiveOffset(const FVector& WorldCenter) const;

    // ==================== 调试 ====================
    
    // 调试绘制辅助
    void DrawHexagon(const FVector& Center, float Radius, FColor Color, float Duration, float Thickness) const;
    void DrawSquare(const FVector& Center, float HalfSize, FColor Color, float Duration, float Thickness) const;
    // 是否打印坐标Text
    float DebugText = false;
    // 日志
    void LogInfo(const FString& Message) const;
    void LogWarning(const FString& Message) const;
    void LogError(const FString& Message) const;
};
