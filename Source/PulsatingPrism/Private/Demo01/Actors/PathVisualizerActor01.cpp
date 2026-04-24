// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Actors/PathVisualizerActor01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Core01/TradeRouteManager01.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

APathVisualizerActor01::APathVisualizerActor01()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));

	// 创建手动路径Spline组件
	ManualPathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("ManualPathSpline"));
	ManualPathSpline->SetupAttachment(RootComponent);
	ManualPathSpline->SetVisibility(false);

	// 创建贸易路线Spline组件
	TradeRouteSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TradeRouteSpline"));
	TradeRouteSpline->SetupAttachment(RootComponent);
	TradeRouteSpline->SetVisibility(false);
}

void APathVisualizerActor01::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 销毁延迟销毁的组件（避免迭代器失效）
	if (!PendingDestroyMeshes.IsEmpty())
	{
		for (USplineMeshComponent* Mesh : PendingDestroyMeshes)
		{
			if (Mesh)
			{
				Mesh->DestroyComponent();
			}
		}
		PendingDestroyMeshes.Empty();
	}

	// 只有手动路径需要动画和动态更新
	if (CurrentPathType == EPathType01::Manual)
	{
		// 路径延伸动画
		if (bIsExtendingPath)
		{
			const float Now = GetWorld()->GetRealTimeSeconds();
			UpdatePathExtension(Now - LastRealTime);
			LastRealTime = Now;
		}

		// 更新路径起点和修剪已走过的节点
		if (ManualPathSpline && ManualPathSpline->IsVisible())
		{
			UpdatePathStartPoint();
			TrimPassedWaypoints();
		}
	}
	// 贸易路线不需要Tick处理（静态显示）
}

void APathVisualizerActor01::ShowManualPath(int32 UnitID, EEntityType01 UnitType)
{
	// 清除贸易路线
	ClearTradeRoute();

	// 设置当前路径类型
	CurrentPathType = EPathType01::Manual;
	CurrentUnitID = UnitID;
	CurrentUnitType = UnitType;

	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[PathVisualizer] ShowManualPath: GameMode为空"));
		return;
	}

	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[PathVisualizer] ShowManualPath: PlayerState为空"));
		return;
	}

	// 获取路径数据
	TArray<FIntVector2> MovementPath;
	int32 StartPathIndex = 0;
	FVector StartPos;

	// 查找实体实例
	TrackedInstance = PS->FindMoveableEntityInstance(UnitID);

	if (UnitType == EEntityType01::Caravan)
	{
		FCaravanData01* Data = PS->GetCaravanDataPtr(UnitID);
		if (!Data)
		{
			UE_LOG(LogTemp, Error, TEXT("[PathVisualizer] ShowManualPath: 未找到商队数据"));
			return;
		}

		MovementPath = Data->MovementPath;
		StartPathIndex = Data->CurrentPathIndex;

		if (TrackedInstance)
		{
			StartPos = TrackedInstance->GetActorLocation();
		}
		else if (Data->State == ECaravanState01::InTown && Data->CurrentTownID != -1)
		{
			FTownData01 TownData = PS->GetTownData(Data->CurrentTownID);
			StartPos = GameMode->GridCoordToWorld(TownData.GridCoord);
		}
		else
		{
			StartPos = GameMode->GridCoordToWorld(Data->GridCoord);
		}
	}
	else if (UnitType == EEntityType01::Colonist)
	{
		FColonistData01* Data = PS->GetColonistData(UnitID);
		if (!Data)
		{
			UE_LOG(LogTemp, Error, TEXT("[PathVisualizer] ShowManualPath: 未找到殖民者数据"));
			return;
		}

		MovementPath = Data->MovementPath;
		StartPathIndex = Data->CurrentPathIndex;

		if (TrackedInstance)
		{
			StartPos = TrackedInstance->GetActorLocation();
		}
		else if (Data->State == EColonistState01::InTown && Data->CurrentTownID != -1)
		{
			FTownData01 TownData = PS->GetTownData(Data->CurrentTownID);
			StartPos = GameMode->GridCoordToWorld(TownData.GridCoord);
		}
		else
		{
			StartPos = GameMode->GridCoordToWorld(Data->GridCoord);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PathVisualizer] ShowManualPath: 未知单位类型"));
		return;
	}

	// 构建完整路径点
	ManualPathPoints.Empty();
	ManualPathPoints.Add(StartPos);

	for (int32 i = StartPathIndex; i < MovementPath.Num(); ++i)
	{
		FVector Point = GameMode->GridCoordToWorld(MovementPath[i]);
		ManualPathPoints.Add(Point);
	}

	if (ManualPathPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PathVisualizer] ShowManualPath: 路径点不足"));
		return;
	}

	// 初始化路径显示
	InitializeManualPathDisplay(ManualPathPoints, TrackedInstance, StartPathIndex);

	LastPathIndex = StartPathIndex;
}

