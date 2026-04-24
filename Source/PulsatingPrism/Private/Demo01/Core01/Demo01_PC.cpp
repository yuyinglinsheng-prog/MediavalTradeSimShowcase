// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/TimeManager01.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Demo01/Actors/SelectableEntity01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/PathVisualizerActor01.h"
#include "Core/PP_PlayerPawn.h"
#include "Core/Input/PP_InputConfig.h"
#include "Demo01/Core01/PathfindingService01.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"

ADemo01_PC::ADemo01_PC()
{
	PrimaryActorTick.bCanEverTick = true;

	// 初始化变量
	SpeedMultiplier = 1.0f;
	CachedEnhancedInput = nullptr;
	
	// 初始化选择状态
	SelectedTownID = -1;
	SelectedMoveableUnitID = -1;
	SelectedMoveableUnitType = EEntityType01::None;
	
	// 设置默认输入模式适合战略游戏
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	// 设置鼠标光标
	DefaultMouseCursor = EMouseCursor::Default;
}

void ADemo01_PC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADemo01_PC::BeginPlay()
{
	Super::BeginPlay();
	
	// 获取 GameMode 引用
	GameMode = Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] 无法获取 Demo01_GM"));
	}
	
	// 确保鼠标光标始终显示（战略游戏需要）
	SetShowMouseCursor(true);
	
	// 输入模式 - 游戏和UI混合
	SetInputMode(FInputModeGameAndUI());
	
	// 确保在LocalController上设置输入
	if (IsLocalController())
	{
		SetupCameraInput();
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 初始化完成"));
}

void ADemo01_PC::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// 获取EnhancedInput组件
	CachedEnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	
	// 如果还没有CachedEnhancedInput，创建它
	if (!CachedEnhancedInput && InputComponent)
	{
		CachedEnhancedInput = NewObject<UEnhancedInputComponent>(this);
		InputComponent = CachedEnhancedInput;
	}
}

void ADemo01_PC::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	// 确保输入系统在拥有Pawn后正确设置
	if (IsLocalController())
	{
		SetupCameraInput();
	}
}

void ADemo01_PC::SetupCameraInput()
{
	// 获取本地玩家子系统
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] 无法获取 LocalPlayer"));
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] 无法获取 EnhancedInputLocalPlayerSubsystem"));
		return;
	}
	
	// 清理现有映射
	InputSubsystem->ClearAllMappings();
	
	// 应用输入配置
	if (InputConfig && InputConfig->DefaultMappingContext)
	{
		InputSubsystem->AddMappingContext(InputConfig->DefaultMappingContext, 0);
	}
	
	// 设置EnhancedInput绑定
	SetupEnhancedInputBindings();
}

void ADemo01_PC::SetupEnhancedInputBindings()
{
	if (!CachedEnhancedInput || !InputConfig)
	{
		return;
	}
	
	// 清理旧的绑定
	CachedEnhancedInput->ClearActionBindings();
	
	// ===== 绑定相机控制 =====
	
	// 保留旧的Axis2D绑定以兼容（如果配置了）
	if (InputConfig->CameraMoveAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraMoveAction, ETriggerEvent::Triggered, this, &ADemo01_PC::MoveCamera);
	}
	
	// 新的分离式Axis1D绑定
	if (InputConfig->CameraMoveForwardAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraMoveForwardAction, ETriggerEvent::Triggered, this, &ADemo01_PC::MoveCameraForward);
	}
	
	if (InputConfig->CameraMoveRightAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraMoveRightAction, ETriggerEvent::Triggered, this, &ADemo01_PC::MoveCameraRight);
	}
	
	if (InputConfig->CameraPanAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraPanAction, ETriggerEvent::Triggered, this, &ADemo01_PC::PanCamera);
	}
	
	if (InputConfig->CameraRotateAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraRotateAction, ETriggerEvent::Triggered, this, &ADemo01_PC::RotateCamera);
	}
	
	if (InputConfig->CameraLookAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraLookAction, ETriggerEvent::Triggered, this, &ADemo01_PC::LookCamera);
	}
	
	if (InputConfig->CameraZoomAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraZoomAction, ETriggerEvent::Triggered, this, &ADemo01_PC::ZoomCamera);
	}
	
	if (InputConfig->CameraAscendAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraAscendAction, ETriggerEvent::Triggered, this, &ADemo01_PC::AscendCamera);
	}
	
	if (InputConfig->CameraDescendAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraDescendAction, ETriggerEvent::Triggered, this, &ADemo01_PC::DescendCamera);
	}
	
	if (InputConfig->CameraFocusAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraFocusAction, ETriggerEvent::Triggered, this, &ADemo01_PC::FocusCamera);
	}
	
	if (InputConfig->CameraResetAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->CameraResetAction, ETriggerEvent::Triggered, this, &ADemo01_PC::ResetCamera);
	}
	
	if (InputConfig->SpeedBoostAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->SpeedBoostAction, ETriggerEvent::Triggered, this, &ADemo01_PC::SpeedBoost);
		CachedEnhancedInput->BindAction(InputConfig->SpeedBoostAction, ETriggerEvent::Completed, this, &ADemo01_PC::SpeedBoost);
	}
	
	if (InputConfig->SpeedReduceAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->SpeedReduceAction, ETriggerEvent::Triggered, this, &ADemo01_PC::SpeedReduce);
		CachedEnhancedInput->BindAction(InputConfig->SpeedReduceAction, ETriggerEvent::Completed, this, &ADemo01_PC::SpeedReduce);
	}
	
	// ===== 绑定 Demo01 特有的输入 =====
	
	if (InputConfig->LeftClickAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->LeftClickAction, ETriggerEvent::Triggered, this, &ADemo01_PC::OnLeftMouseClick);
	}
	
	if (InputConfig->RightClickAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->RightClickAction, ETriggerEvent::Triggered, this, &ADemo01_PC::OnRightMouseClick);
	}
	
	if (InputConfig->TogglePauseAction)
	{
		CachedEnhancedInput->BindAction(InputConfig->TogglePauseAction, ETriggerEvent::Triggered, this, &ADemo01_PC::OnTogglePause);
	}
}

