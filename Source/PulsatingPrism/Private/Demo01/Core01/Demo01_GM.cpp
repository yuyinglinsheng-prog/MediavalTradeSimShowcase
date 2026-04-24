// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/PathfindingService01.h"
#include "Demo01/Core01/TimeManager01.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/UnitControlManager01.h"
#include "Demo01/Core01/TradeRouteManager01.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Actors/HexGridActorBase.h"
#include "Demo01/Actors/PathVisualizerActor01.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Services/GridService.h"
#include "DataModels/GridTypes.h"

ADemo01_GM::ADemo01_GM()
{
	// 启用 Tick
	PrimaryActorTick.bCanEverTick = true;
	
	// 设置默认网格配置
	GridConfig.GridWidth = 20;
	GridConfig.GridHeight = 20;
	GridConfig.HexRadius = 100.0f;
	GridConfig.bIsFlatTop = true;
	
	// 初始化资源生成权重
	ResourceGenerationWeights = {
		{"iron_ore", 0.15f},   // 15%概率铁矿
		{"wood", 0.25f},       // 25%概率木材
		{"stone", 0.20f},      // 20%概率石头
		{"grains", 0.20f},     // 20%概率粮食
		{"", 0.20f}            // 20%概率无资源
	};
	
	// 初始化资源配方解锁映射（资源类型 -> 可解锁的配方ID）
	ResourceBuildingUnlocks = {
		{"iron_ore", FRecipeIDArray{ {"mine_iron_ore"} }},      // 铁矿解锁铁矿配方
		{"wood", FRecipeIDArray{ {"cut_wood_axe"} }},           // 木材解锁伐木配方
		{"stone", FRecipeIDArray{ {"mine_stone"} }},            // 石头解锁采石配方
		{"grains", FRecipeIDArray{ {"farm_grains_tool"}} }      // 粮食解锁农场配方
	};
}

void ADemo01_GM::BeginPlay()
{
	Super::BeginPlay();
	
	// 调试：测试纯蓝图Widget
	if (bUseBlueprintOnlyWidget)
	{
		TestBlueprintWidget();
		return;
	}
	
	// 1. 初始化网格系统
	InitializeGridSystem(GridConfig);
	
	// 2. 生成资源分布（关键：必须在创建城镇之前）
	GenerateGridResources();
	
	// 3. 初始化所有服务
	InitializeServices();
	
	// 4. 初始化 UI 系统
	InitializeUISystem();
	
	// 5. 初始化市场定价服务（无状态，不需要DataTable）
	MarketManager = NewObject<UMarketManager01>(this);
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 市场定价服务初始化完成"));
	
	// 6. 生成测试城镇（城镇创建时会检查地块资源）
	SpawnTestTown();
	
	// 7. 生成六边形网格可视化
	if (bShowHexGridMesh && HexGridActorClass)
	{
		GenerateHexGridMeshes();
	}
	
	// 8. 初始化路径可视化系统
	InitializePathVisualizer();
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 初始化完成"));
}

void ADemo01_GM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 执行时间管理器的 Execute 函数
	if (TimeManager)
	{
		TimeManager->Execute(DeltaTime);
	}
	
	// 绘制网格坐标标签（如果启用）
	if (bShowGridLabels)
	{
		DrawGridLabels();
	}
}

void ADemo01_GM::InitializeGridSystem(const FDemo01GridConfig& Config)
{
	GridConfig = Config;
	
	// 获取 GridService
	GridService = GetWorld()->GetSubsystem<UGridService>();
	if (!GridService)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 无法获取 GridService"));
		return;
	}
	
	// 配置 GridService
	FGridLayoutParams LayoutParams;
	LayoutParams.CellSize = GridConfig.HexRadius;
	LayoutParams.bUseHexGrid = true;
	LayoutParams.bFlatTop = GridConfig.bIsFlatTop;
	LayoutParams.GridWidth = GridConfig.GridWidth;
	LayoutParams.GridHeight = GridConfig.GridHeight;
	LayoutParams.CalculateGridOffset();
	
	UE_LOG(LogTemp, Warning, TEXT("========== 网格系统初始化 =========="));
	UE_LOG(LogTemp, Warning, TEXT("[配置] 网格大小: %dx%d"), GridConfig.GridWidth, GridConfig.GridHeight);
	UE_LOG(LogTemp, Warning, TEXT("[配置] 六边形半径: %.2f"), GridConfig.HexRadius);
	UE_LOG(LogTemp, Warning, TEXT("[配置] 平顶六边形: %s"), GridConfig.bIsFlatTop ? TEXT("是") : TEXT("否"));
	UE_LOG(LogTemp, Warning, TEXT("[配置] 网格偏移: (%.2f, %.2f)"), LayoutParams.GridOffset.X, LayoutParams.GridOffset.Y);
	
	if (!GridService->SetGridLayout(LayoutParams))
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] GridService 设置失败"));
		return;
	}
	
	// 初始化网格单元数据（可选）
	InitializeGridCells();
	
	// Debug 绘制网格
	if (bDebugDrawGrid)
	{
		// 调用原本的 GridService 绘制
		DrawDebugGridCells();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("===================================="));
}