void APathVisualizerActor01::ShowTradeRoute(int32 CaravanID)
{
	// 清除手动路径
	ClearManualPath();

	// 设置当前路径类型
	CurrentPathType = EPathType01::TradeRoute;
	CurrentUnitID = CaravanID;
	CurrentUnitType = EEntityType01::Caravan;

	ADemo01_GM* GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[PathVisualizer] ShowTradeRoute: GameMode为空"));
		return;
	}

	UTradeRouteManager01* TradeRouteManager = GameMode->GetTradeRouteManager();
	if (!TradeRouteManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[PathVisualizer] ShowTradeRoute: TradeRouteManager为空"));
		return;
	}

	// 获取贸易路线路径
	TArray<FIntVector2> TradePath = TradeRouteManager->GetTradeRoutePath(CaravanID);

	if (TradePath.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PathVisualizer] ShowTradeRoute: 贸易路线点不足"));
		return;
	}

	// 转换为世界坐标
	TArray<FVector> PathPoints;
	for (const FIntVector2& GridCoord : TradePath)
	{
		PathPoints.Add(GameMode->GridCoordToWorld(GridCoord));
	}

	// 注意：GetTradeRoutePath 已经确保闭环，这里不需要重复检查

	// 直接生成完整的Spline（无动画）
	TradeRouteSpline->ClearSplinePoints(false);

	for (int32 i = 0; i < PathPoints.Num(); ++i)
	{
		TradeRouteSpline->AddSplinePoint(PathPoints[i], ESplineCoordinateSpace::World, false);
		TradeRouteSpline->SetSplinePointType(i, ESplinePointType::Linear, false);
	}

	TradeRouteSpline->UpdateSpline();
	TradeRouteSpline->SetVisibility(true);

	// 立即生成所有SplineMesh
	RebuildTradeRouteSplineMeshes(PathPoints);
}

void APathVisualizerActor01::ClearPathVisualization()
{
	// 清除贸易路线
	ClearTradeRoute();

	// 清除手动路径
	ClearManualPath();

	// 重置状态
	CurrentPathType = EPathType01::None;
	CurrentUnitID = -1;
	CurrentUnitType = EEntityType01::None;
	TrackedInstance = nullptr;
}

void APathVisualizerActor01::ClearTradeRoute()
{
	// 将贸易路线SplineMesh添加到延迟销毁列表（避免迭代器失效）
	PendingDestroyMeshes.Append(TradeRouteSplineMeshes);
	TradeRouteSplineMeshes.Empty();

	// 清空贸易路线Spline
	if (TradeRouteSpline)
	{
		TradeRouteSpline->ClearSplinePoints(false);
		TradeRouteSpline->UpdateSpline();
		TradeRouteSpline->SetVisibility(false);
	}

	// 如果当前显示的是贸易路线，重置路径类型
	if (CurrentPathType == EPathType01::TradeRoute)
	{
		CurrentPathType = EPathType01::None;
	}
}

