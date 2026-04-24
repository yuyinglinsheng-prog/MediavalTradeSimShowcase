// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/UnitControlManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "Demo01/Core01/PathfindingService01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"

namespace
{
bool BuildMergedPath(
	UPathfindingService01* PathfindingService,
	const FIntVector2& CurrentCoord,
	const TArray<FIntVector2>& MovementPath,
	int32 CurrentPathIndex,
	const FIntVector2& TargetCoord,
	bool bAppendPath,
	TArray<FIntVector2>& OutMergedPath)
{
	OutMergedPath.Empty();

	TArray<FIntVector2> RemainingPath;
	if (MovementPath.Num() > 0 && CurrentPathIndex >= 0 && CurrentPathIndex < MovementPath.Num())
	{
		for (int32 Idx = CurrentPathIndex; Idx < MovementPath.Num(); ++Idx)
		{
			RemainingPath.Add(MovementPath[Idx]);
		}
	}

	const FIntVector2 AppendStart = (bAppendPath && RemainingPath.Num() > 0)
		? RemainingPath.Last()
		: CurrentCoord;

	if (bAppendPath && RemainingPath.Num() > 0 && AppendStart == TargetCoord)
	{
		OutMergedPath = RemainingPath;
		return true;
	}

	TArray<FIntVector2> NewSegment = PathfindingService->FindPath(AppendStart, TargetCoord);
	if (NewSegment.IsEmpty())
	{
		return false;
	}

	if (bAppendPath && RemainingPath.Num() > 0)
	{
		OutMergedPath = RemainingPath;
		const int32 NewSegStartIdx = (NewSegment[0] == RemainingPath.Last()) ? 1 : 0;
		for (int32 Idx = NewSegStartIdx; Idx < NewSegment.Num(); ++Idx)
		{
			OutMergedPath.Add(NewSegment[Idx]);
		}
	}
	else
	{
		OutMergedPath = NewSegment;
	}

	return true;
}
}

void UUnitControlManager01::Initialize(ADemo01_GM* InGameMode)
{
	GameMode = InGameMode;
	UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] 初始化完成"));
}

// ===== 每帧驱动 =====