// ===== 新的选中管理系统 =====

void ADemo01_PC::SelectTown(int32 TownID)
{
	if (TownID == SelectedTownID) 
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 城镇已选中: ID=%d"), TownID);
		return; // 已选中同一城镇
	}
	
	int32 OldTownID = SelectedTownID;
	
	// 取消之前选中的城镇高亮
	if (SelectedTownID != -1)
	{
		UpdateEntityHighlight(SelectedTownID, EEntityType01::Town, false);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 取消城镇选中: ID=%d"), SelectedTownID);
	}
	
	// 设置新选中的城镇
	SelectedTownID = TownID;
	
	// 设置新城镇高亮
	if (TownID != -1)
	{
		UpdateEntityHighlight(TownID, EEntityType01::Town, true);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 选中城镇: ID=%d"), TownID);
	}
	
	// 通知UIManager更新面板
	if (GameMode && GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->OnTownSelectionChanged(TownID, OldTownID);
	}
}

void ADemo01_PC::ClearTownSelection()
{
	if (SelectedTownID != -1)
	{
		SelectTown(-1);
	}
}

void ADemo01_PC::SelectMoveableUnit(int32 UnitID, EEntityType01 UnitType)
{
	if (UnitID == SelectedMoveableUnitID && UnitType == SelectedMoveableUnitType) 
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 可移动单位已选中: Type=%d, ID=%d"), (int32)UnitType, UnitID);
		return; // 已选中同一单位
	}
	
	int32 OldUnitID = SelectedMoveableUnitID;
	EEntityType01 OldUnitType = SelectedMoveableUnitType;
	
	// 取消之前选中的可移动单位高亮
	if (SelectedMoveableUnitID != -1)
	{
		UpdateEntityHighlight(SelectedMoveableUnitID, SelectedMoveableUnitType, false);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 取消可移动单位选中: Type=%d, ID=%d"), 
			(int32)SelectedMoveableUnitType, SelectedMoveableUnitID);
	}
	
	// 设置新选中的可移动单位
	SelectedMoveableUnitID = UnitID;
	SelectedMoveableUnitType = UnitType;
	
	// 设置新单位高亮
	if (UnitID != -1)
	{
		UpdateEntityHighlight(UnitID, UnitType, true);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 选中可移动单位: Type=%d, ID=%d"), (int32)UnitType, UnitID);
	}
	
	// 通知UIManager更新面板
	if (GameMode && GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->OnMoveableUnitSelectionChanged(UnitID, UnitType, OldUnitID, OldUnitType);
	}

	// 路径可视化 - 根据状态显示对应的路径
	if (UnitID != -1)
	{
		// 获取PlayerState
		ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (PS && UnitType == EEntityType01::Caravan)
		{
			FCaravanData01* CaravanData = PS->GetCaravanDataPtr(UnitID);
			if (CaravanData && CaravanData->bIsAutoTrading)
			{
				// 商队处于自动贸易状态 → 显示贸易路线
				ShowTradeRoute(UnitID);
			}
			else
			{
				// 商队手动控制 → 显示手动路径
				ShowManualPath(UnitID, UnitType);
			}
		}
		else
		{
			// 殖民者 → 显示手动路径
			ShowManualPath(UnitID, UnitType);
		}
	}
	else
	{
		ClearPathVisualization();
	}
}

