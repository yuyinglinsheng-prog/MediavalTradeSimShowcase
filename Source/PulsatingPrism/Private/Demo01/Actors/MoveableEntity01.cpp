// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/TimeManager01.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_PC.h"

AMoveableEntity01::AMoveableEntity01()
{
	// 启用 Tick
	PrimaryActorTick.bCanEverTick = true;
	
}

void AMoveableEntity01::BeginPlay()
{
	Super::BeginPlay();
	VisualTargetCoord = GridCoord;
	bVisualArrived = true;
}

void AMoveableEntity01::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 从数据层读取移动状态并插值显示
	UpdateVisualPosition(DeltaTime);
	
}

// ===== 移动控制 =====

void AMoveableEntity01::SetMovementPath(const TArray<FIntVector2>& Path)
{
	// 如果正在移动，先停止
	if (bIsMoving)
	{
		StopMovement();
	}
	
	CurrentPath = Path;
	CurrentPathIndex = 0;
	MoveProgress = 0.0f;
	LastDataPathIndex = 0;  // 重置数据层路径索引追踪
	
	if (CurrentPath.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[可移动实体] %s 设置路径，共 %d 个格子"), 
			*EntityName, CurrentPath.Num());
		
		// 打印路径
		FString PathStr;
		for (const FIntVector2& Coord : CurrentPath)
		{
			PathStr += FString::Printf(TEXT("(%d,%d) "), Coord.X, Coord.Y);
		}
		UE_LOG(LogTemp, Log, TEXT("[可移动实体] 路径: %s"), *PathStr);
		
		// 同步写入数据层
		WritePathToData(Path);
	}
}

void AMoveableEntity01::StartMovement()
{
	if (CurrentPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[可移动实体] %s 无法开始移动：路径为空"), *EntityName);
		return;
	}
	
	bIsMoving = true;
	bIsMovementPaused = false;
	CurrentPathIndex = 0;
	MoveProgress = 0.0f;
	StartLocation = GetActorLocation();
	
	UE_LOG(LogTemp, Log, TEXT("[可移动实体] %s 开始移动"), *EntityName);

	// 同步写入数据层
	WriteStartToData();
}

void AMoveableEntity01::StopMovement()
{
	bIsMoving = false;
	bIsMovementPaused = false;
	CurrentPath.Empty();
	CurrentPathIndex = 0;
	MoveProgress = 0.0f;
	LastDataPathIndex = 0;
	VisualTargetCoord = GridCoord;
	bVisualArrived = true;
	
	UE_LOG(LogTemp, Log, TEXT("[可移动实体] %s 停止移动"), *EntityName);

	// 同步清除数据层
	WriteStopToData();
}

void AMoveableEntity01::PauseMovement()
{
	bIsMovementPaused = true;
	UE_LOG(LogTemp, Log, TEXT("[可移动实体] %s 暂停移动"), *EntityName);
}

void AMoveableEntity01::ResumeMovement()
{
	bIsMovementPaused = false;
	UE_LOG(LogTemp, Log, TEXT("[可移动实体] %s 恢复移动"), *EntityName);
}

void AMoveableEntity01::SetVisualMoveTarget(FIntVector2 NewTargetCoord)
{
	if (!bVisualArrived)
	{
		// FVector ForcedTargetPos = AdjustZPosition(GridCoordToWorld(VisualTargetCoord));  // 注释：Z轴调整应在蓝图中处理
		SetActorLocation(GridCoordToWorld(VisualTargetCoord));
		UE_LOG(LogTemp, Warning, TEXT("[可移动实体] %s 视觉层未追上时收到新目标，先跳到旧目标 (%d,%d) 再开始下一段"),
			*EntityName, VisualTargetCoord.X, VisualTargetCoord.Y);
	}

	VisualTargetCoord = NewTargetCoord;
	bVisualArrived = false;
	bIsMoving = true;

	UE_LOG(LogTemp, Log, TEXT("[可移动实体] %s 设置视觉目标格子 (%d,%d)"),
		*EntityName, VisualTargetCoord.X, VisualTargetCoord.Y);
}

// ===== 移动逻辑 =====