void UUnitControlManager01::UpdateAllMovements(float DeltaDays)
{
	if (!GameMode)
	{
		return;
	}

	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	ADemo01_PS* PS = PC->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return;
	}

	// 更新所有商队的移动
	TArray<int32> CaravanIDs = PS->GetAllCaravanIDs();
	for (int32 CaravanID : CaravanIDs)
	{
		FCaravanData01* Caravan = PS->GetCaravanDataPtr(CaravanID);
		if (!Caravan || !Caravan->bIsMoving || Caravan->MovementPath.IsEmpty())
		{
			continue;
		}

		Caravan->AccumulatedTime += DeltaDays;

		float TimeToNextCell = 1.0f / Caravan->MoveSpeed;

		while (Caravan->AccumulatedTime >= TimeToNextCell && Caravan->CurrentPathIndex < Caravan->MovementPath.Num())
		{
			FIntVector2 OldCoord = Caravan->GridCoord;

			Caravan->GridCoord = Caravan->MovementPath[Caravan->CurrentPathIndex];
			Caravan->CurrentPathIndex++;
			Caravan->AccumulatedTime -= TimeToNextCell;

			UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 商队 %s 到达格子 (%d, %d)"),
				*Caravan->EntityName, Caravan->GridCoord.X, Caravan->GridCoord.Y);

			// 从城镇离开：创建实体实例
			if (!Caravan->Instance && Caravan->State == ECaravanState01::InTown)
			{
				Caravan->State = ECaravanState01::Moving;
				GameMode->SpawnCaravanInstanceAtCoord(CaravanID, OldCoord);
				UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 商队 %s 离开城镇，创建实例于旧坐标 (%d, %d)，首个视觉目标为 (%d, %d)"),
					*Caravan->EntityName, OldCoord.X, OldCoord.Y, Caravan->GridCoord.X, Caravan->GridCoord.Y);
				Caravan->CurrentTownID = -1;
			}

			if (Caravan->Instance)
			{
				Caravan->Instance->InitializeGridCoord(Caravan->GridCoord);
				Caravan->Instance->SetVisualMoveTarget(Caravan->GridCoord);
			}

			// 到达终点
			if (Caravan->CurrentPathIndex >= Caravan->MovementPath.Num())
			{
				Caravan->bIsMoving = false;
				Caravan->MovementPath.Empty();
				Caravan->CurrentPathIndex = 0;
				Caravan->AccumulatedTime = 0.0f;

// 检查目标格子是否有城镇
			ATownActor01* ArrivalTown = GameMode->GetTownAtCoord(Caravan->GridCoord);
			if (ArrivalTown)
			{
				Caravan->State = ECaravanState01::InTown;
				Caravan->CurrentTownID = ArrivalTown->EntityID;
				ArrivalTown->AddCaravanByID(CaravanID);
				UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 商队 %s 进入城镇 %s"),
					*Caravan->EntityName, *ArrivalTown->EntityName);
				
				// 同步贸易路线索引（如果处于自动贸易模式）
				if (Caravan->bIsAutoTrading && Caravan->TradeRoute.bIsActive)
				{
					int32 TownIndex = Caravan->TradeRoute.TownVisitOrder.Find(ArrivalTown->EntityID);
					if (TownIndex != INDEX_NONE)
					{
						Caravan->TradeRoute.CurrentVisitIndex = TownIndex;
						UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 商队 %s 同步贸易路线索引: %d"),
							*Caravan->EntityName, TownIndex);
					}
				}
				
				if (AMoveableEntity01* Inst = Caravan->Instance)
				{
					Caravan->Instance = nullptr;
					Inst->Destroy();
				}
				if (GameMode->GetUIManager())
				{
					GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(ArrivalTown);
					GameMode->GetUIManager()->NotifyCaravanStateChanged(CaravanID);
				}
			}
				else
				{
					Caravan->State = ECaravanState01::Arrived;
					UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 商队 %s 已到达终点"), *Caravan->EntityName);
					if (GameMode->GetUIManager())
					{
						GameMode->GetUIManager()->NotifyCaravanStateChanged(CaravanID);
					}
				}
				break;
			}
		}
	}

	// 更新所有殖民者的移动
	TArray<int32> ColonistIDs = PS->GetAllColonistIDs();
	for (int32 ColonistID : ColonistIDs)
	{
		FColonistData01* Colonist = PS->GetColonistData(ColonistID);
		if (!Colonist || !Colonist->bIsMoving || Colonist->MovementPath.IsEmpty())
		{
			continue;
		}

		Colonist->AccumulatedTime += DeltaDays;

		float TimeToNextCell = 1.0f / Colonist->MoveSpeed;

		while (Colonist->AccumulatedTime >= TimeToNextCell && Colonist->CurrentPathIndex < Colonist->MovementPath.Num())
		{
			FIntVector2 OldCoord = Colonist->GridCoord;

			Colonist->GridCoord = Colonist->MovementPath[Colonist->CurrentPathIndex];
			Colonist->CurrentPathIndex++;
			Colonist->AccumulatedTime -= TimeToNextCell;

			UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 殖民者 %s 到达格子 (%d, %d)"),
				*Colonist->EntityName, Colonist->GridCoord.X, Colonist->GridCoord.Y);

			// 从城镇离开：创建实体实例
			if (!Colonist->Instance && Colonist->State == EColonistState01::InTown)
			{
				Colonist->State = EColonistState01::Moving;
				GameMode->SpawnColonistInstanceAtCoord(ColonistID, OldCoord);
				UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 殖民者 %s 离开城镇，创建实例于旧坐标 (%d, %d)，首个视觉目标为 (%d, %d)"),
					*Colonist->EntityName, OldCoord.X, OldCoord.Y, Colonist->GridCoord.X, Colonist->GridCoord.Y);
				Colonist->CurrentTownID = -1;
			}

			if (Colonist->Instance)
			{
				Colonist->Instance->InitializeGridCoord(Colonist->GridCoord);
				Colonist->Instance->SetVisualMoveTarget(Colonist->GridCoord);
			}

			// 到达终点
			if (Colonist->CurrentPathIndex >= Colonist->MovementPath.Num())
			{
				Colonist->bIsMoving = false;
				Colonist->MovementPath.Empty();
				Colonist->CurrentPathIndex = 0;
				Colonist->AccumulatedTime = 0.0f;

				// 检查建城命令
				if (Colonist->bHasPendingFoundCityCommand
					&& Colonist->GridCoord == Colonist->FoundCityTargetCoord)
				{
					Colonist->bHasPendingFoundCityCommand = false;
					UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 殖民者 %s 开始建城"), *Colonist->EntityName);
					GameMode->CreateCityFromColonist(ColonistID);
				}
				else
				{
					// 检查目标格子是否有城镇
					ATownActor01* ArrivalTown = GameMode->GetTownAtCoord(Colonist->GridCoord);
					if (ArrivalTown)
					{
						Colonist->State = EColonistState01::InTown;
						Colonist->CurrentTownID = ArrivalTown->EntityID;
						ArrivalTown->AddColonistByID(ColonistID);
						UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 殖民者 %s 进入城镇 %s"),
							*Colonist->EntityName, *ArrivalTown->EntityName);
						if (AMoveableEntity01* Inst = Colonist->Instance)
						{
							Colonist->Instance = nullptr;
							Inst->Destroy();
						}
						if (GameMode->GetUIManager())
						{
							GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(ArrivalTown);
							GameMode->GetUIManager()->UpdateCurrentColonistIfOpen(nullptr);
						}
					}
					else
					{
						Colonist->State = EColonistState01::Arrived;
						UE_LOG(LogTemp, Log, TEXT("[UnitControlManager] 殖民者 %s 已到达终点"), *Colonist->EntityName);
					}
				}
				break;
			}
		}
	}
}