void ADemo01_PC::ClearMoveableUnitSelection()
{
	if (SelectedMoveableUnitID != -1)
	{
		SelectMoveableUnit(-1, EEntityType01::None);
	}
}

// ===== 高亮管理 =====

void ADemo01_PC::UpdateEntityHighlight(int32 EntityID, EEntityType01 EntityType, bool bHighlight)
{
	if (EntityID == -1) return;
	
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS) return;
	
	ASelectableEntity01* Instance = nullptr;
	
	switch (EntityType)
	{
	case EEntityType01::Town:
		Instance = PS->FindTownInstance(EntityID);
		break;
	case EEntityType01::Caravan:
	case EEntityType01::Colonist:
		Instance = PS->FindMoveableEntityInstance(EntityID);
		break;
	default:
		break;
	}
	
	if (Instance)
	{
		Instance->SetSelected(bHighlight);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 更新实体高亮: Type=%d, ID=%d, Highlight=%s, Instance=%s"), 
			(int32)EntityType, EntityID, bHighlight ? TEXT("true") : TEXT("false"), *Instance->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] 无法更新高亮，实体实例不存在: Type=%d, ID=%d"), 
			(int32)EntityType, EntityID);
	}
}

// ===== 兼容性接口实现 =====

void ADemo01_PC::SelectEntity(FEntityReference01 EntityRef)
{
	if (!EntityRef.IsValid())
	{
		ClearSelection();
		return;
	}
	
	// 根据实体类型调用新的选择方法
	if (EntityRef.Type == EEntityType01::Town)
	{
		SelectTown(EntityRef.EntityID);
	}
	else if (EntityRef.Type == EEntityType01::Caravan || EntityRef.Type == EEntityType01::Colonist)
	{
		SelectMoveableUnit(EntityRef.EntityID, EntityRef.Type);
	}
	
	// 更新兼容性字段
	SelectedEntityRef = EntityRef;
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 兼容性选择实体: Type=%d, ID=%d"), 
		(int32)EntityRef.Type, EntityRef.EntityID);
}

FEntityReference01 ADemo01_PC::GetSelectedEntity() const
{
	// 优先返回可移动单位选择
	if (SelectedMoveableUnitID != -1)
	{
		return FEntityReference01(SelectedMoveableUnitType, SelectedMoveableUnitID);
	}
	
	// 如果没有可移动单位选择，返回城镇选择
	if (SelectedTownID != -1)
	{
		return FEntityReference01(EEntityType01::Town, SelectedTownID);
	}
	
	return FEntityReference01();
}

void ADemo01_PC::SelectEntityByActor(ASelectableEntity01* Entity)
{
	if (!Entity)
	{
		ClearSelection();
		return;
	}
	
	// 根据Actor类型调用新的选择方法
	if (Cast<ATownActor01>(Entity))
	{
		SelectTown(Entity->EntityID);
	}
	else if (AMoveableEntity01* Moveable = Cast<AMoveableEntity01>(Entity))
	{
		SelectMoveableUnit(Entity->EntityID, Moveable->UnitType);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] 未知的实体类型"));
		return;
	}
}

void ADemo01_PC::ClearSelection()
{
	// 清除所有选择
	ClearTownSelection();
	ClearMoveableUnitSelection();
	
	// 清除兼容性字段
	SelectedEntityRef = FEntityReference01();
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 清除所有选中"));
}

ASelectableEntity01* ADemo01_PC::GetSelectedEntityInstance() const
{
	// 优先返回可移动单位实例
	if (SelectedMoveableUnitID != -1)
	{
		ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (PS)
		{
			return PS->FindMoveableEntityInstance(SelectedMoveableUnitID);
		}
	}
	
	// 如果没有可移动单位选择，返回城镇实例
	if (SelectedTownID != -1)
	{
		ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (PS)
		{
			return PS->FindTownInstance(SelectedTownID);
		}
	}
	
	return nullptr;
}