void AMoveableEntity01::UpdateVisualPosition(float DeltaTime)
{
	// 从数据层获取移动信息
	bool bDataIsMoving = false;
	FIntVector2 DataCurrentCoord;
	FIntVector2 DataNextCoord;
	float DataProgress = 0.0f;
	
	if (!GetMovementData(bDataIsMoving, DataCurrentCoord, DataNextCoord, DataProgress))
	{
		// 子类未实现GetMovementData，不更新位置
		return;
	}

	// Snap模式：跳过插值，XY直接跳到数据层当前格子，Z轴保持spawn时的值
	ADemo01_GM* SnapGM = GetDemo01GameMode();
	if (SnapGM && SnapGM->IsSnapMovementEnabled())
	{
		FVector SnapTarget = GridCoordToWorld(DataCurrentCoord);
		SetActorLocation(FVector(SnapTarget.X, SnapTarget.Y, GetActorLocation().Z));
		VisualTargetCoord = DataCurrentCoord;
		bVisualArrived = true;
		return;
	}

	if (bVisualArrived)
	{
		return;
	}

	// FVector TargetWorldPos = AdjustZPosition(GridCoordToWorld(VisualTargetCoord));  // 注释：Z轴调整应在蓝图中处理
	FVector TargetWorldPos = GridCoordToWorld(VisualTargetCoord);
	FVector CurrentPos = GetActorLocation();
	if (!CurrentPos.Equals(TargetWorldPos, VisualArrivalTolerance))
	{
		const float EffectiveVisualSpeed = CalculateVisualMoveSpeed(CurrentPos, TargetWorldPos);
		FVector NewPos = FMath::VInterpConstantTo(CurrentPos, TargetWorldPos, DeltaTime, EffectiveVisualSpeed);
		SetActorLocation(NewPos);
		
		// 添加朝向调整：让单位朝向移动方向
		FVector MoveDirection = (TargetWorldPos - CurrentPos).GetSafeNormal();
		if (!MoveDirection.IsNearlyZero())
		{
			// 计算目标朝向（只考虑水平方向的旋转）
			FRotator TargetRotation = MoveDirection.Rotation();
			TargetRotation.Pitch = 0.0f;  // 保持水平，不倾斜
			TargetRotation.Roll = 0.0f;   // 保持水平，不滚动
			
			// 平滑旋转到目标朝向
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
			SetActorRotation(NewRotation);
		}
	}

	const FVector UpdatedPos = GetActorLocation();
	const bool bPosArrived = UpdatedPos.Equals(TargetWorldPos, VisualArrivalTolerance);

	if (bPosArrived)
	{
		SetActorLocation(TargetWorldPos);
		bVisualArrived = true;
		bIsMoving = bDataIsMoving;
		UE_LOG(LogTemp, Verbose, TEXT("[可移动实体] %s 视觉层已到达数据层格子 (%d,%d)"),
			*EntityName, VisualTargetCoord.X, VisualTargetCoord.Y);
	}
	else if (!bDataIsMoving && VisualTargetCoord != DataCurrentCoord)
	{
		UE_LOG(LogTemp, Warning, TEXT("[可移动实体] %s 视觉目标与数据层最终坐标不一致: VisualTarget=(%d,%d) DataCurrent=(%d,%d) DataNext=(%d,%d) Progress=%.2f"),
			*EntityName,
			VisualTargetCoord.X, VisualTargetCoord.Y,
			DataCurrentCoord.X, DataCurrentCoord.Y,
			DataNextCoord.X, DataNextCoord.Y,
			DataProgress);
	}
}

float AMoveableEntity01::CalculateVisualMoveSpeed(const FVector& CurrentPos, const FVector& TargetPos) const
{
	const ADemo01_GM* GM = GetDemo01GameMode();
	const UTimeManager01* TM = GM ? GM->GetTimeManager() : nullptr;
	if (!TM)
	{
		return VisualMoveSpeed;
	}

	const float GameSpeed = FMath::Max(0.1f, TM->GetGameSpeed());
	const float SecondsPerDay = FMath::Max(0.1f, TM->GetSecondsPerDay());
	const float DesiredTravelSeconds = FMath::Max(0.05f, (SecondsPerDay / GameSpeed) * VisualTravelDayFraction);
	const float Distance = FVector::Dist2D(CurrentPos, TargetPos);
	const float CalibratedSpeed = Distance / DesiredTravelSeconds;
	return FMath::Max(VisualMoveSpeed, CalibratedSpeed);
}