// ===== 移动命令处理 =====

void UUnitControlManager01::HandleMoveCommand(AMoveableEntity01* Entity, FIntVector2 TargetCoord, bool bAppendPath)
{
	if (!Entity)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] 移动命令失败：实体为空"));
		return;
	}

	HandleMoveCommandByData(Entity->EntityID, Entity->UnitType, TargetCoord, bAppendPath);
}

void UUnitControlManager01::HandleMoveCommandByData(int32 UnitID, EEntityType01 UnitType, FIntVector2 TargetCoord, bool bAppendPath)
{
	if (!GameMode || !GameMode->GetPathfindingService())
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] 移动命令失败：寻路服务未初始化"));
		return;
	}

	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	ADemo01_PS* PS = PC ? PC->GetPlayerState<ADemo01_PS>() : nullptr;
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] 移动命令失败：无法获取PlayerState"));
		return;
	}

	const TCHAR* ModeText = bAppendPath ? TEXT("追加") : TEXT("替换");

	if (UnitType == EEntityType01::Caravan)
	{
		FCaravanData01* Data = PS->GetCaravanDataPtr(UnitID);
		if (!Data)
		{
			UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] 商队移动命令失败：未找到数据 ID=%d"), UnitID);
			return;
		}

		TArray<FIntVector2> MergedPath;
		if (!BuildMergedPath(
			GameMode->GetPathfindingService(),
			Data->GridCoord,
			Data->MovementPath,
			Data->CurrentPathIndex,
			TargetCoord,
			bAppendPath,
			MergedPath))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UnitControlManager01] 商队路径%s失败：未找到从当前起点到目标(%d,%d)的路径，保留原路线"),
				ModeText,
				TargetCoord.X,
				TargetCoord.Y);
			return;
		}

		Data->MovementPath = MergedPath;
		Data->CurrentPathIndex = 0;
		Data->AccumulatedTime = 1.0f;

		if (!Data->MovementPath.IsEmpty())
		{
			Data->bIsMoving = true;
			if (!(Data->State == ECaravanState01::InTown && Data->Instance == nullptr))
			{
				Data->State = ECaravanState01::Moving;
				Data->CurrentTownID = -1;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] 商队路径%s成功：ID=%d 目标=(%d,%d) 剩余路径长度=%d"),
			ModeText,
			UnitID,
			TargetCoord.X,
			TargetCoord.Y,
			Data->MovementPath.Num());

		if (GameMode->GetUIManager())
		{
			GameMode->GetUIManager()->RefreshCaravanInfoPanelIfOpen(UnitID);
			GameMode->GetUIManager()->NotifyCaravanStateChanged(UnitID);
		}

		return;
	}

	if (UnitType == EEntityType01::Colonist)
	{
		FColonistData01* Data = PS->GetColonistData(UnitID);
		if (!Data)
		{
			UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] 殖民者移动命令失败：未找到数据 ID=%d"), UnitID);
			return;
		}

		TArray<FIntVector2> MergedPath;
		if (!BuildMergedPath(
			GameMode->GetPathfindingService(),
			Data->GridCoord,
			Data->MovementPath,
			Data->CurrentPathIndex,
			TargetCoord,
			bAppendPath,
			MergedPath))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UnitControlManager01] 殖民者路径%s失败：未找到从当前起点到目标(%d,%d)的路径，保留原路线"),
				ModeText,
				TargetCoord.X,
				TargetCoord.Y);
			return;
		}

		Data->MovementPath = MergedPath;
		Data->CurrentPathIndex = 0;
		Data->AccumulatedTime = 1.0f;

		if (!Data->MovementPath.IsEmpty())
		{
			Data->bIsMoving = true;
			if (!(Data->State == EColonistState01::InTown && Data->Instance == nullptr))
			{
				Data->State = EColonistState01::Moving;
				Data->CurrentTownID = -1;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] 殖民者路径%s成功：ID=%d 目标=(%d,%d) 剩余路径长度=%d"),
			ModeText,
			UnitID,
			TargetCoord.X,
			TargetCoord.Y,
			Data->MovementPath.Num());

		if (GameMode->GetUIManager())
		{
			GameMode->GetUIManager()->RefreshColonistInfoPanelIfOpen(UnitID);
		}

		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UnitControlManager01] 移动命令失败：不支持的单位类型 %d"), (int32)UnitType);
}

