// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Widgets/CaravanTradeRouteTownEntryWidget01.h"
#include "Demo01/UIs/Panels/CaravanTradeRoutePage01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UCaravanTradeRouteTownEntryWidget01::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定按钮事件
	if (MoveUpButton)
	{
		MoveUpButton->OnClicked.AddDynamic(this, &UCaravanTradeRouteTownEntryWidget01::OnMoveUpClicked);
	}
	
	if (MoveDownButton)
	{
		MoveDownButton->OnClicked.AddDynamic(this, &UCaravanTradeRouteTownEntryWidget01::OnMoveDownClicked);
	}
	
	if (RemoveButton)
	{
		RemoveButton->OnClicked.AddDynamic(this, &UCaravanTradeRouteTownEntryWidget01::OnRemoveClicked);
	}
}

void UCaravanTradeRouteTownEntryWidget01::InitializeEntry(int32 InTownID, int32 InRouteIndex, UCaravanTradeRoutePage01* InParentPage)
{
	TownID = InTownID;
	RouteIndex = InRouteIndex;
	ParentPage = InParentPage;
	
	if (!GameMode)
	{
		return;
	}
	
	// 获取城镇名称
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 TownData = PS->GetTownData(TownID);
		
		if (TownNameText)
		{
			TownNameText->SetText(FText::FromString(TownData.EntityName));
		}
	}
	
	if (TownIDText)
	{
		TownIDText->SetText(FText::AsNumber(TownID));
	}
}

void UCaravanTradeRouteTownEntryWidget01::UpdateButtonStates(int32 TotalCount)
{
	// 上移按钮：如果不是第一个则启用
	if (MoveUpButton)
	{
		MoveUpButton->SetIsEnabled(RouteIndex > 0);
	}
	
	// 下移按钮：如果不是最后一个则启用
	if (MoveDownButton)
	{
		MoveDownButton->SetIsEnabled(RouteIndex < TotalCount - 1);
	}
}

void UCaravanTradeRouteTownEntryWidget01::OnMoveUpClicked()
{
	if (ParentPage)
	{
		ParentPage->SwapTownPosition(TownID, RouteIndex, true);
	}
}

void UCaravanTradeRouteTownEntryWidget01::OnMoveDownClicked()
{
	if (ParentPage)
	{
		ParentPage->SwapTownPosition(TownID, RouteIndex, false);
	}
}

void UCaravanTradeRouteTownEntryWidget01::OnRemoveClicked()
{
	if (ParentPage)
	{
		ParentPage->RemoveTownFromRoute(TownID);
	}
}
