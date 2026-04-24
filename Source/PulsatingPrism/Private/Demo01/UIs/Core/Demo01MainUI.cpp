// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Core/Demo01MainUI.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/UIs/Panels/TownInfoPanel.h"
#include "Demo01/UIs/Panels/CaravanInfoPanel01.h"
#include "Demo01/UIs/Panels/ColonistInfoPanel01.h"
#include "Demo01/UIs/Panels/PausePanel01.h"
#include "Components/CanvasPanel.h"

void UDemo01MainUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 面板已经在蓝图中创建并通过BindWidget绑定
	// 初始隐藏所有面板
	if (TownInfoPanel)
	{
		TownInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (CaravanInfoPanel)
	{
		CaravanInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (ColonistInfoPanel)
	{
		ColonistInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// 暂停面板永久显示
	if (PausePanel)
	{
		PausePanel->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDemo01MainUI::InitializeMainUI(ADemo01_GM* InGameMode, const FDemo01UIConfig& InUIConfig)
{
	// 调用基类的InitializeUI来设置GameMode
	InitializeUI(InGameMode);
	UIConfig = InUIConfig;
	
	// 初始化面板
	if (TownInfoPanel)
	{
		TownInfoPanel->InitializeUI(GameMode);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 城镇面板已初始化"));
	}
	
	if (CaravanInfoPanel)
	{
		CaravanInfoPanel->InitializeUI(GameMode);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 商队面板已初始化"));
	}
	
	if (ColonistInfoPanel)
	{
		ColonistInfoPanel->InitializeUI(GameMode);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 殖民者面板已初始化"));
	}
	
	if (PausePanel)
	{
		PausePanel->InitializePanel(GameMode);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 暂停面板已初始化"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("[MainUI] 主UI容器初始化完成"));
}

// ===== 城镇面板管理 =====

void UDemo01MainUI::ShowTownInfoPanel(ATownActor01* Town)
{
	if (!Town)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainUI] 无法显示城镇面板：城镇为空"));
		return;
	}
	
	if (TownInfoPanel)
	{
		// 设置数据并显示
		TownInfoPanel->SetTownData(Town->TownData);
		TownInfoPanel->SetTownInstanceToPages(Town);
		TownInfoPanel->SetVisibility(ESlateVisibility::Visible);
		
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 城镇面板已显示：%s"), *Town->TownData.EntityName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 城镇面板未绑定，请检查蓝图配置"));
	}
}

void UDemo01MainUI::HideTownInfoPanel()
{
	if (TownInfoPanel)
	{
		TownInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 城镇面板已隐藏"));
	}
}

// ===== 商队面板管理 =====

void UDemo01MainUI::ShowCaravanInfoPanel(AMoveableEntity01* Caravan)
{
	if (!Caravan)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainUI] 无法显示商队面板：商队为空"));
		return;
	}
	
	if (CaravanInfoPanel)
	{
		CaravanInfoPanel->SetCaravanInstance(Caravan);
		CaravanInfoPanel->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 商队面板已显示：%s"), *Caravan->EntityName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 商队面板未绑定，请检查蓝图配置"));
	}
}

void UDemo01MainUI::ShowCaravanInfoPanelByID(int32 CaravanID)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainUI] 无法显示商队面板：GameMode为空"));
		return;
	}
	
	if (!CaravanInfoPanel)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 商队面板未绑定，请检查蓝图配置"));
		return;
	}
	
	// 通过PlayerState获取商队数据
	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 无法获取PlayerController"));
		return;
	}
	
	ADemo01_PS* PS = PC->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 无法获取PlayerState"));
		return;
	}
	
	// 获取商队数据
	FCaravanData01 CaravanData = PS->GetCaravanData(CaravanID);
	if (CaravanData.EntityID == CaravanID)
	{
		CaravanInfoPanel->SetCaravanData(CaravanData);
		CaravanInfoPanel->SetCaravanInstance(CaravanData.Instance);
		CaravanInfoPanel->SetVisibility(ESlateVisibility::Visible);
		
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 商队面板已显示（通过ID）：%s (ID: %d)"), 
			*CaravanData.EntityName, CaravanID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainUI] 找不到商队数据：ID %d"), CaravanID);
	}
}

void UDemo01MainUI::HideCaravanInfoPanel()
{
	if (CaravanInfoPanel)
	{
		CaravanInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 商队面板已隐藏"));
	}
}
// ===== 殖民者面板管理 =====

void UDemo01MainUI::ShowColonistInfoPanel(AMoveableEntity01* Colonist)
{
	if (!Colonist)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainUI] 无法显示殖民者面板：殖民者为空"));
		return;
	}
	
	if (ColonistInfoPanel)
	{
		ShowColonistInfoPanelByID(Colonist->EntityID);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 殖民者面板已显示：%s"), *Colonist->EntityName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 殖民者面板未绑定，请检查蓝图配置"));
	}
}

void UDemo01MainUI::ShowColonistInfoPanelByID(int32 ColonistID)
{
	if (ColonistID < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 无法显示殖民者面板：无效ID %d"), ColonistID);
		return;
	}
	
	if (!ColonistInfoPanel)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 殖民者面板未绑定，请检查蓝图配置"));
		return;
	}
	
	// 从PlayerState获取殖民者数据
	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 无法获取PlayerController"));
		return;
	}
	
	ADemo01_PS* PS = PC->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainUI] 无法获取PlayerState"));
		return;
	}
	
	FColonistData01 ColonistData = PS->GetColonistDataCopy(ColonistID);
	if (ColonistData.EntityID == ColonistID)
	{
		// 设置数据并显示（不需要重复初始化UI）
		ColonistInfoPanel->InitializePanelWithData(ColonistData);
		ColonistInfoPanel->SetVisibility(ESlateVisibility::Visible);
		
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 殖民者面板已显示（通过ID）：%s (ID: %d)"), 
			*ColonistData.EntityName, ColonistID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainUI] 找不到殖民者数据：ID %d"), ColonistID);
	}
}

void UDemo01MainUI::HideColonistInfoPanel()
{
	if (ColonistInfoPanel)
	{
		ColonistInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Log, TEXT("[MainUI] 殖民者面板已隐藏"));
	}
}