FVector ADemo01_GM::GridCoordToWorld(FIntVector2 Coord) const
{
	if (GridService)
	{
		FGridTransformResult Result = GridService->GridToWorld(Coord, FVector::ZeroVector);
		return Result.WorldPosition;
	}
	
	// 备用：简单转换
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] GridService 不可用，使用简单转换"));
	return FVector(Coord.X * GridConfig.HexRadius, Coord.Y * GridConfig.HexRadius, 0.0f);
}

FIntVector2 ADemo01_GM::WorldToGridCoord(FVector WorldLocation) const
{
	FIntVector2 RawCoord;
	
	if (GridService)
	{
		RawCoord = GridService->WorldToGrid(WorldLocation, FVector::ZeroVector);
	}
	else
	{
		// 备用：简单转换
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] GridService 不可用，使用简单转换"));
		RawCoord = FIntVector2(
			FMath::RoundToInt(WorldLocation.X / GridConfig.HexRadius),
			FMath::RoundToInt(WorldLocation.Y / GridConfig.HexRadius)
		);
	}
	
	// 总是记录原始转换结果
	UE_LOG(LogTemp, Warning, TEXT("[坐标转换] 世界坐标 (%.2f, %.2f, %.2f) -> 原始网格坐标 (%d, %d)"), 
		WorldLocation.X, WorldLocation.Y, WorldLocation.Z, RawCoord.X, RawCoord.Y);
	
	// 限制坐标在有效范围内
	FIntVector2 ClampedCoord;
	ClampedCoord.X = FMath::Clamp(RawCoord.X, 0, GridConfig.GridWidth - 1);
	ClampedCoord.Y = FMath::Clamp(RawCoord.Y, 0, GridConfig.GridHeight - 1);
	
	// 如果发生了 Clamp，打印 Clamp 后的坐标
	if (RawCoord.X != ClampedCoord.X || RawCoord.Y != ClampedCoord.Y)
	{
		UE_LOG(LogTemp, Warning, TEXT("[坐标转换] Clamp 后网格坐标: (%d, %d)"), 
			ClampedCoord.X, ClampedCoord.Y);
	}
	
	return ClampedCoord;
}

void ADemo01_GM::HandleMoveCommand(AMoveableEntity01* Entity, FIntVector2 TargetCoord, bool bAppendPath)
{
	if (UnitControlManager)
	{
		UnitControlManager->HandleMoveCommand(Entity, TargetCoord, bAppendPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 移动命令失败：UnitControlManager未初始化"));
	}
}

void ADemo01_GM::HandleMoveCommandByData(int32 UnitID, EEntityType01 UnitType, FIntVector2 TargetCoord, bool bAppendPath)
{
	if (UnitControlManager)
	{
		UnitControlManager->HandleMoveCommandByData(UnitID, UnitType, TargetCoord, bAppendPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 数据层移动命令失败：UnitControlManager未初始化"));
	}
}

void ADemo01_GM::InitializeServices()
{
	// 初始化寻路服务
	InitializePathfindingService();
	
	// 初始化时间管理器
	InitializeTimeManager();
	
	// 初始化生产系统
	InitializeProductionSystem();
	
	// 初始化单位控制管理器
	UnitControlManager = NewObject<UUnitControlManager01>(this);
	if (UnitControlManager)
	{
		UnitControlManager->Initialize(this);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 单位控制管理器初始化完成"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 单位控制管理器初始化失败"));
	}
	
	// 初始化贸易路线管理器
	TradeRouteManager = NewObject<UTradeRouteManager01>(this);
	if (TradeRouteManager)
	{
		TradeRouteManager->Initialize(this);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 贸易路线管理器初始化完成"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 贸易路线管理器初始化失败"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 所有服务初始化完成"));
}

void ADemo01_GM::InitializePathVisualizer()
{
	if (!PathVisualizerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] PathVisualizerClass未设置，路径可视化功能将不可用"));
		return;
	}
	
	// 创建路径可视化Actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	PathVisualizer = GetWorld()->SpawnActor<APathVisualizerActor01>(
		PathVisualizerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);
	
	if (PathVisualizer)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 路径可视化Actor创建成功"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 路径可视化Actor创建失败"));
	}
}

void ADemo01_GM::InitializePathfindingService()
{
	PathfindingService = NewObject<UPathfindingService01>(this);
	if (PathfindingService)
	{
		PathfindingService->Initialize(this, GridConfig);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 寻路服务初始化完成"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 寻路服务初始化失败"));
	}
}

void ADemo01_GM::InitializeTimeManager()
{
	TimeManager = NewObject<UTimeManager01>(this);
	if (TimeManager)
	{
		TimeManager->Initialize(this);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 时间管理器初始化完成"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 时间管理器初始化失败"));
	}
}

void ADemo01_GM::InitializeProductionSystem()
{
	ProductionManager = NewObject<UProductionManager01>(this);
	if (ProductionManager)
	{
		if (ProductDataTable && RecipeDataTable)
		{
			ProductionManager->Initialize(ProductDataTable, RecipeDataTable);
			UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 生产系统初始化完成"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 生产系统数据表未配置，跳过初始化"));
			if (!ProductDataTable)
			{
				UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] ProductDataTable 未配置！"));
			}
			if (!RecipeDataTable)
			{
				UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] RecipeDataTable 未配置！"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 生产管理器创建失败"));
	}
}