// ===== 输入处理 =====

void ADemo01_PC::OnLeftMouseClick()
{
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] ========== 左键点击 =========="));
	
	FHitResult HitResult;
	AActor* HitActor = PerformRaycast(HitResult);
	
	if (HitActor)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 点击到Actor：%s"), *HitActor->GetName());
		
		ASelectableEntity01* Entity = Cast<ASelectableEntity01>(HitActor);
		if (Entity)
		{
			UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 识别为可选中实体：%s"), *Entity->EntityName);
			SelectEntityByActor(Entity);
			Entity->OnClicked();
		}
		else
		{
			// 点击到非可选中实体（如地面、装饰物等），不关闭面板
			UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 点击到非可选中实体，保持当前选择"));
			// 不再调用 ClearSelection()，保持面板打开
		}
	}
	else
	{
		// 未点击到任何物体（空白区域），不关闭面板
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 未点击到任何物体，保持当前选择"));
		// 不再调用 ClearSelection()，保持面板打开
	}
}

void ADemo01_PC::OnRightMouseClick()
{
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 右键点击"));
	
	// 检查是否有选中的可移动单位
	if (SelectedMoveableUnitID == -1)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 右键点击：未选中任何可移动单位"));
		return;
	}
	
	// 只处理可移动单位（商队和殖民者）
	if (SelectedMoveableUnitType != EEntityType01::Caravan && SelectedMoveableUnitType != EEntityType01::Colonist)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 右键点击：选中的不是可移动单位"));
		return;
	}
	
	// 获取PlayerState
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] 无法获取PlayerState"));
		return;
	}
	
	// 检查商队是否处于自动贸易模式，阻止手动控制
	if (SelectedMoveableUnitType == EEntityType01::Caravan)
	{
		FCaravanData01* CaravanData = PS->GetCaravanDataPtr(SelectedMoveableUnitID);
		if (CaravanData && CaravanData->bIsAutoTrading)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] 商队 %d 正在自动贸易中，无法手动控制"), SelectedMoveableUnitID);
			return; // 阻止手动移动
		}
	}
	
	// 执行射线检测
	FHitResult HitResult;
	AActor* HitActor = PerformRaycast(HitResult);
	
	// 如果射线未命中任何物体，不执行移动命令
	if (!HitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] 右键点击：射线未命中任何物体，取消移动命令"));
		return;
	}
	
	FIntVector2 TargetCoord;
	
	// 检查是否点击了城镇
	ATownActor01* Town = Cast<ATownActor01>(HitActor);
	if (Town)
	{
		TargetCoord = Town->GetGridCoord();
	}
	else
	{
		// 移动到点击的地面位置
		TargetCoord = GameMode->WorldToGridCoord(HitResult.Location);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 点击地面位置: %s -> 网格坐标: (%d, %d)"), 
			*HitResult.Location.ToString(), TargetCoord.X, TargetCoord.Y);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 最终目标坐标: (%d, %d)"), TargetCoord.X, TargetCoord.Y);

	const bool bAppendPath = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
	
	// 根据选中的单位类型处理移动命令
	if (SelectedMoveableUnitType == EEntityType01::Caravan)
	{
		HandleCaravanMoveCommand(PS, TargetCoord, bAppendPath);
	}
	else if (SelectedMoveableUnitType == EEntityType01::Colonist)
	{
		HandleColonistMoveCommand(PS, TargetCoord, bAppendPath);
	}

	// 路径已更新，刷新可视化（只刷新手动路径）
	if (!GameMode) return;
	
	APathVisualizerActor01* PathVisualizer = GameMode->GetPathVisualizer();
	if (PathVisualizer && SelectedMoveableUnitType != EEntityType01::None)
	{
		// 如果当前显示的是手动路径，刷新它
		if (PathVisualizer->GetCurrentPathType() == EPathType01::Manual)
		{
			PathVisualizer->ShowManualPath(SelectedMoveableUnitID, SelectedMoveableUnitType);
		}
	}
}

void ADemo01_PC::OnTogglePause()
{
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 切换暂停输入"));
	if (GameMode)
	{
		UTimeManager01* TimeManager = GameMode->GetTimeManager();
		if (TimeManager)
		{
			TimeManager->TogglePause();
		}
	}
}

