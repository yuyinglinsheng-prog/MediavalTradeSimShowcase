// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "Demo01/Core01/MarketManager01.h"
#include "Demo01/UIs/MarketWidget.h"

#include "Demo01_GM.generated.h"

// 包装结构体：用于在 TMap 中存储 TArray<FString>
USTRUCT(BlueprintType)
struct FRecipeIDArray
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipes")
	TArray<FString> RecipeIDs;
};

// 前向声明
class UPathfindingService01;
class UTimeManager01;
class UGridService;
class ATownActor01;
class AMoveableEntity01;
class UDemo01UIManager;
class UProductionManager01;
class UUnitControlManager01;
class UTradeRouteManager01;
class AHexGridActorBase;  // 六边形网格地块基类
class APathVisualizerActor01;  // 路径可视化Actor

/**
 * Demo01 GameMode
 * 管理网格系统、寻路服务、时间管理器等
 */
UCLASS()
class PULSATINGPRISM_API ADemo01_GM : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ADemo01_GM();
	
	// ===== 网格系统 =====
	
	// 初始化网格系统
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void InitializeGridSystem(const FDemo01GridConfig& Config);
	
	// 网格坐标转世界坐标
	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GridCoordToWorld(FIntVector2 Coord) const;
	
	// 世界坐标转网格坐标
	UFUNCTION(BlueprintPure, Category = "Grid")
	FIntVector2 WorldToGridCoord(FVector WorldLocation) const;
	
	// 获取网格配置
	UFUNCTION(BlueprintPure, Category = "Grid")
	FDemo01GridConfig GetGridConfig() const { return GridConfig; }
	
	// ===== 移动命令 =====
	
	// 处理移动命令（商队/殖民者统一接口，由 PlayerController 调用）
	void HandleMoveCommand(AMoveableEntity01* Entity, FIntVector2 TargetCoord, bool bAppendPath = false);

	// 无实例时直接基于数据层处理移动命令
	void HandleMoveCommandByData(int32 UnitID, EEntityType01 UnitType, FIntVector2 TargetCoord, bool bAppendPath = false);
	
	// ===== 服务访问 =====
	
	// 获取寻路服务
	UFUNCTION(BlueprintPure, Category = "Services")
	UPathfindingService01* GetPathfindingService() const { return PathfindingService; }
	
	// 获取时间管理器
	UFUNCTION(BlueprintPure, Category = "Services")
	UTimeManager01* GetTimeManager() const { return TimeManager; }
	
	// 获取市场管理器
	UFUNCTION(BlueprintPure, Category = "Services")
	UMarketManager01* GetMarketManager() const { return MarketManager; }
	
	// 获取 GridService
	UFUNCTION(BlueprintPure, Category = "Services")
	UGridService* GetGridService() const { return GridService; }
	
	// 获取 UI 管理器
	UFUNCTION(BlueprintPure, Category = "Services")
	UDemo01UIManager* GetUIManager() const { return UIManager; }
	
	// 获取生产管理器
	UFUNCTION(BlueprintPure, Category = "Services")
	UProductionManager01* GetProductionManager() const { return ProductionManager; }
	
	// 获取单位控制管理器
	UFUNCTION(BlueprintPure, Category = "Services")
	UUnitControlManager01* GetUnitControlManager() const { return UnitControlManager; }
	
	// 获取贸易路线管理器
	UFUNCTION(BlueprintPure, Category = "Services")
	UTradeRouteManager01* GetTradeRouteManager() const { return TradeRouteManager; }
	
	// 获取路径可视化Actor
	UFUNCTION(BlueprintPure, Category = "Services")
	APathVisualizerActor01* GetPathVisualizer() const { return PathVisualizer; }
	
	// 获取测试类
	TSubclassOf<AMoveableEntity01> GetTestCaravanClass() const { return TestCaravanClass; }
	TSubclassOf<AMoveableEntity01> GetTestColonistClass() const { return TestColonistClass; }
	
	// ===== 生产系统 =====
	
	// 手动触发所有城镇执行一轮生产（由 OnGameDayPassed 自动调用，也可手动调用）
	UFUNCTION(BlueprintCallable, Category = "Production")
	void ExecuteAllTownsProduction();
	
	// 游戏天数推进回调（由 TimeManager 每天调用一次）
	void OnGameDayPassed();
	
	// ===== 实体查询 =====
	
	// 根据网格坐标查找城镇
	UFUNCTION(BlueprintPure, Category = "Entity Query")
	ATownActor01* GetTownAtCoord(FIntVector2 Coord) const;
	
	// ===== Debug 功能 =====
	
	// 切换网格绘制（运行时调用）
	UFUNCTION(BlueprintCallable, Category = "Debug|Grid")
	void ToggleGridDrawing();
	
	// 重新绘制网格（运行时调用）
	UFUNCTION(BlueprintCallable, Category = "Debug|Grid")
	void RedrawGrid();
	
	// 切换网格坐标标签显示
	UFUNCTION(BlueprintCallable, Category = "Debug|Grid")
	void ToggleGridLabels();
	
	// 绘制所有网格坐标标签（每帧调用）
	void DrawGridLabels();
	
	// ===== 测试功能 =====
	
	// 在网格中心生成测试城镇
	UFUNCTION(BlueprintCallable, Category = "Debug|Test")
	void SpawnTestTown();
	
	// 初始化测试城镇数据（旧版，硬编码）
	void InitializeTestTownData(ATownActor01* Town);

	// 用随机原料建筑初始化城镇（从ProductLevel==0配方中随机选取）
	void InitializeTownWithNativeBuildings(ATownActor01* Town, int32 NumNativeBuildings = 3);
	
	// 基于地理资源分布选择基础建筑配方
	TArray<FString> SelectNativeRecipesByGeography(FIntVector2 CityCoord);
	
	// 获取城市及其周围格子的资源分布
	TArray<FString> GetResourcesInCityRadius(FIntVector2 CityCoord);
	
	// 生成六边形网格可视化Mesh
	UFUNCTION(BlueprintCallable, Category = "Debug|Test")
	void GenerateHexGridMeshes();
	
	// 清除六边形网格Mesh
	UFUNCTION(BlueprintCallable, Category = "Debug|Test")
	void ClearHexGridMeshes();
	
	// 从城镇spawn商队实例（数据驱动）
	AMoveableEntity01* SpawnCaravanFromTown(int32 CaravanID, ATownActor01* Town);

	// 在指定坐标创建商队实例（用于离开城镇时）
	AMoveableEntity01* SpawnCaravanInstanceAtCoord(int32 CaravanID, FIntVector2 GridCoord);
	
	// ===== 殖民者管理 =====
	
	// 购买殖民者（在城镇中创建殖民者数据）
	UFUNCTION(BlueprintCallable, Category = "Colonist")
	bool PurchaseColonist(ATownActor01* Town, const FString& ColonistName = TEXT(""));
	
	// 从城镇spawn殖民者实例（数据驱动）
	AMoveableEntity01* SpawnColonistFromTown(int32 ColonistID, ATownActor01* Town);

	// 在指定坐标创建殖民者实例（用于离开城镇时）
	AMoveableEntity01* SpawnColonistInstanceAtCoord(int32 ColonistID, FIntVector2 GridCoord);

	// 从殖民者创建城镇（建城功能，通过ID识别殖民者）
	ATownActor01* CreateCityFromColonist(int32 ColonistID);
	
	// 检查指定坐标是否可以建城（距离所有城市至少MinDistance格）
	UFUNCTION(BlueprintPure, Category = "City")
	bool CanFoundCityAtCoord(FIntVector2 Coord, int32 MinDistance = 3) const;
	
	// 获取距离指定坐标最近的城市距离
	UFUNCTION(BlueprintPure, Category = "City")
	int32 GetNearestTownDistance(FIntVector2 Coord) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// 初始化所有服务
	void InitializeServices();
	
	// 初始化寻路服务
	void InitializePathfindingService();
	
	// 初始化时间管理器
	void InitializeTimeManager();
	
	// 初始化 UI 系统
	void InitializeUISystem();
	
	// 初始化生产系统
	void InitializeProductionSystem();
	
	// 初始化路径可视化系统
	void InitializePathVisualizer();
	
	// ===== 测试内容 =====
	
	void TestBlueprintWidget();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	TSubclassOf<UUserWidget> BlueprintOnlyWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsDebugOn = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bUseBlueprintOnlyWidget = false;

	// 调试开关：Snap移动模式（单位瞬间跳到下一格，跳过插值）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bSnapMovement = false;