void ADemo01_GM::InitializeUISystem()
{
	UIManager = NewObject<UDemo01UIManager>(this);
	if (UIManager)
	{
		UIManager->Initialize(this, UIConfig);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] UI 系统初始化完成"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] UI 系统初始化失败"));
	}
}

void ADemo01_GM::TestBlueprintWidget()
{
	if (!BlueprintOnlyWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BlueprintOnlyWidgetClass is not set!"));
		return;
	}
	UUserWidget* TestWidget = CreateWidget<UUserWidget>(GetWorld(), BlueprintOnlyWidgetClass);
	if (TestWidget)
	{
		TestWidget->AddToViewport();

		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			PlayerController->SetInputMode(FInputModeUIOnly());
			PlayerController->SetShowMouseCursor(true);
		}

		UE_LOG(LogTemp, Warning, TEXT("Blueprint-only Widget created and added to viewport"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create Blueprint-only Widget"));
	}
}

// ===== 网格初始化和绘制 =====

void ADemo01_GM::InitializeGridCells()
{
	GridCells.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 开始初始化网格单元数据：%dx%d"), 
		GridConfig.GridWidth, GridConfig.GridHeight);
	
	for (int32 Y = 0; Y < GridConfig.GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridConfig.GridWidth; ++X)
		{
			FIntVector2 Coord(X, Y);
			FDemo01GridCell Cell;
			Cell.Coord = Coord;
			Cell.bIsWalkable = true;
			Cell.MovementCost = 1.0f;
			
			GridCells.Add(Coord, Cell);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 网格单元数据初始化完成：共 %d 个格子"), GridCells.Num());
}

void ADemo01_GM::DrawDebugGridCells()
{
	if (!GridService)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 无法绘制网格：GridService 为空"));
		return;
	}
	
	if (GridCells.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 无法绘制网格：网格数据为空"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 开始绘制网格：共 %d 个格子"), GridCells.Num());
	
	int32 PrintCount = 0;
	const int32 MaxPrintCount = 5;
	
	for (const auto& Pair : GridCells)
	{
		const FIntVector2& Coord = Pair.Key;
		
		if (PrintCount < MaxPrintCount)
		{
			FGridTransformResult Result = GridService->GridToWorld(Coord, FVector::ZeroVector);
			UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 格子 Grid(%d,%d) -> World(%.1f, %.1f, %.1f)"), 
				Coord.X, Coord.Y, 
				Result.WorldPosition.X, Result.WorldPosition.Y, Result.WorldPosition.Z);
			PrintCount++;
		}
		
		GridService->DrawDebugCell(
			Coord, 
			FVector::ZeroVector, 
			DebugGridColor, 
			DebugDrawDuration, 
			DebugLineThickness
		);
	}
	
	FString Status = GridService->GetServiceStatus();
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] GridService 状态:\n%s"), *Status);
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 网格绘制完成"));
}

// ===== Debug 功能 =====

void ADemo01_GM::ToggleGridDrawing()
{
	bDebugDrawGrid = !bDebugDrawGrid;
	
	if (bDebugDrawGrid)
	{
		DrawDebugGridCells();
	}
}

void ADemo01_GM::RedrawGrid()
{
	if (!bDebugDrawGrid)
	{
		return;
	}
	
	DrawDebugGridCells();
}

void ADemo01_GM::ToggleGridLabels()
{
	bShowGridLabels = !bShowGridLabels;
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 网格坐标标签显示: %s"), bShowGridLabels ? TEXT("开启") : TEXT("关闭"));
}

void ADemo01_GM::DrawGridLabels()
{
	if (!GridService || GridCells.Num() == 0)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	for (const auto& Pair : GridCells)
	{
		const FIntVector2& Coord = Pair.Key;
		FVector WorldPos = GridCoordToWorld(Coord);
		
		FString CoordText = FString::Printf(TEXT("(%d,%d)"), Coord.X, Coord.Y);
		DrawDebugString(World, WorldPos + FVector(0, 0, 50), CoordText, nullptr, FColor::White, 0.0f, false, 1.0f);
	}
}

// ===== 资源系统实现 =====