void AMoveableEntity01::UpdateMovement(float DeltaTime)
{
	if (CurrentPath.IsEmpty() || CurrentPathIndex >= CurrentPath.Num())
	{
		return;
	}
	
	// 获取当前目标格子
	FIntVector2 TargetCoord = CurrentPath[CurrentPathIndex];
	
	// 转换为世界坐标
	TargetLocation = GridCoordToWorld(TargetCoord);
	
	// 插值移动（使用子类提供的移动速度）
	MoveProgress += DeltaTime * GetMoveSpeed();
	
	if (MoveProgress >= 1.0f)
	{
		// 到达当前格子
		// FVector AdjustedLocation = AdjustZPosition(TargetLocation);  // 注释：Z轴调整应在蓝图中处理
		SetActorLocation(TargetLocation);
		GridCoord = TargetCoord;
		// 到达新格子（UCM已处理坐标同步）
		
		// 移动到下一个格子
		CurrentPathIndex++;
		MoveProgress = 0.0f;
		StartLocation = TargetLocation;
		
		// 检查是否到达终点
		if (CurrentPathIndex >= CurrentPath.Num())
		{
			bIsMoving = false;
			OnArrived();
		}
	}
	else
	{
		// 平滑移动
		FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, MoveProgress);
		// FVector AdjustedLocation = AdjustZPosition(NewLocation);  // 注释：Z轴调整应在蓝图中处理
		SetActorLocation(NewLocation);
	}
}

void AMoveableEntity01::OnArrived()
{
	// 清空路径
	CurrentPath.Empty();
	CurrentPathIndex = 0;
	
	UE_LOG(LogTemp, Log, TEXT("[可移动实体] %s 到达目的地"), *EntityName);
	
	// 子类可以重写此方法添加特定逻辑
}

// ===== Z轴位置调整 =====

FVector AMoveableEntity01::AdjustZPosition(const FVector& WorldPosition) const
{
	FVector AdjustedPosition = WorldPosition;
	
	// 注释：模型可能会更换，Z轴调整应在蓝图中处理，此处直接返回原始位置
	// FVector TraceStart = FVector(WorldPosition.X, WorldPosition.Y, WorldPosition.Z + 1000.0f);
	// FVector TraceEnd = FVector(WorldPosition.X, WorldPosition.Y, WorldPosition.Z - 1000.0f);
	// 
	// FHitResult HitResult;
	// FCollisionQueryParams QueryParams;
	// QueryParams.AddIgnoredActor(this);
	// QueryParams.bTraceComplex = false;
	// 
	// // 使用WorldStatic通道检测地面
	// bool bHit = GetWorld()->LineTraceSingleByChannel(
	// 	HitResult,
	// 	TraceStart,
	// 	TraceEnd,
	// 	ECC_WorldStatic,
	// 	QueryParams
	// );
	// 
	// if (bHit)
	// {
	// 	// 找到地面，调整Z坐标，并添加一个小的偏移避免穿透
	// 	AdjustedPosition.Z = HitResult.Location.Z + 50.0f;  // 50单位的高度偏移
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[可移动实体] %s: 在位置 (%f, %f) 未找到地面"), 
	// 		*EntityName, WorldPosition.X, WorldPosition.Y);
	// }
	// 
	// return AdjustedPosition;
	
	// 直接返回原始位置，Z轴调整应在蓝图中处理
	return WorldPosition;
}

// ===== 数据层读取（根据UnitType dispatch） =====

float AMoveableEntity01::GetMoveSpeed() const
{
	ADemo01_GM* GM = GetDemo01GameMode();
	if (!GM) return 1.0f;
	APlayerController* PC = GM->GetWorld()->GetFirstPlayerController();
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;
	if (!PS) return 1.0f;

	if (UnitType == EEntityType01::Caravan)
	{
		if (const FCaravanData01* Data = PS->GetCaravanDataPtr(EntityID))
		{
			return Data->MoveSpeed;
		}
	}
	else if (UnitType == EEntityType01::Colonist)
	{
		if (FColonistData01* Data = PS->GetColonistData(EntityID))
		{
			return Data->MoveSpeed;
		}
	}
	return 1.0f;
}