void APathVisualizerActor01::ClearManualPath()
{
	// 清理手动路径动画状态
	bIsExtendingPath = false;
	ExtensionProgress = 0.0f;

	// 将手动路径SplineMesh添加到延迟销毁列表（避免迭代器失效）
	PendingDestroyMeshes.Append(ManualPathSplineMeshes);
	ManualPathSplineMeshes.Empty();

	// 清空手动路径Spline
	if (ManualPathSpline)
	{
		ManualPathSpline->ClearSplinePoints(false);
		ManualPathSpline->UpdateSpline();
		ManualPathSpline->SetVisibility(false);
	}

	// 清空路径点
	ManualPathPoints.Empty();

	// 如果当前显示的是手动路径，重置路径类型
	if (CurrentPathType == EPathType01::Manual)
	{
		CurrentPathType = EPathType01::None;
	}
}

void APathVisualizerActor01::UpdatePathStartPoint()
{
	// 只有手动路径且当前显示的是手动路径时才更新
	if (CurrentPathType != EPathType01::Manual || !ManualPathSpline || ManualPathPoints.IsEmpty())
	{
		return;
	}

	if (TrackedInstance)
	{
		FVector NewStart = TrackedInstance->GetActorLocation();
		ManualPathPoints[0] = NewStart;
		ManualPathSpline->SetLocationAtSplinePoint(0, NewStart, ESplineCoordinateSpace::World, false);
		ManualPathSpline->UpdateSpline();

		// 更新第一个SplineMesh
		if (ManualPathSplineMeshes.Num() > 0 && ManualPathSplineMeshes[0] && ManualPathSpline->GetNumberOfSplinePoints() >= 2)
		{
			const FTransform SplineTransform = ManualPathSpline->GetComponentTransform();
			FVector S, ST, E, ET;
			ManualPathSpline->GetLocationAndTangentAtSplinePoint(0, S, ST, ESplineCoordinateSpace::World);
			ManualPathSpline->GetLocationAndTangentAtSplinePoint(1, E, ET, ESplineCoordinateSpace::World);

			S = SplineTransform.InverseTransformPosition(S);
			ST = SplineTransform.InverseTransformVector(ST);
			E = SplineTransform.InverseTransformPosition(E);
			ET = SplineTransform.InverseTransformVector(ET);

			ManualPathSplineMeshes[0]->SetStartAndEnd(S, ST, E, ET);
		}
	}
}

void APathVisualizerActor01::InitializeManualPathDisplay(const TArray<FVector>& PathPoints, AMoveableEntity01* TrackedEntity, int32 StartIndex)
{
	TrackedInstance = TrackedEntity;

	// 初始化Spline（只有起点）
	ManualPathSpline->ClearSplinePoints(false);
	ManualPathSpline->AddSplinePoint(PathPoints[0], ESplineCoordinateSpace::World, false);
	ManualPathSpline->SetSplinePointType(0, ESplinePointType::Linear, false);
	ManualPathSpline->UpdateSpline();
	ManualPathSpline->SetVisibility(true);

	// 开始延伸动画
	bIsExtendingPath = true;
	ExtensionProgress = 0.0f;
	LastRealTime = GetWorld()->GetRealTimeSeconds();
}

