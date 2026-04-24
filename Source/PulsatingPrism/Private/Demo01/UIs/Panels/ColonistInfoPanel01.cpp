// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/ColonistInfoPanel01.h"
#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UColonistInfoPanel01::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮事件
	if (FoundCityButton)
	{
		FoundCityButton->OnClicked.AddDynamic(this, &UColonistInfoPanel01::OnFoundCityClicked);
	}

	if (DeleteColonistButton)
	{
		DeleteColonistButton->OnClicked.AddDynamic(this, &UColonistInfoPanel01::OnDeleteColonistClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UColonistInfoPanel01::OnCloseClicked);
	}
}

void UColonistInfoPanel01::InitializePanel(AMoveableEntity01* InColonist)
{
	if (!InColonist)
	{
		UE_LOG(LogTemp, Error, TEXT("[ColonistInfoPanel] InitializePanel: 殖民者为空"));
		return;
	}

	Colonist = InColonist;
	int32 ColonistID = Colonist->EntityID;
	
	// 统一通过PS获取数据，保持架构一致性
	if (GameMode)
	{
		UWorld* World = GameMode->GetWorld();
		if (World)
		{
			ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
			if (PS)
			{
				ColonistData = PS->GetColonistDataCopy(ColonistID);
				UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 殖民者面板初始化完成（通过PS数据）：%s (ID: %d)"), 
					*ColonistData.EntityName, ColonistID);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ColonistInfoPanel] PS不可用，无法获取殖民者数据"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ColonistInfoPanel] GameMode不可用，无法获取殖民者数据"));
	}
	
	// 更新显示
	UpdateColonistInfo();
	UpdateButtonStates();
}

void UColonistInfoPanel01::InitializePanelWithData(const FColonistData01& InColonistData)
{
	// 无实例版本，只使用数据
	Colonist = nullptr;
	ColonistData = InColonistData;
	
	UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 使用数据初始化（无实例）：%s"), *ColonistData.EntityName);
	
	// 更新显示
	UpdateColonistInfo();
	UpdateButtonStates();
}

void UColonistInfoPanel01::InitializeUI(ADemo01_GM* InGameMode)
{
	Super::InitializeUI(InGameMode);
}

void UColonistInfoPanel01::HideUI()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UColonistInfoPanel01::RefreshPanel()
{
	// 从 PS 读取最新数据
	if (GameMode)
	{
		// 无实例时，从PlayerState获取最新数据
		UWorld* World = GameMode->GetWorld();
		if (World)
		{
			ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
			if (PS)
			{
				FColonistData01 UpdatedData = PS->GetColonistDataCopy(ColonistData.EntityID);
				if (UpdatedData.EntityID == ColonistData.EntityID)
				{
					ColonistData = UpdatedData;
					
					// 尝试获取实例（如果殖民者已经出城）
					Colonist = PS->FindColonistInstance(ColonistData.EntityID);
				}
			}
		}
	}

	// 更新显示
	UpdateColonistInfo();
	UpdateButtonStates();
}