void ADemo01_GM::GenerateGridResources()
{
	if (GridCells.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateGridResources: GridCells为空，无法生成资源"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("开始生成网格资源分布，共%d个格子"), GridCells.Num());
	
	// 遍历所有格子
	for (auto& CellPair : GridCells)
	{
		FIntVector2 Coord = CellPair.Key;
		FDemo01GridCell& Cell = CellPair.Value;
		
		// 随机选择资源类型
		FString SelectedResource = GetRandomResourceType();
		Cell.ResourceType = SelectedResource;
		
		UE_LOG(LogTemp, Verbose, TEXT("格子(%d, %d) 资源: %s"), 
			Coord.X, Coord.Y, *SelectedResource);
	}
	
	UE_LOG(LogTemp, Log, TEXT("网格资源分布生成完成"));
}

FString ADemo01_GM::GetRandomResourceType() const
{
	if (ResourceGenerationWeights.IsEmpty())
	{
		return "";  // 默认无资源
	}
	
	float RandomValue = FMath::FRand();  // 0-1随机数
	float CumulativeWeight = 0.0f;
	
	for (const auto& Pair : ResourceGenerationWeights)
	{
		CumulativeWeight += Pair.Value;
		if (RandomValue <= CumulativeWeight)
		{
			return Pair.Key;
		}
	}
	
	return "";  // 默认返回无资源
}

void ADemo01_GM::CheckTownResourceAndUnlockBuildings(ATownActor01* Town, FIntVector2 GridCoord)
{
	if (!Town) return;

	// 获取地块资源
	if (const FDemo01GridCell* Cell = GridCells.Find(GridCoord))
	{
		Town->TownData.LocalResourceType = Cell->ResourceType;
		Town->TownData.UnlockedBuildingIDs = GetUnlockedBuildingsForResource(Cell->ResourceType);
		
		// 同步到PlayerState
		ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (PS)
		{
			FTownData01 CurrentData = PS->GetTownData(Town->EntityID);
			CurrentData.LocalResourceType = Cell->ResourceType;
			CurrentData.UnlockedBuildingIDs = GetUnlockedBuildingsForResource(Cell->ResourceType);
			PS->UpdateTownData(Town->EntityID, CurrentData);
		}
		
		if (!Cell->ResourceType.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("城镇 %s 创建在资源地块上: %s，解锁建筑: %d个"),
				*Town->TownData.EntityName,
				*Cell->ResourceType,
				Town->TownData.UnlockedBuildingIDs.Num());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("城镇 %s 创建在普通地块上"), *Town->TownData.EntityName);
		}
	}
}

TArray<FString> ADemo01_GM::GetUnlockedBuildingsForResource(const FString& ResourceType) const
{
	if (ResourceType.IsEmpty())
	{
		return {};  // 无资源不解锁特殊建筑
	}
	
	const FRecipeIDArray* UnlockedBuildings = ResourceBuildingUnlocks.Find(ResourceType);
	if (UnlockedBuildings)
	{
		return UnlockedBuildings->RecipeIDs;
	}
	
	return {};
}

void ADemo01_GM::SpawnTestTown()
{
	if (!TestTownClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 无法生成测试城镇：TestTownClass 未设置"));
		return;
	}
	
	if (!GridService)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 无法生成测试城镇：GridService 为空"));
		return;
	}
	
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 无法生成测试城镇：PlayerState 为空"));
		return;
	}

	// 两个城镇的名称和坐标偏移
	struct FTownSpawnConfig { FString Name; FIntVector2 Offset; };
	TArray<FTownSpawnConfig> Configs = {
		{ TEXT("铁石镇"), FIntVector2(-4, 0) },
		{ TEXT("谷仓城"), FIntVector2( 4, 0) }
	};

	FIntVector2 CenterCoord(GridConfig.GridWidth / 2, GridConfig.GridHeight / 2);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FTownSpawnConfig& Cfg : Configs)
	{
		FIntVector2 Coord = FIntVector2(CenterCoord.X + Cfg.Offset.X, CenterCoord.Y + Cfg.Offset.Y);
		FVector SpawnLocation = GridCoordToWorld(Coord);

		ATownActor01* NewTown = GetWorld()->SpawnActor<ATownActor01>(
			TestTownClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (!NewTown)
		{
			UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] 城镇 %s 生成失败"), *Cfg.Name);
			continue;
		}

		// 注释：模型底部在原点Z轴高度，不需要Z轴调整
		// FVector Origin, Extent;
		// NewTown->GetActorBounds(false, Origin, Extent);
		// FVector AdjustedLocation = SpawnLocation;
		// AdjustedLocation.Z += -(Origin.Z - Extent.Z);
		// NewTown->SetActorLocation(AdjustedLocation);
		
		NewTown->SetActorLocation(SpawnLocation);

		NewTown->InitializeGridCoord(Coord);
		NewTown->TownData.EntityName = Cfg.Name;
		NewTown->TownData.GridCoord  = Coord;
		NewTown->TownData.Population = 1000;

		PS->RegisterTown(NewTown);
		
		// 检查城镇资源并解锁建筑
		CheckTownResourceAndUnlockBuildings(NewTown, Coord);

		// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] ========== 城镇 %s (ID:%d) 创建完成 =========="), *NewTown->TownData.EntityName, NewTown->EntityID);
		
		TArray<FString> AvailableAfterRegister = NewTown->GetAvailableRecipeIDs();
		// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] RegisterTown 后 AvailableRecipeIDs 数量: %d"), AvailableAfterRegister.Num());

		InitializeTownWithNativeBuildings(NewTown, 3);

		TArray<FString> AvailableAfterInit = NewTown->GetAvailableRecipeIDs();
		// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] InitializeTownWithNativeBuildings 后 AvailableRecipeIDs 数量: %d"), AvailableAfterInit.Num());

		NewTown->TownData = PS->GetTownData(NewTown->EntityID);

		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 城镇已生成：%s (ID:%d) 网格(%d,%d)"),
			*NewTown->TownData.EntityName, NewTown->TownData.EntityID, Coord.X, Coord.Y);
	}
}

// ===== 商队生成 =====

AMoveableEntity01* ADemo01_GM::SpawnCaravanFromTown(int32 CaravanID, ATownActor01* Town)
{
	if (UnitControlManager)
	{
		return UnitControlManager->SpawnCaravanFromTown(CaravanID, Town);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] SpawnCaravanFromTown失败：UnitControlManager未初始化"));
		return nullptr;
	}
}