void APathVisualizerActor01::UpdatePathExtension(float RealDeltaTime)
{
	if (!bIsExtendingPath || ManualPathPoints.IsEmpty() || !ManualPathSpline)
	{
		return;
	}

	float OldProgress = ExtensionProgress;
	ExtensionProgress += RealDeltaTime * PathExtensionSpeed;

	const int32 CurrentSegment = FMath::FloorToInt(ExtensionProgress);
	const float SegmentProgress = ExtensionProgress - CurrentSegment;
	const int32 LastValidSegment = ManualPathPoints.Num() - 1;

	if (CurrentSegment < LastValidSegment)
	{
		// 延伸头在两个路径点之间插值
		const FVector HeadPos = FMath::Lerp(
			ManualPathPoints[CurrentSegment],
			ManualPathPoints[CurrentSegment + 1],
			SegmentProgress
		);

		// 确保已固定的点都已添加
		while (ManualPathSpline->GetNumberOfSplinePoints() <= CurrentSegment)
		{
			const int32 Idx = ManualPathSpline->GetNumberOfSplinePoints();
			ManualPathSpline->AddSplinePoint(ManualPathPoints[Idx], ESplineCoordinateSpace::World, false);
			ManualPathSpline->SetSplinePointType(Idx, ESplinePointType::Linear, false);
		}

		// 更新或添加延伸头
		const int32 HeadIndex = CurrentSegment + 1;
		if (ManualPathSpline->GetNumberOfSplinePoints() <= HeadIndex)
		{
			ManualPathSpline->AddSplinePoint(HeadPos, ESplineCoordinateSpace::World, false);
			ManualPathSpline->SetSplinePointType(HeadIndex, ESplinePointType::Linear, false);
		}
		else
		{
			ManualPathSpline->SetLocationAtSplinePoint(HeadIndex, HeadPos, ESplineCoordinateSpace::World, false);
		}

		ManualPathSpline->UpdateSpline();
		RebuildManualPathSplineMeshes();
	}
	else
	{
		// 延伸完成，补全所有点
		while (ManualPathSpline->GetNumberOfSplinePoints() < ManualPathPoints.Num())
		{
			const int32 Idx = ManualPathSpline->GetNumberOfSplinePoints();
			ManualPathSpline->AddSplinePoint(ManualPathPoints[Idx], ESplineCoordinateSpace::World, false);
			ManualPathSpline->SetSplinePointType(Idx, ESplinePointType::Linear, false);
		}

		ManualPathSpline->UpdateSpline();
		RebuildManualPathSplineMeshes();

		bIsExtendingPath = false;
	}
}

void APathVisualizerActor01::TrimPassedWaypoints()
{
	if (ManualPathPoints.Num() < 2 || !ManualPathSpline)
	{
		return;
	}

	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return;
	}

	int32 CurrentDataIndex = LastPathIndex;
	bool bHasPath = false;

	if (CurrentUnitType == EEntityType01::Caravan)
	{
		if (const FCaravanData01* Data = PS->GetCaravanDataPtr(CurrentUnitID))
		{
			CurrentDataIndex = Data->CurrentPathIndex;
			bHasPath = !Data->MovementPath.IsEmpty();
		}
	}
	else if (CurrentUnitType == EEntityType01::Colonist)
	{
		if (const FColonistData01* Data = PS->GetColonistData(CurrentUnitID))
		{
			CurrentDataIndex = Data->CurrentPathIndex;
			bHasPath = !Data->MovementPath.IsEmpty();
		}
	}

	// 如果数据层已经没有路径了，说明单位已到达终点，清理所有剩余路径点
	if (!bHasPath && ManualPathPoints.Num() > 1)
	{
		UE_LOG(LogTemp, Log, TEXT("[PathVisualizer] 单位已到达终点，清理剩余路径点"));
		ClearManualPath();
		return;
	}

	const int32 PointsToRemove = CurrentDataIndex - LastPathIndex;
	if (PointsToRemove <= 0)
	{
		return;
	}

	for (int32 i = 0; i < PointsToRemove; ++i)
	{
		if (ManualPathSpline->GetNumberOfSplinePoints() > 1)
		{
			ManualPathSpline->RemoveSplinePoint(0, false);
		}
		if (ManualPathPoints.Num() > 1)
		{
			ManualPathPoints.RemoveAt(0);
		}
	}
	ManualPathSpline->UpdateSpline();
	RebuildManualPathSplineMeshes();

	LastPathIndex = CurrentDataIndex;
}