// ===== 商队实例管理 =====

AMoveableEntity01* UUnitControlManager01::SpawnCaravanFromTown(int32 CaravanID, ATownActor01* Town)
{
	if (!Town)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanFromTown: 城镇为空"));
		return nullptr;
	}
	
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanFromTown: GameMode为空"));
		return nullptr;
	}
	
	// 获取PlayerState
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanFromTown: 无法获取PlayerState"));
		return nullptr;
	}
	
	// 获取商队数据
	FCaravanData01 CaravanData = PS->GetCaravanData(CaravanID);
	if (CaravanData.EntityID != CaravanID)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanFromTown: 未找到商队数据 ID: %d"), CaravanID);
		return nullptr;
	}
	
	// 检查是否已有实例
	if (CaravanData.Instance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UnitControlManager01] SpawnCaravanFromTown: 商队已有实例 ID: %d"), CaravanID);
		return CaravanData.Instance;
	}
	
	// 获取商队蓝图类
	if (!GameMode->GetTestCaravanClass())
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanFromTown: TestCaravanClass 未设置"));
		return nullptr;
	}
	
	// Snap模式下在城镇格子坐标spawn，正常模式在城镇Actor位置spawn
	FVector SpawnLocation = (GameMode->IsSnapMovementEnabled())
		? GameMode->GridCoordToWorld(CaravanData.GridCoord)
		: Town->GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AMoveableEntity01* NewCaravan = GameMode->GetWorld()->SpawnActor<AMoveableEntity01>(
		GameMode->GetTestCaravanClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewCaravan)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanFromTown: Spawn失败"));
		return nullptr;
	}

	// 调整Z轴
	FVector Origin, Extent;
	NewCaravan->GetActorBounds(false, Origin, Extent);
	float BottomZ = Origin.Z - Extent.Z;
	FVector AdjustedLocation = SpawnLocation;
	AdjustedLocation.Z += -BottomZ;
	NewCaravan->SetActorLocation(AdjustedLocation);

	// 设置实体数据
	NewCaravan->EntityID = CaravanID;
	NewCaravan->EntityName = CaravanData.EntityName;
	NewCaravan->UnitType = EEntityType01::Caravan;

	PS->SetCaravanInstance(CaravanID, NewCaravan);
	if (FCaravanData01* LiveData = PS->GetCaravanDataPtr(CaravanID))
	{
		LiveData->State = ECaravanState01::Idle;
		LiveData->CurrentTownID = -1;
		LiveData->Instance = NewCaravan;
	}

	Town->RemoveCaravanByID(CaravanID);
	NewCaravan->SetSelected(true);

	UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] SpawnCaravanFromTown: 成功spawn商队 %s (ID: %d) 从城镇 %s"),
		*NewCaravan->EntityName, CaravanID, *Town->TownData.EntityName);

	if (GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
		GameMode->GetUIManager()->NotifyCaravanStateChanged(CaravanID);
	}

	return NewCaravan;
}