void UColonistInfoPanel01::OnFoundCityClicked()
{
	// 触发蓝图确认事件（可选）
	OnFoundCityConfirm();
	
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[ColonistInfoPanel] GameMode为空，无法建城"));
		return;
	}
	
	// 检查是否已有待执行的建城命令
	if (ColonistData.bHasPendingFoundCityCommand)
	{
		// 取消建城命令（直接修改数据层）
		{
			UWorld* World = GameMode->GetWorld();
			if (World)
			{
				ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
				if (PS)
				{
					FColonistData01* Data = PS->GetColonistData(ColonistData.EntityID);
					if (Data)
					{
						Data->bHasPendingFoundCityCommand = false;
						Data->FoundCityTargetCoord = FIntVector2::ZeroValue;
					}
				}
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 建城命令已取消"));
		
		// 刷新面板
		RefreshPanel();
		return;
	}
	
	// 检查是否可以在目的地建城
	bool bCanFound = false;
	
	// 确定目标坐标：移动中则取路径终点，否则取当前坐标
	FIntVector2 TargetCoord = (ColonistData.bIsMoving && ColonistData.MovementPath.Num() > 0)
		? ColonistData.MovementPath.Last()
		: ColonistData.GridCoord;
	
	bCanFound = GameMode->CanFoundCityAtCoord(TargetCoord, 3);
	
	if (!bCanFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ColonistInfoPanel] 无法在坐标 (%d,%d) 建城（距离其他城市太近）"), TargetCoord.X, TargetCoord.Y);
		return;
	}
	
	// 判断是立即建城还是设置建城命令
	bool bIsAtDestination = false;
	bIsAtDestination = !ColonistData.bIsMoving &&
					   (ColonistData.State == EColonistState01::Idle ||
					    ColonistData.State == EColonistState01::Arrived);
	
	if (bIsAtDestination)
	{
		// 立即建城
		ATownActor01* NewTown = GameMode->CreateCityFromColonist(ColonistData.EntityID);
		
		if (NewTown)
		{
			UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 建城成功: %s"), *NewTown->EntityName);
			OnCityFounded(NewTown->EntityName);
			HideUI();
			OnPanelClosed();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ColonistInfoPanel] 建城失败"));
		}
	}
	else
	{
		// 设置建城命令（殖民者在移动中）
		// 设置建城命令（直接修改数据层）
		{
			UWorld* World = GameMode->GetWorld();
			if (World)
			{
				ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
				if (PS)
				{
					FColonistData01* Data = PS->GetColonistData(ColonistData.EntityID);
						if (Data)
						{
							Data->bHasPendingFoundCityCommand = true;
							Data->FoundCityTargetCoord = TargetCoord;  // 使用目标坐标而不是当前坐标
						}
					}
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 建城命令已设置，将在到达目的地后执行"));
		
		// 刷新面板以更新按钮状态
		RefreshPanel();
	}
}

void UColonistInfoPanel01::OnDeleteColonistClicked()
{
	// 触发蓝图确认事件（可选）
	OnDeleteColonistConfirm();
	
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[ColonistInfoPanel] GameMode为空，无法删除殖民者"));
		return;
	}
	
	UWorld* World = GameMode->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[ColonistInfoPanel] 无法获取World"));
		return;
	}
	
	ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[ColonistInfoPanel] 无法获取PlayerState"));
		return;
	}
	
	// 执行删除
	if (Colonist)
	{
		// 有实例版本：销毁Actor并从PlayerState移除数据
		int32 ColonistID = Colonist->EntityID;
		
		// 如果殖民者在城镇中，需要从城镇列表移除并刷新城镇UI
		if (ColonistData.State == EColonistState01::InTown && ColonistData.CurrentTownID != -1)
		{
			ATownActor01* Town = PS->FindTownInstance(ColonistData.CurrentTownID);
			if (Town)
			{
				Town->RemoveColonistByID(ColonistID);
				if (GameMode->GetUIManager())
				{
					GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
				}
			}
		}
		
		// 销毁Actor实例（不在城镇时实体在世界中）
		Colonist->Destroy();
		Colonist = nullptr;
		
		// 从PlayerState移除数据
		PS->RemoveColonistData(ColonistID);
		
		UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 殖民者已删除: %s (ID: %d)"), 
			*ColonistData.EntityName, ColonistID);
	}
	else
	{
		// 无实例版本：从PlayerState和城镇列表移除数据
		int32 ColonistID = ColonistData.EntityID;
		
		// +++ 新增：查找并销毁实体实例 +++
		AMoveableEntity01* ColonistInstance = PS->FindColonistInstance(ColonistID);
		if (ColonistInstance)
		{
			// 如果殖民者在城镇中，需要从城镇列表移除
			if (ColonistData.State == EColonistState01::InTown && ColonistData.CurrentTownID != -1)
			{
				ATownActor01* Town = PS->FindTownInstance(ColonistData.CurrentTownID);
				if (Town)
				{
					Town->RemoveColonistByID(ColonistID);
					
					// 刷新城镇UI
					if (GameMode->GetUIManager())
					{
						GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
					}
				}
			}
			
			// 销毁Actor实例
			ColonistInstance->Destroy();
			UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 殖民者实体已销毁: %s (ID: %d)"), 
				*ColonistData.EntityName, ColonistID);
		}
		// --- 新增结束 ---
		else
		{
			// 如果没有实例，仅处理数据移除（殖民者在城镇中或无实体）
			// 如果殖民者在城镇中，需要从城镇列表移除
			if (ColonistData.State == EColonistState01::InTown && ColonistData.CurrentTownID != -1)
			{
				ATownActor01* Town = PS->FindTownInstance(ColonistData.CurrentTownID);
				if (Town)
				{
					Town->RemoveColonistByID(ColonistID);
					
					// 刷新城镇UI
					if (GameMode->GetUIManager())
					{
						GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
					}
				}
			}
		}
		
		// 从PlayerState移除数据
		PS->RemoveColonistData(ColonistID);
		
		UE_LOG(LogTemp, Log, TEXT("[ColonistInfoPanel] 殖民者数据已删除: %s (ID: %d)"), 
			*ColonistData.EntityName, ColonistID);
	}
	
	// 触发蓝图事件
	OnColonistDeleted();
	
	// 关闭面板
	HideUI();
	OnPanelClosed();
}