AMoveableEntity01* ADemo01_GM::SpawnCaravanInstanceAtCoord(int32 CaravanID, FIntVector2 GridCoord)
{
	if (UnitControlManager)
	{
		return UnitControlManager->SpawnCaravanInstanceAtCoord(CaravanID, GridCoord);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] SpawnCaravanInstanceAtCoord失败：UnitControlManager未初始化"));
		return nullptr;
	}
}

// ===== 殖民者管理 =====

AMoveableEntity01* ADemo01_GM::SpawnColonistFromTown(int32 ColonistID, ATownActor01* Town)
{
	if (UnitControlManager)
	{
		return UnitControlManager->SpawnColonistFromTown(ColonistID, Town);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] SpawnColonistFromTown失败：UnitControlManager未初始化"));
		return nullptr;
	}
}

AMoveableEntity01* ADemo01_GM::SpawnColonistInstanceAtCoord(int32 ColonistID, FIntVector2 GridCoord)
{
	if (UnitControlManager)
	{
		return UnitControlManager->SpawnColonistInstanceAtCoord(ColonistID, GridCoord);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] SpawnColonistInstanceAtCoord失败：UnitControlManager未初始化"));
		return nullptr;
	}
}

// ===== 生产系统 =====

void ADemo01_GM::ExecuteAllTownsProduction()
{
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return;
	}

	TArray<int32> TownIDs = PS->GetAllTownIDs();
	for (int32 TownID : TownIDs)
	{
		ATownActor01* Town = PS->FindTownInstance(TownID);
		if (Town)
		{
			Town->ExecuteAllProduction();
		}
	}
}

void ADemo01_GM::OnGameDayPassed()
{
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 新的一天开始，触发城镇生产"));
	ExecuteAllTownsProduction();
}

ATownActor01* ADemo01_GM::CreateCityFromColonist(int32 ColonistID)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] CreateCityFromColonist: 世界为空"));
		return nullptr;
	}

	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] CreateCityFromColonist: 无法获取PlayerState"));
		return nullptr;
	}

	FColonistData01* ColonistData = PS->GetColonistData(ColonistID);
	if (!ColonistData)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] CreateCityFromColonist: 未找到殖民者数据 ID: %d"), ColonistID);
		return nullptr;
	}

	FIntVector2 CityCoord = ColonistData->GridCoord;
	ATownActor01* ExistingTown = GetTownAtCoord(CityCoord);
	if (ExistingTown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] CreateCityFromColonist: 位置 (%d,%d) 已有城镇 %s"),
			CityCoord.X, CityCoord.Y, *ExistingTown->TownData.EntityName);
		return nullptr;
	}

	if (!TestTownClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] CreateCityFromColonist: TestTownClass 未设置"));
		return nullptr;
	}

	// 使用殖民者当前坐标作为建城位置
	FVector CityLocation = GridCoordToWorld(CityCoord);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATownActor01* NewTown = GetWorld()->SpawnActor<ATownActor01>(
		TestTownClass,
		CityLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!NewTown)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] CreateCityFromColonist: 城镇生成失败"));
		return nullptr;
	}

	// 调整Z轴
	//FVector Origin, Extent;
	// 注释：模型底部在原点Z轴高度，不需要Z轴调整
	// NewTown->GetActorBounds(false, Origin, Extent);
	// float BottomZ = Origin.Z - Extent.Z;
	// FVector AdjustedLocation = CityLocation;
	// AdjustedLocation.Z += -BottomZ;
	// NewTown->SetActorLocation(AdjustedLocation);
	
	NewTown->SetActorLocation(CityLocation);

	NewTown->InitializeGridCoord(CityCoord);
	NewTown->TownData.GridCoord = CityCoord;
	NewTown->TownData.Population = 500;
	PS->RegisterTown(NewTown);
	
	// 检查城镇资源并解锁建筑
	CheckTownResourceAndUnlockBuildings(NewTown, CityCoord);
	
	// 初始化基础建筑（与GameMode建城保持一致）
	InitializeTownWithNativeBuildings(NewTown, 3);

	// 销毁实例（如有），移除殖民者数据
	if (AMoveableEntity01* Inst = ColonistData->Instance)
	{
		ColonistData->Instance = nullptr;
		Inst->Destroy();
	}
	PS->RemoveColonistData(ColonistID);
	
	// 关闭殖民者相关的UI面板（如果打开，带ID检查）
	if (UIManager)
	{
		UIManager->CloseColonistInfoPanelByID(ColonistID);
	}

	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] CreateCityFromColonist: 成功建立城镇 %s (ID: %d) 在 (%d,%d)"),
		*NewTown->TownData.EntityName, NewTown->TownData.EntityID, CityCoord.X, CityCoord.Y);

	return NewTown;
}