void APathVisualizerActor01::RebuildManualPathSplineMeshes()
{
	if (!ManualPathSpline || !PathArrowMesh)
	{
		return;
	}

	const int32 NumPoints = ManualPathSpline->GetNumberOfSplinePoints();

	if (NumPoints < 2)
	{
		return;
	}

	// 清理旧的SplineMesh
	for (USplineMeshComponent* Mesh : ManualPathSplineMeshes)
	{
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}
	ManualPathSplineMeshes.Empty();

	const FTransform SplineTransform = ManualPathSpline->GetComponentTransform();
	const int32 NumSegments = NumPoints - 1;

	for (int32 i = 0; i < NumSegments; ++i)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		if (!SplineMesh)
		{
			continue;
		}

		SplineMesh->SetStaticMesh(PathArrowMesh);
	
	// 使用材质实例确保运行时材质应用正确
	if (PathArrowMaterial && PathArrowMaterial->IsValidLowLevel())
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PathArrowMaterial, this);
		if (DynamicMaterial)
		{
			SplineMesh->SetMaterial(0, DynamicMaterial);
			// 强制刷新渲染状态
			SplineMesh->MarkRenderStateDirty();
		}
		else
		{
			SplineMesh->SetMaterial(0, PathArrowMaterial);
			// 强制刷新渲染状态
			SplineMesh->MarkRenderStateDirty();
		}
	}

	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SplineMesh->RegisterComponent();
	SplineMesh->AttachToComponent(ManualPathSpline, FAttachmentTransformRules::KeepRelativeTransform);

		FVector S, ST, E, ET;
		ManualPathSpline->GetLocationAndTangentAtSplinePoint(i, S, ST, ESplineCoordinateSpace::World);
		ManualPathSpline->GetLocationAndTangentAtSplinePoint(i + 1, E, ET, ESplineCoordinateSpace::World);

		// 转换到局部坐标
		S = SplineTransform.InverseTransformPosition(S);
		ST = SplineTransform.InverseTransformVector(ST);
		E = SplineTransform.InverseTransformPosition(E);
		ET = SplineTransform.InverseTransformVector(ET);

		SplineMesh->SetStartAndEnd(S, ST, E, ET);
		SplineMesh->SetVisibility(true);

		ManualPathSplineMeshes.Add(SplineMesh);
	}
}

void APathVisualizerActor01::RebuildTradeRouteSplineMeshes(const TArray<FVector>& PathPoints)
{
	if (!TradeRouteSpline || !TradeRouteMesh)
	{
		return;
	}

	// 清理旧的SplineMesh
	for (USplineMeshComponent* Mesh : TradeRouteSplineMeshes)
	{
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}
	TradeRouteSplineMeshes.Empty();

	const FTransform SplineTransform = TradeRouteSpline->GetComponentTransform();
	const int32 NumPoints = PathPoints.Num();

	if (NumPoints < 2)
	{
		return;
	}

	// 创建闭环的SplineMesh（最后一个点连回第一个点）
	for (int32 i = 0; i < NumPoints - 1; ++i)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		if (!SplineMesh)
		{
			continue;
		}

		SplineMesh->SetStaticMesh(TradeRouteMesh);
	
	// 使用材质实例确保运行时材质应用正确
	if (TradeRouteMaterial && TradeRouteMaterial->IsValidLowLevel())
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(TradeRouteMaterial, this);
		if (DynamicMaterial)
		{
			SplineMesh->SetMaterial(0, DynamicMaterial);
			// 强制刷新渲染状态
			SplineMesh->MarkRenderStateDirty();
		}
		else
		{
			SplineMesh->SetMaterial(0, TradeRouteMaterial);
			// 强制刷新渲染状态
			SplineMesh->MarkRenderStateDirty();
		}
	}

	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SplineMesh->RegisterComponent();
	SplineMesh->AttachToComponent(TradeRouteSpline, FAttachmentTransformRules::KeepRelativeTransform);

		FVector S, ST, E, ET;
		TradeRouteSpline->GetLocationAndTangentAtSplinePoint(i, S, ST, ESplineCoordinateSpace::World);
		TradeRouteSpline->GetLocationAndTangentAtSplinePoint(i + 1, E, ET, ESplineCoordinateSpace::World);

		// 转换到局部坐标
		S = SplineTransform.InverseTransformPosition(S);
		ST = SplineTransform.InverseTransformVector(ST);
		E = SplineTransform.InverseTransformPosition(E);
		ET = SplineTransform.InverseTransformVector(ET);

		SplineMesh->SetStartAndEnd(S, ST, E, ET);
		SplineMesh->SetVisibility(true);

		TradeRouteSplineMeshes.Add(SplineMesh);
	}
}