AMoveableEntity01* UUnitControlManager01::SpawnCaravanInstanceAtCoord(int32 CaravanID, FIntVector2 GridCoord)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanInstanceAtCoord: GameMode为空"));
		return nullptr;
	}
	
	// 获取PlayerState
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanInstanceAtCoord: 无法获取PlayerState"));
		return nullptr;
	}
	
	// 获取商队数据
	FCaravanData01* CaravanData = PS->GetCaravanDataPtr(CaravanID);
	if (!CaravanData)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanInstanceAtCoord: 未找到商队数据 ID: %d"), CaravanID);
		return nullptr;
	}
	
	// 检查是否已有实例
	if (CaravanData->Instance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UnitControlManager01] SpawnCaravanInstanceAtCoord: 商队已有实例 ID: %d"), CaravanID);
		return CaravanData->Instance;
	}
	
	// 获取商队蓝图类
	if (!GameMode->GetTestCaravanClass())
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanInstanceAtCoord: TestCaravanClass 未设置"));
		return nullptr;
	}
	
	// 计算Spawn位置
	FVector SpawnLocation = GameMode->GridCoordToWorld(GridCoord);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AMoveableEntity01* NewCaravan = GameMode->GetWorld()->SpawnActor<AMoveableEntity01>(
		GameMode->GetTestCaravanClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewCaravan)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnCaravanInstanceAtCoord: Spawn失败"));
		return nullptr;
	}

	// 调整Z轴
	FVector Origin, Extent;
	NewCaravan->GetActorBounds(false, Origin, Extent);
	float BottomZ = Origin.Z - Extent.Z;
	FVector AdjustedLocation = SpawnLocation;
	AdjustedLocation.Z += -BottomZ;
	NewCaravan->SetActorLocation(AdjustedLocation);

	// 设置商队数据
	NewCaravan->EntityID = CaravanID;
	NewCaravan->EntityName = CaravanData->EntityName;
	NewCaravan->UnitType = EEntityType01::Caravan;
	NewCaravan->SetGridCoord(GridCoord);

	// 更新PS数据
	PS->SetCaravanInstance(CaravanID, NewCaravan);
	CaravanData->Instance = NewCaravan;
	CaravanData->GridCoord = GridCoord;
	
	// 从城镇移除Entry
	ATownActor01* DepartureTown = nullptr;
	if (CaravanData->CurrentTownID != -1)
	{
		DepartureTown = PS->FindTownInstance(CaravanData->CurrentTownID);
		if (DepartureTown)
		{
			DepartureTown->RemoveCaravanByID(CaravanID);
			UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] 商队 %s 已从城镇 %s 的UI列表中移除"), 
				*CaravanData->EntityName, *DepartureTown->EntityName);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] SpawnCaravanInstanceAtCoord: 成功spawn商队 %s (ID: %d) 于坐标 (%d, %d)"), 
		*NewCaravan->EntityName, CaravanID, GridCoord.X, GridCoord.Y);
	
	// 恢复选中状态
	ADemo01_PC* PC = Cast<ADemo01_PC>(GameMode->GetWorld()->GetFirstPlayerController());
	if (PC && PC->GetSelectedMoveableUnitType() == EEntityType01::Caravan && PC->GetSelectedMoveableUnitID() == CaravanID)
	{
		NewCaravan->SetSelected(true);
		UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] 商队 %s 创建时恢复选中状态"), *CaravanData->EntityName);
	}
	
	// 通知UI
	if (GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->NotifyCaravanStateChanged(CaravanID);
		
		// 刷新城镇UI面板（立即更新Entry列表）
		if (DepartureTown)
		{
			GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(DepartureTown);
		}
	}
	
	return NewCaravan;
}