public:
	bool IsSnapMovementEnabled() const { return bSnapMovement; }
	
	// ===== 资源系统接口 =====
	
	// 生成网格资源分布
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void GenerateGridResources();
	
	// 根据权重随机选择资源类型
	UFUNCTION(BlueprintPure, Category = "Resources")
	FString GetRandomResourceType() const;
	
	// 检查城镇资源并解锁建筑
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void CheckTownResourceAndUnlockBuildings(ATownActor01* Town, FIntVector2 GridCoord);
	
	// 根据资源类型获取解锁的建筑
	UFUNCTION(BlueprintPure, Category = "Resources")
	TArray<FString> GetUnlockedBuildingsForResource(const FString& ResourceType) const;

private:
	// ===== 网格系统 =====
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	FDemo01GridConfig GridConfig;
	
	UPROPERTY()
	TObjectPtr<UGridService> GridService = nullptr;
	
	// 网格单元数据存储
	UPROPERTY()
	TMap<FIntVector2, FDemo01GridCell> GridCells;
	
	// 生成的六边形格子Actor列表
	UPROPERTY()
	TArray<class AHexGridActorBase*> HexGridActors;
	
	// 路径可视化Actor
	UPROPERTY()
	TObjectPtr<APathVisualizerActor01> PathVisualizer = nullptr;
	
	// ===== 资源系统配置 =====
	
	// 资源生成权重配置
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Resources")
	TMap<FString, float> ResourceGenerationWeights;  // 资源类型 -> 生成权重
	
	// 资源建筑解锁映射
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Resources")
	TMap<FString, FRecipeIDArray> ResourceBuildingUnlocks;  // 资源类型 -> 可解锁配方ID列表
	
	// ===== 服务实例（作为 UObject 使用） =====
	
	UPROPERTY()
	TObjectPtr<UPathfindingService01> PathfindingService = nullptr;
	
	UPROPERTY()
	TObjectPtr<UTimeManager01> TimeManager = nullptr;
	
	UPROPERTY()
	TObjectPtr<UMarketManager01> MarketManager = nullptr;
	
	UPROPERTY()
	TObjectPtr<UDemo01UIManager> UIManager = nullptr;
	
	UPROPERTY()
	TObjectPtr<UProductionManager01> ProductionManager = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUnitControlManager01> UnitControlManager = nullptr;
	
	UPROPERTY()
	TObjectPtr<UTradeRouteManager01> TradeRouteManager = nullptr;