void ADemo01_GM::InitializeTestTownData(ATownActor01* Town)
{
	if (!Town || !ProductionManager || !ProductionManager->IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 无法初始化测试城镇数据：城镇或生产管理器为空"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] ========== 开始初始化测试城镇数据：%s (ID:%d) =========="), *Town->TownData.EntityName, Town->EntityID);

	TArray<FString> AvailableBefore = Town->GetAvailableRecipeIDs();
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] InitializeTestTownData 前 AvailableRecipeIDs 数量: %d"), AvailableBefore.Num());

	// 初始化可建造配方列表
	Town->InitializeAvailableRecipes();

	TArray<FString> AvailableAfter = Town->GetAvailableRecipeIDs();
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] InitializeTestTownData::InitializeAvailableRecipes 后 AvailableRecipeIDs 数量: %d"), AvailableAfter.Num());

	// 添加一些初始资源
	Town->AddResource(TEXT("wood"), 50);
	Town->AddResource(TEXT("stone"), 30);
	Town->AddResource(TEXT("iron_ore"), 20);
	Town->AddResource(TEXT("grain"), 40);
	Town->AddResource(TEXT("bread"), 15);
	Town->AddResource(TEXT("tools"), 5);

	// 添加一些初始建筑
	int32 BuildingID1 = Town->AddBuilding(FText::FromString(TEXT("伐木场")), TEXT("basic_logging"));
	int32 BuildingID2 = Town->AddBuilding(FText::FromString(TEXT("采石场")), TEXT("basic_mining"));
	int32 BuildingID3 = Town->AddBuilding(FText::FromString(TEXT("面包房")), TEXT("bread_baking"));

	// 激活一些建筑
	if (BuildingID1 != -1)
	{
		Town->SetBuildingActive(BuildingID1, true);
	}
	if (BuildingID2 != -1)
	{
		Town->SetBuildingActive(BuildingID2, true);
	}

	// 同步数据到PlayerState
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		PS->UpdateTownData(Town->EntityID, Town->TownData);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 城镇数据已同步到PlayerState"));
	}

	TArray<FString> AvailableFinal = Town->GetAvailableRecipeIDs();
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] InitializeTestTownData 最终 AvailableRecipeIDs 数量: %d"), AvailableFinal.Num());

	UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] ========== 测试城镇数据初始化完成 =========="));
}

void ADemo01_GM::InitializeTownWithNativeBuildings(ATownActor01* Town, int32 NumNativeBuildings)
{
	// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] ========== 开始初始化城镇 %s (ID:%d) 的原料建筑 =========="), *Town->TownData.EntityName, Town->EntityID);

	if (!Town || !ProductionManager || !ProductionManager->IsInitialized())
	{
		// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] InitializeTownWithNativeBuildings: 城镇或生产管理器未就绪"));
		return;
	}

	TArray<FString> AvailableBefore = Town->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] InitializeTownWithNativeBuildings 前 AvailableRecipeIDs 数量: %d"), AvailableBefore.Num());

	// 获取城市坐标
	FIntVector2 CityCoord = Town->GetGridCoord();
	
	// 基于地理资源分布选择配方（完全由地理资源决定，不进行随机补充）
	TArray<FString> SelectedRecipes = SelectNativeRecipesByGeography(CityCoord);
	
	// 限制最大数量（不超过NumNativeBuildings，但允许为0）
	SelectedRecipes.SetNum(FMath::Min(SelectedRecipes.Num(), NumNativeBuildings));
	
	if (SelectedRecipes.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] %s: 位置(%d,%d)没有发现任何资源，不建造基础资源建筑"), 
			*Town->TownData.EntityName, CityCoord.X, CityCoord.Y);
	}
	
	// 确保建筑类型不重复
	TArray<FString> FinalRecipes;
	TArray<FString> UsedBuildingTypes;
	for (const FString& RecipeID : SelectedRecipes)
	{
		FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RecipeID);
		FString BuildingType = RecipeData.ProductionFacility.ToString();
		
		if (!BuildingType.IsEmpty() && !UsedBuildingTypes.Contains(BuildingType))
		{
			FinalRecipes.Add(RecipeID);
			UsedBuildingTypes.Add(BuildingType);
			// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 选取配方: %s (建筑类型: %s)"), *RecipeID, *BuildingType);
		}
	}
	
	TArray<FString> RawRecipes = FinalRecipes;
	int32 Count = RawRecipes.Num();

	// 先给城镇一定量的工具（原料配方的通用输入）
	Town->AddResource(TEXT("tools"), 20);

	// 1. 写入 NativeRecipeIDs 白名单
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(Town->EntityID);
		// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 写入 NativeRecipeIDs 前数量: %d"), CurrentData.NativeRecipeIDs.Num());
		CurrentData.NativeRecipeIDs.Empty();
		for (int32 i = 0; i < Count; ++i)
		{
			CurrentData.NativeRecipeIDs.Add(RawRecipes[i]);
			// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM]   添加 NativeRecipe: %s"), *RawRecipes[i]);
		}
		PS->UpdateTownData(Town->EntityID, CurrentData);
		Town->TownData = CurrentData;
		// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 写入 NativeRecipeIDs 后数量: %d"), CurrentData.NativeRecipeIDs.Num());
	}

	// 2. 用白名单初始化可建造列表（现在 AddBuilding 不再移除配方）
	Town->InitializeAvailableRecipes();

	TArray<FString> AvailableAfterInit = Town->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] InitializeTownWithNativeBuildings::InitializeAvailableRecipes 后 AvailableRecipeIDs 数量: %d"), AvailableAfterInit.Num());

	// 3. 建造并激活原料建筑，添加初始库存
	TArray<FString> BuiltRecipeIDs;
	for (int32 i = 0; i < Count; ++i)
	{
		const FString& RecipeID = RawRecipes[i];
		FRecipeData01 Recipe = ProductionManager->GetRecipeData(RecipeID);

		// 为每个原料建筑的主产出添加初始库存 50
		if (Recipe.Outputs.Num() > 0)
		{
			Town->AddResource(Recipe.Outputs[0].ResourceID, 50);
		}

		// 建造并激活建筑（AddBuilding 不再移除配方）
		int32 BID = Town->AddBuilding(Recipe.ProductionFacility, RecipeID);
		if (BID != -1)
		{
			Town->SetBuildingActive(BID, true);
			BuiltRecipeIDs.Add(RecipeID);
		}
	}

	TArray<FString> AvailableAfterBuild = Town->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 建造原料建筑后 AvailableRecipeIDs 数量: %d"), AvailableAfterBuild.Num());

	// 4. 重新初始化可建造配方列表，确保建造完成后可建造列表正确更新
	// （现在 AddBuilding 不再移除配方，所以这一步是为了更新 AvailableRecipeIDs 以反映已建造的建筑类型）
	Town->InitializeAvailableRecipes();

	TArray<FString> AvailableAfterReinit = Town->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] 重新初始化后 AvailableRecipeIDs 数量: %d"), AvailableAfterReinit.Num());

	// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] ========== 初始化城镇原料建筑完成 =========="));

	FString RecipeList = FString::Join(BuiltRecipeIDs, TEXT(", "));
	// UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] %s: 原料建筑初始化完成，共 %d 个 [%s]"),
	// 	*Town->TownData.EntityName, BuiltRecipeIDs.Num(), *RecipeList);
}