// ===== 殖民者实例管理 =====

AMoveableEntity01* UUnitControlManager01::SpawnColonistFromTown(int32 ColonistID, ATownActor01* Town)
{
	if (!Town)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistFromTown: 城镇为空"));
		return nullptr;
	}

	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistFromTown: GameMode为空"));
		return nullptr;
	}

	// 获取PlayerState
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistFromTown: 无法获取PlayerState"));
		return nullptr;
	}

	// 获取殖民者数据
	FColonistData01* ColonistDataPtr = PS->GetColonistData(ColonistID);
	if (!ColonistDataPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistFromTown: 未找到殖民者数据 ID: %d"), ColonistID);
		return nullptr;
	}

	FColonistData01 ColonistData = *ColonistDataPtr;

	// 检查是否已有实例
	if (ColonistData.Instance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UnitControlManager01] SpawnColonistFromTown: 殖民者已有实例 ID: %d"), ColonistID);
		return ColonistData.Instance;
	}

	// 获取殖民者蓝图类
	if (!GameMode->GetTestColonistClass())
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistFromTown: TestColonistClass 未设置"));
		return nullptr;
	}

	// Snap模式下在城镇格子坐标spawn，正常模式在城镇Actor位置spawn
	FVector SpawnLocation = (GameMode->IsSnapMovementEnabled())
		? GameMode->GridCoordToWorld(ColonistData.GridCoord)
		: Town->GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMoveableEntity01* NewColonist = GameMode->GetWorld()->SpawnActor<AMoveableEntity01>(
		GameMode->GetTestColonistClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewColonist)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistFromTown: Spawn失败"));
		return nullptr;
	}

	// 调整Z轴
	FVector Origin, Extent;
	NewColonist->GetActorBounds(false, Origin, Extent);
	float BottomZ = Origin.Z - Extent.Z;
	FVector AdjustedLocation = SpawnLocation;
	AdjustedLocation.Z += -BottomZ;
	NewColonist->SetActorLocation(AdjustedLocation);

	// 设置实体数据
	NewColonist->EntityID = ColonistID;
	NewColonist->EntityName = ColonistData.EntityName;
	NewColonist->UnitType = EEntityType01::Colonist;

	PS->SetColonistInstance(ColonistID, NewColonist);
	if (FColonistData01* LiveData = PS->GetColonistData(ColonistID))
	{
		LiveData->State = EColonistState01::Idle;
		LiveData->CurrentTownID = -1;
		LiveData->Instance = NewColonist;
	}

	Town->RemoveColonistByID(ColonistID);
	NewColonist->SetSelected(true);

	UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] SpawnColonistFromTown: 成功spawn殖民者 %s (ID: %d) 从城镇 %s"),
		*NewColonist->EntityName, ColonistID, *Town->TownData.EntityName);

	if (GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
		GameMode->GetUIManager()->UpdateCurrentColonistIfOpen(NewColonist);
	}

	return NewColonist;
}