bool AMoveableEntity01::GetMovementData(bool& bOutIsMoving, FIntVector2& OutCurrentCoord,
	FIntVector2& OutNextCoord, float& OutProgress) const
{
	ADemo01_GM* GM = GetDemo01GameMode();
	if (!GM) return false;
	APlayerController* PC = GM->GetWorld()->GetFirstPlayerController();
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;
	if (!PS) return false;

	if (UnitType == EEntityType01::Caravan)
	{
		FCaravanData01* Data = PS->GetCaravanDataPtr(EntityID);
		if (!Data) return false;

		bOutIsMoving = Data->bIsMoving && !Data->MovementPath.IsEmpty()
			&& Data->CurrentPathIndex < Data->MovementPath.Num();
		OutCurrentCoord = Data->GridCoord;
		if (bOutIsMoving)
		{
			OutNextCoord = Data->MovementPath[Data->CurrentPathIndex];
			float TimeToNextCell = (Data->MoveSpeed > 0.0f) ? (1.0f / Data->MoveSpeed) : 1.0f;
			OutProgress = FMath::Clamp(Data->AccumulatedTime / TimeToNextCell, 0.0f, 1.0f);
		}
		else
		{
			OutNextCoord = Data->GridCoord;
			OutProgress = 0.0f;
		}
		return true;
	}
	else if (UnitType == EEntityType01::Colonist)
	{
		FColonistData01* Data = PS->GetColonistData(EntityID);
		if (!Data) return false;

		bOutIsMoving = Data->bIsMoving && !Data->MovementPath.IsEmpty()
			&& Data->CurrentPathIndex < Data->MovementPath.Num();
		OutCurrentCoord = Data->GridCoord;
		if (bOutIsMoving)
		{
			OutNextCoord = Data->MovementPath[Data->CurrentPathIndex];
			float TimeToNextCell = (Data->MoveSpeed > 0.0f) ? (1.0f / Data->MoveSpeed) : 1.0f;
			OutProgress = FMath::Clamp(Data->AccumulatedTime / TimeToNextCell, 0.0f, 1.0f);
		}
		else
		{
			OutNextCoord = Data->GridCoord;
			OutProgress = 0.0f;
		}
		return true;
	}

	return false;
}

void AMoveableEntity01::OnClicked()
{
	Super::OnClicked();
	ADemo01_PC* PC = Cast<ADemo01_PC>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		PC->SelectMoveableUnit(EntityID, UnitType);
	}
}

// ===== 数据层写入（SetMovementPath / StartMovement / StopMovement） =====

void AMoveableEntity01::WritePathToData(const TArray<FIntVector2>& Path)
{
	ADemo01_GM* GM = GetDemo01GameMode();
	if (!GM) return;
	APlayerController* PC = GM->GetWorld()->GetFirstPlayerController();
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;
	if (!PS) return;

	if (UnitType == EEntityType01::Caravan)
	{
		if (FCaravanData01* Data = PS->GetCaravanDataPtr(EntityID))
		{
			Data->MovementPath = Path;
			Data->CurrentPathIndex = 0;
			Data->AccumulatedTime = 0.0f;
		}
	}
	else if (UnitType == EEntityType01::Colonist)
	{
		if (FColonistData01* Data = PS->GetColonistData(EntityID))
		{
			Data->MovementPath = Path;
			Data->CurrentPathIndex = 0;
			Data->AccumulatedTime = 0.0f;
		}
	}
}

void AMoveableEntity01::WriteStartToData()
{
	ADemo01_GM* GM = GetDemo01GameMode();
	if (!GM) return;
	APlayerController* PC = GM->GetWorld()->GetFirstPlayerController();
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;
	if (!PS) return;

	if (UnitType == EEntityType01::Caravan)
	{
		if (FCaravanData01* Data = PS->GetCaravanDataPtr(EntityID))
		{
			Data->State = ECaravanState01::Moving;
			Data->bIsMoving = true;
		}
	}
	else if (UnitType == EEntityType01::Colonist)
	{
		if (FColonistData01* Data = PS->GetColonistData(EntityID))
		{
			Data->State = EColonistState01::Moving;
			Data->bIsMoving = true;
		}
	}
}

void AMoveableEntity01::WriteStopToData()
{
	ADemo01_GM* GM = GetDemo01GameMode();
	if (!GM) return;
	APlayerController* PC = GM->GetWorld()->GetFirstPlayerController();
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;
	if (!PS) return;

	if (UnitType == EEntityType01::Caravan)
	{
		if (FCaravanData01* Data = PS->GetCaravanDataPtr(EntityID))
		{
			Data->State = ECaravanState01::Idle;
			Data->bIsMoving = false;
			Data->MovementPath.Empty();
			Data->CurrentPathIndex = 0;
			Data->AccumulatedTime = 0.0f;
		}
	}
	else if (UnitType == EEntityType01::Colonist)
	{
		if (FColonistData01* Data = PS->GetColonistData(EntityID))
		{
			Data->State = EColonistState01::Idle;
			Data->bIsMoving = false;
			Data->MovementPath.Empty();
			Data->CurrentPathIndex = 0;
			Data->AccumulatedTime = 0.0f;
		}
	}
}