bool ADemo01_GM::CanFoundCityAtCoord(FIntVector2 Coord, int32 MinDistance) const
{
	// 1. 检查该坐标是否已有城镇
	if (GetTownAtCoord(Coord) != nullptr)
	{
		return false;
	}
	
	// 2. 检查距离所有城市的距离
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return false;
	}
	
	// 获取所有城镇ID
	TArray<int32> AllTownIDs = PS->GetAllTownIDs();
	
	// 遍历所有城镇，检查距离
	for (int32 TownID : AllTownIDs)
	{
		FTownData01 TownData = PS->GetTownData(TownID);
		if (TownData.EntityID != TownID) continue; // 跳过无效数据
		
		// 计算六边形距离（使用PathfindingService的启发式函数）
		int32 Distance = FMath::Abs(Coord.X - TownData.GridCoord.X) + 
		                 FMath::Abs(Coord.Y - TownData.GridCoord.Y) + 
		                 FMath::Abs(-Coord.X - Coord.Y - (-TownData.GridCoord.X - TownData.GridCoord.Y));
		Distance = Distance / 2;
		
		if (Distance < MinDistance)
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Demo01_GM] CanFoundCityAtCoord: 坐标 (%d,%d) 距离城镇 %s 只有 %d 格，小于最小距离 %d"),
				Coord.X, Coord.Y, *TownData.EntityName, Distance, MinDistance);
			return false;
		}
	}
	
	return true;
}

TArray<FString> ADemo01_GM::SelectNativeRecipesByGeography(FIntVector2 CityCoord)
{
	TArray<FString> ResultRecipes;
	
	if (!ProductionManager || !ProductionManager->IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] SelectNativeRecipesByGeography: 生产管理器未就绪"));
		return ResultRecipes;
	}
	
	// 获取城市及其周围格子的资源
	TArray<FString> AvailableResources = GetResourcesInCityRadius(CityCoord);
	
	// 如果没有任何资源，返回空列表
	if (AvailableResources.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 位置(%d,%d)没有发现任何资源"), CityCoord.X, CityCoord.Y);
		return ResultRecipes;
	}
	
	// 获取所有ProductLevel==0的配方
	TArray<FString> AllLevel0Recipes = ProductionManager->GetRecipesByProductLevel(0);
	
	// 根据可用资源筛选配方
	for (const FString& RecipeID : AllLevel0Recipes)
	{
		FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RecipeID);
		
		// 检查这个配方的主要产品是否在可用资源中
		if (AvailableResources.Contains(RecipeData.ProductID))
		{
			ResultRecipes.Add(RecipeID);
			UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 为位置(%d,%d)选择配方: %s (资源: %s)"), 
				CityCoord.X, CityCoord.Y, *RecipeID, *RecipeData.ProductID);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 为位置(%d,%d)选择了 %d 个基于地理资源的配方"), 
		CityCoord.X, CityCoord.Y, ResultRecipes.Num());
	
	return ResultRecipes;
}