AMoveableEntity01* UUnitControlManager01::SpawnColonistInstanceAtCoord(int32 ColonistID, FIntVector2 GridCoord)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistInstanceAtCoord: GameMode为空"));
		return nullptr;
	}
	
	// 获取PlayerState
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistInstanceAtCoord: 无法获取PlayerState"));
		return nullptr;
	}
	
	// 获取殖民者数据
	FColonistData01* ColonistData = PS->GetColonistData(ColonistID);
	if (!ColonistData)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistInstanceAtCoord: 未找到殖民者数据 ID: %d"), ColonistID);
		return nullptr;
	}
	
	// 检查是否已有实例
	if (ColonistData->Instance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UnitControlManager01] SpawnColonistInstanceAtCoord: 殖民者已有实例 ID: %d"), ColonistID);
		return ColonistData->Instance;
	}
	
	// 获取殖民者蓝图类
	if (!GameMode->GetTestColonistClass())
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistInstanceAtCoord: TestColonistClass 未设置"));
		return nullptr;
	}
	
	// 计算Spawn位置
	FVector SpawnLocation = GameMode->GridCoordToWorld(GridCoord);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AMoveableEntity01* NewColonist = GameMode->GetWorld()->SpawnActor<AMoveableEntity01>(
		GameMode->GetTestColonistClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewColonist)
	{
		UE_LOG(LogTemp, Error, TEXT("[UnitControlManager01] SpawnColonistInstanceAtCoord: Spawn失败"));
		return nullptr;
	}

	// 调整Z轴
	FVector Origin, Extent;
	NewColonist->GetActorBounds(false, Origin, Extent);
	float BottomZ = Origin.Z - Extent.Z;
	FVector AdjustedLocation = SpawnLocation;
	AdjustedLocation.Z += -BottomZ;
	NewColonist->SetActorLocation(AdjustedLocation);

	// 设置实体数据
	NewColonist->EntityID = ColonistID;
	NewColonist->EntityName = ColonistData->EntityName;
	NewColonist->UnitType = EEntityType01::Colonist;
	NewColonist->SetGridCoord(GridCoord);

	// 更新PS数据
	PS->SetColonistInstance(ColonistID, NewColonist);
	ColonistData->Instance = NewColonist;
	ColonistData->GridCoord = GridCoord;
	
	// 从城镇移除Entry
	ATownActor01* DepartureTown = nullptr;
	if (ColonistData->CurrentTownID != -1)
	{
		DepartureTown = PS->FindTownInstance(ColonistData->CurrentTownID);
		if (DepartureTown)
		{
			DepartureTown->RemoveColonistByID(ColonistID);
			UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] 殖民者 %s 已从城镇 %s 的UI列表中移除"), 
				*ColonistData->EntityName, *DepartureTown->EntityName);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] SpawnColonistInstanceAtCoord: 成功spawn殖民者 %s (ID: %d) 于坐标 (%d, %d)"), 
		*NewColonist->EntityName, ColonistID, GridCoord.X, GridCoord.Y);
	
	// 恢复选中状态
	ADemo01_PC* PC = Cast<ADemo01_PC>(GameMode->GetWorld()->GetFirstPlayerController());
	if (PC && PC->GetSelectedMoveableUnitType() == EEntityType01::Colonist && PC->GetSelectedMoveableUnitID() == ColonistID)
	{
		NewColonist->SetSelected(true);
		UE_LOG(LogTemp, Log, TEXT("[UnitControlManager01] 殖民者 %s 创建时恢复选中状态"), *ColonistData->EntityName);
	}
	
	// 通知UI
	if (GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->UpdateCurrentColonistIfOpen(NewColonist);
		
		// 刷新城镇UI面板（立即更新Entry列表）
		if (DepartureTown)
		{
			GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(DepartureTown);
		}
	}
	
	return NewColonist;
}