protected:
	// ===== 生产系统配置 =====
	
	// 产品数据表
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
	TObjectPtr<UDataTable> ProductDataTable = nullptr;
	
	// 配方数据表
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
	TObjectPtr<UDataTable> RecipeDataTable = nullptr;
	
private:
	// ===== 开关配置 =====
	
	// 是否启用市场系统
	UPROPERTY(EditAnywhere, Category = "Features")
	bool bEnableMarketSystem = true;
	
	// ===== UI 配置 =====
	
	// UI Widget 类配置
	UPROPERTY(EditAnywhere, Category = "UI")
	FDemo01UIConfig UIConfig;
	
	// ===== 测试配置 =====
	
	// 测试城镇蓝图类
	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<ATownActor01> TestTownClass;
	
	// 商队实体蓝图类
	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<AMoveableEntity01> TestCaravanClass;

	// 殖民者实体蓝图类
	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<AMoveableEntity01> TestColonistClass;
	
	// ===== 网格可视化配置 =====
	
	// 六边形格子Actor类（必须是AHexGridActorBase或其子类）
	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<AHexGridActorBase> HexGridActorClass;
	
	// 路径可视化Actor类
	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<APathVisualizerActor01> PathVisualizerClass;
	
	// 是否显示网格Mesh
	UPROPERTY(EditAnywhere, Category = "Test")
	bool bShowHexGridMesh = true;
	
	// ===== Debug 配置 =====
	
	// 是否绘制网格
	UPROPERTY(EditAnywhere, Category = "Debug|Grid")
	bool bDebugDrawGrid = true;
	
	// 是否显示网格坐标标签
	UPROPERTY(EditAnywhere, Category = "Debug|Grid")
	bool bShowGridLabels = false;
	
	// Debug 绘制持续时间（秒，0 = 永久）
	UPROPERTY(EditAnywhere, Category = "Debug|Grid")
	float DebugDrawDuration = 0.0f;
	
	// Debug 网格颜色
	UPROPERTY(EditAnywhere, Category = "Debug|Grid")
	FColor DebugGridColor = FColor::Green;
	
	// Debug 线条粗细
	UPROPERTY(EditAnywhere, Category = "Debug|Grid")
	float DebugLineThickness = 1.0f;
	
	// ===== 内部辅助函数 =====
	
	// 初始化网格单元数据（可选）
	void InitializeGridCells();
	
	// 绘制 Debug 网格
	void DrawDebugGridCells();
};