AActor* ADemo01_PC::PerformRaycast(FHitResult& OutHit)
{
	FVector WorldLocation, WorldDirection;
	if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + WorldDirection * 100000.0f;
		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetPawn());
		QueryParams.bTraceComplex = false;  // 使用简单碰撞
		QueryParams.bReturnPhysicalMaterial = false;
		
		if (bDebugDrawRaycast)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] [DEBUG] 射线检测：起点 %s，方向 %s"), 
				*Start.ToString(), *WorldDirection.ToString());
		}
		
		bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);
		
		// Debug绘制射线
		if (bDebugDrawRaycast)
		{
			if (bHit)
			{
				AActor* HitActor = OutHit.GetActor();
				ASelectableEntity01* SelectableEntity = Cast<ASelectableEntity01>(HitActor);
				
				if (SelectableEntity)
				{
					// 命中可选中实体（城镇或商队）：绿色射线和球体
					DrawDebugLine(GetWorld(), Start, OutHit.Location, FColor::Green, false, DebugRaycastDuration, 0, 2.0f);
					DrawDebugSphere(GetWorld(), OutHit.Location, 20.0f, 8, FColor::Green, false, DebugRaycastDuration, 0, 2.0f);
					UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] [DEBUG] 射线命中可选中实体：%s, 位置=%s, 距离=%.1f"), 
						*SelectableEntity->EntityName,
						*OutHit.Location.ToString(),
						OutHit.Distance);
				}
				else
				{
					// 命中其他物体（如地面）：黄色射线和球体
					DrawDebugLine(GetWorld(), Start, OutHit.Location, FColor::Yellow, false, DebugRaycastDuration, 0, 2.0f);
					DrawDebugSphere(GetWorld(), OutHit.Location, 20.0f, 8, FColor::Yellow, false, DebugRaycastDuration, 0, 2.0f);
					UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] [DEBUG] 射线命中其他物体：%s, 位置=%s, 距离=%.1f"), 
						HitActor ? *HitActor->GetName() : TEXT("None"),
						*OutHit.Location.ToString(),
						OutHit.Distance);
				}
			}
			else
			{
				// 未命中：红色射线
				DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, DebugRaycastDuration, 0, 1.0f);
				UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] [DEBUG] 射线未命中任何物体"));
			}
		}
		
		if (bHit)
		{
			return OutHit.GetActor();
		}
	}
	else
	{
		if (bDebugDrawRaycast)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Demo01_PC] [DEBUG] 射线检测失败：无法获取鼠标世界坐标"));
		}
	}
	
	return nullptr;
}

// ===== 相机控制 =====

void ADemo01_PC::MoveCamera(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn)
	{
		CameraPawn->MoveCamera(Value.Get<FVector2D>());
	}
}

void ADemo01_PC::MoveCameraForward(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn)
	{
		CameraPawn->MoveCameraForward(Value.Get<float>());
	}
}

void ADemo01_PC::MoveCameraRight(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn)
	{
		CameraPawn->MoveCameraRight(Value.Get<float>());
	}
}

void ADemo01_PC::PanCamera(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn) CameraPawn->PanCamera(Value.Get<FVector2D>());
}

void ADemo01_PC::RotateCamera(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn) CameraPawn->RotateCamera(Value.Get<float>());
}

void ADemo01_PC::LookCamera(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn) CameraPawn->LookCamera(Value.Get<FVector2D>());
}

void ADemo01_PC::ZoomCamera(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn) CameraPawn->ZoomCamera(Value.Get<float>());
}

void ADemo01_PC::AscendCamera(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn)
	{
		float AscendInput = Value.Get<float>();
		if (AscendInput > 0.0f) CameraPawn->AdjustHeight(AscendInput);
	}
}

void ADemo01_PC::DescendCamera(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn)
	{
		float DescendInput = Value.Get<float>();
		if (DescendInput < 0.0f) CameraPawn->AdjustHeight(DescendInput);
	}
}

void ADemo01_PC::FocusCamera()
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn && SelectedEntityRef.IsValid())
	{
		ASelectableEntity01* SelectedInstance = GetSelectedEntityInstance();
		if (SelectedInstance)
		{
			CameraPawn->FocusOnTarget(SelectedInstance);
		}
	}
}

void ADemo01_PC::ResetCamera()
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn) CameraPawn->ResetCamera();
}

void ADemo01_PC::SpeedBoost(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn)
	{
		bool bBoostActive = Value.Get<bool>();
		CameraPawn->SetMovementSpeed(bBoostActive ? SpeedBoostMultiplier : 1.0f);
	}
}