TArray<FString> ADemo01_GM::GetResourcesInCityRadius(FIntVector2 CityCoord)
{
	TArray<FString> Resources;
	
	// 获取GridService实例（使用不同的变量名避免冲突）
	UGridService* GridServiceInstance = GetWorld()->GetSubsystem<UGridService>();
	if (!GridServiceInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] GetResourcesInCityRadius: GridService未找到"));
		return Resources;
	}
	
	// 获取城市所在格子及其周围六个格子
	TArray<FIntVector2> AllCoords;
	AllCoords.Add(CityCoord); // 城市所在格子
	
	// 获取周围六个邻居格子（使用正确的GetHexNeighbors方法）
	TArray<FIntVector2> Neighbors = GridServiceInstance->GetHexNeighbors(CityCoord);
	AllCoords.Append(Neighbors);
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 查询位置(%d,%d)及其周围 %d 个格子的资源"), 
		CityCoord.X, CityCoord.Y, Neighbors.Num());
	
	// 查询每个格子的资源（使用demo01自己的GridCells系统）
	for (const FIntVector2& Coord : AllCoords)
	{
		// 在GridCells中查找该坐标的网格单元
		if (GridCells.Contains(Coord))
		{
			const FDemo01GridCell& GridCell = GridCells[Coord];
			
			// 如果格子有资源类型，添加到资源列表
			if (!GridCell.ResourceType.IsEmpty())
			{
				Resources.Add(GridCell.ResourceType);
				UE_LOG(LogTemp, Verbose, TEXT("[Demo01_GM] 格子(%d,%d)发现资源: %s"), 
					Coord.X, Coord.Y, *GridCell.ResourceType);
			}
		}
	}
	
	// 去重并返回（使用正确的去重方法）
	Resources.Sort();
	TArray<FString> UniqueResources;
	for (const FString& Resource : Resources)
	{
		if (!UniqueResources.Contains(Resource))
		{
			UniqueResources.Add(Resource);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] 位置(%d,%d)周围发现 %d 种不同的资源"), 
		CityCoord.X, CityCoord.Y, UniqueResources.Num());
	
	return UniqueResources;
}

int32 ADemo01_GM::GetNearestTownDistance(FIntVector2 Coord) const
{
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return -1;
	}
	
	TArray<int32> AllTownIDs = PS->GetAllTownIDs();
	if (AllTownIDs.Num() == 0)
	{
		return -1;
	}
	
	int32 MinDistance = INT32_MAX;
	for (int32 TownID : AllTownIDs)
	{
		FTownData01 TownData = PS->GetTownData(TownID);
		if (TownData.EntityID != TownID) continue;
		
		// 计算六边形距离
		int32 Distance = FMath::Abs(Coord.X - TownData.GridCoord.X) + 
		                 FMath::Abs(Coord.Y - TownData.GridCoord.Y) + 
		                 FMath::Abs(-Coord.X - Coord.Y - (-TownData.GridCoord.X - TownData.GridCoord.Y));
		Distance = Distance / 2;
		
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
		}
	} // added closing bracket here
	return MinDistance;
}


ATownActor01* ADemo01_GM::GetTownAtCoord(FIntVector2 Coord) const
{
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return nullptr;
	}

	TArray<int32> AllTownIDs = PS->GetAllTownIDs();
	for (int32 TownID : AllTownIDs)
	{
		ATownActor01* Town = PS->FindTownInstance(TownID);
		if (Town && Town->GetGridCoord() == Coord)
		{
			return Town;
		}

		// 若无实例，通过数据查找
		FTownData01 TownData = PS->GetTownData(TownID);
		if (TownData.EntityID == TownID && TownData.GridCoord == Coord)
		{
			return Town; // 可能为 nullptr（无实例）
		}
	}

	return nullptr;
}

void ADemo01_GM::GenerateHexGridMeshes()
{
	if (!HexGridActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_GM] GenerateHexGridMeshes: HexGridActorClass 未设置"));
		return;
	}

	ClearHexGridMeshes();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const auto& Pair : GridCells)
	{
		const FIntVector2& Coord = Pair.Key;
		const FDemo01GridCell& Cell = Pair.Value;
		FVector WorldPos = GridCoordToWorld(Coord);

		// 生成 AHexGridActorBase 类型的 Actor
		AHexGridActorBase* HexActor = GetWorld()->SpawnActor<AHexGridActorBase>(
			HexGridActorClass,
			WorldPos,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (HexActor)
		{
			// 设置网格坐标
			HexActor->GridCoord = Coord;

			// 设置资源类型（如果有）
			if (!Cell.ResourceType.IsEmpty())
			{
				HexActor->ResourceType = Cell.ResourceType;
				UE_LOG(LogTemp, Verbose, TEXT("[Demo01_GM] 设置地块(%d,%d)资源类型: %s"),
					Coord.X, Coord.Y, *Cell.ResourceType);
			}

			// 立即更新视觉效果
			HexActor->UpdateVisuals();

			HexGridActors.Add(HexActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] GenerateHexGridMeshes: 生成了 %d 个六边形格子，已设置资源可视化"),
		HexGridActors.Num());
}

void ADemo01_GM::ClearHexGridMeshes()
{
	for (AActor* HexActor : HexGridActors)
	{
		if (IsValid(HexActor))
		{
			HexActor->Destroy();
		}
	}
	HexGridActors.Empty();
	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] ClearHexGridMeshes: 已清除所有六边形格子"));
}

bool ADemo01_GM::PurchaseColonist(ATownActor01* Town, const FString& ColonistName)
{
	if (!Town)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] PurchaseColonist: 城镇为空"));
		return false;
	}

	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] PurchaseColonist: 无法获取PlayerState"));
		return false;
	}

	int32 ColonistID = PS->CreateColonistData(Town->EntityID, ColonistName);
	if (ColonistID < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_GM] PurchaseColonist: 创建殖民者数据失败"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[Demo01_GM] PurchaseColonist: 殖民者已创建 ID:%d 城镇:%s"), ColonistID, *Town->TownData.EntityName);

	if (UIManager)
	{
		UIManager->RefreshTownInfoPanelIfOpen(Town);
	}

	return true;
}