void UColonistInfoPanel01::OnCloseClicked()
{
	// 通知UIManager用户主动关闭面板
	UWorld* World = GetWorld();
	if (World)
	{
		ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
		if (GM && GM->GetUIManager())
		{
			GM->GetUIManager()->OnMoveableUnitPanelClosedByUser();
		}
	}
	
	// 触发蓝图事件
	OnPanelClosed();
}

void UColonistInfoPanel01::UpdateColonistInfo()
{
	// 更新殖民者名称
	if (ColonistNameText)
	{
		ColonistNameText->SetText(FText::FromString(ColonistData.EntityName));
	}

	// 更新状态
	if (ColonistStatusText)
	{
		ColonistStatusText->SetText(GetColonistStatusText());
	}

	// 更新位置
	if (ColonistLocationText)
	{
		ColonistLocationText->SetText(GetColonistLocationText());
	}
}

FText UColonistInfoPanel01::GetColonistStatusText() const
{
	FString StatusStr;
	switch (ColonistData.State)
	{
	case EColonistState01::Idle:
		StatusStr = TEXT("空闲 - 可以移动或建城");
		break;
	case EColonistState01::Moving:
		StatusStr = TEXT("移动中");
		break;
	case EColonistState01::Arrived:
		StatusStr = TEXT("已到达目标位置");
		break;
	case EColonistState01::InTown:
		{
			FString TownName = GetCurrentTownName();
			if (!TownName.IsEmpty())
			{
				StatusStr = FString::Printf(TEXT("在城镇中 - %s"), *TownName);
			}
			else
			{
				StatusStr = TEXT("在城镇中");
			}
		}
		break;
	default:
		StatusStr = TEXT("未知状态");
		break;
	}
	
	return FText::FromString(StatusStr);
}

FText UColonistInfoPanel01::GetColonistLocationText() const
{
	FString LocationStr = FString::Printf(TEXT("位置: (%d, %d)"), 
		ColonistData.GridCoord.X, ColonistData.GridCoord.Y);
	return FText::FromString(LocationStr);
}

void UColonistInfoPanel01::UpdateButtonStates()
{
	// 建城按钮状态
	if (FoundCityButton)
	{
		bool bCanFoundCity = false;
		bool bHasPendingCommand = ColonistData.bHasPendingFoundCityCommand;
		
		// 基于数据层判断是否可以建城（不依赖实例）
		if (GameMode)
		{
			// 取目的地坐标：移动中则取路径终点，否则取当前坐标
			FIntVector2 DestCoord = (ColonistData.bIsMoving && ColonistData.MovementPath.Num() > 0)
				? ColonistData.MovementPath.Last()
				: ColonistData.GridCoord;
			
			// 检查建城条件：
			// 1. 如果正在移动，检查移动终点是否可以建城（忽略当前状态）
			// 2. 如果没有移动，检查当前状态是否在城镇中，以及当前坐标是否可以建城
			if (ColonistData.bIsMoving && ColonistData.MovementPath.Num() > 0)
			{
				// 正在移动：只检查移动终点是否可以建城
				bCanFoundCity = GameMode->CanFoundCityAtCoord(DestCoord, 3);
			}
			else
			{
				// 没有移动：检查当前状态是否在城镇中，以及当前坐标是否可以建城
				bCanFoundCity = (ColonistData.State != EColonistState01::InTown) && 
					GameMode->CanFoundCityAtCoord(DestCoord, 3);
			}
		}
		
		// 更新按钮文本和状态
		if (bHasPendingCommand)
		{
			// 有待执行的建城命令，按钮显示"取消建城"
			FoundCityButton->SetIsEnabled(true);
		}
		else
		{
			// 没有建城命令，按钮显示"建城"
			FoundCityButton->SetIsEnabled(bCanFoundCity);
		}
	}
	
	// 删除按钮始终可用
	if (DeleteColonistButton)
	{
		DeleteColonistButton->SetIsEnabled(true);
	}
}

FString UColonistInfoPanel01::GetCurrentTownName() const
{
	if (ColonistData.CurrentTownID == -1 || !GameMode)
	{
		return FString();
	}
	
	UWorld* World = GameMode->GetWorld();
	if (!World)
	{
		return FString();
	}
	
	ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return FString();
	}
	
	ATownActor01* Town = PS->FindTownInstance(ColonistData.CurrentTownID);
	if (Town)
	{
		return Town->EntityName;
	}
	
	return FString();
}