void ADemo01_PC::SpeedReduce(const FInputActionValue& Value)
{
	APP_PlayerPawn* CameraPawn = GetCameraPawn();
	if (CameraPawn)
	{
		bool bReduceActive = Value.Get<bool>();
		CameraPawn->SetMovementSpeed(bReduceActive ? SpeedReduceMultiplier : 1.0f);
	}
}

// ===== 公共接口 =====

void ADemo01_PC::SetInputConfig(UPP_InputConfig* NewConfig)
{
	if (!NewConfig) return;
	
	InputConfig = NewConfig;
	
	if (IsLocalController())
	{
		SetupCameraInput();
	}
}

APP_PlayerPawn* ADemo01_PC::GetCameraPawn() const
{
	return Cast<APP_PlayerPawn>(GetPawn());
}

// ===== 移动命令处理 =====

void ADemo01_PC::HandleCaravanMoveCommand(ADemo01_PS* PS, FIntVector2 TargetCoord, bool bAppendPath)
{
	if (!PS || !GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] HandleCaravanMoveCommand: PS或GameMode为空"));
		return;
	}
	
	// 获取商队数据指针
	FCaravanData01* CaravanData = PS->GetCaravanDataPtr(SelectedMoveableUnitID);
	if (!CaravanData)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] 无法找到商队数据"));
		return;
	}
	
	AMoveableEntity01* CaravanInstance = PS->FindMoveableEntityInstance(SelectedMoveableUnitID);
	if (CaravanInstance)
	{
		GameMode->HandleMoveCommand(CaravanInstance, TargetCoord, bAppendPath);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 商队移动命令已发送"));
	}
	else
	{
		GameMode->HandleMoveCommandByData(SelectedMoveableUnitID, EEntityType01::Caravan, TargetCoord, bAppendPath);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 商队数据层移动命令已发送（无实例）"));
	}
}

void ADemo01_PC::HandleColonistMoveCommand(ADemo01_PS* PS, FIntVector2 TargetCoord, bool bAppendPath)
{
	if (!PS || !GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] HandleColonistMoveCommand: PS或GameMode为空"));
		return;
	}
	
	// 获取殖民者数据指针
	FColonistData01* ColonistData = PS->GetColonistData(SelectedMoveableUnitID);
	if (!ColonistData)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] 无法找到殖民者数据"));
		return;
	}
	
	AMoveableEntity01* ColonistInstance = PS->FindMoveableEntityInstance(SelectedMoveableUnitID);
	if (ColonistInstance)
	{
		GameMode->HandleMoveCommand(ColonistInstance, TargetCoord, bAppendPath);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 殖民者移动命令已发送"));
	}
	else
	{
		GameMode->HandleMoveCommandByData(SelectedMoveableUnitID, EEntityType01::Colonist, TargetCoord, bAppendPath);
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PC] 殖民者数据层移动命令已发送（无实例）"));
	}
	
	// 新增：立即更新殖民者信息面板的建城按钮状态
	if (GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->UpdateColonistFoundCityButtonState(SelectedMoveableUnitID);
	}
}

// ===== 路径可视化（委托给PathVisualizerActor） =====

void ADemo01_PC::ShowManualPath(int32 UnitID, EEntityType01 UnitType)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] ShowManualPath: GameMode为空"));
		return;
	}
	
	APathVisualizerActor01* PathVisualizer = GameMode->GetPathVisualizer();
	if (!PathVisualizer)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] ShowManualPath: PathVisualizer为空"));
		return;
	}
	
	PathVisualizer->ShowManualPath(UnitID, UnitType);
}

void ADemo01_PC::ShowTradeRoute(int32 CaravanID)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] ShowTradeRoute: GameMode为空"));
		return;
	}
	
	APathVisualizerActor01* PathVisualizer = GameMode->GetPathVisualizer();
	if (!PathVisualizer)
	{
		UE_LOG(LogTemp, Error, TEXT("[Demo01_PC] ShowTradeRoute: PathVisualizer为空"));
		return;
	}
	
	PathVisualizer->ShowTradeRoute(CaravanID);
}

void ADemo01_PC::ClearPathVisualization()
{
	if (!GameMode) return;
	
	APathVisualizerActor01* PathVisualizer = GameMode->GetPathVisualizer();
	if (PathVisualizer)
	{
		PathVisualizer->ClearPathVisualization();
	}
}