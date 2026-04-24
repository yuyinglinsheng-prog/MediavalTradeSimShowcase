// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/CaravanTradeRoutePage01.h"
#include "Demo01/UIs/Widgets/CaravanTradeRouteTownEntryWidget01.h"
#include "Demo01/UIs/Core/Demo01StyleManager.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/TradeRouteManager01.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/Actors/PathVisualizerActor01.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/ComboBoxString.h"

void UCaravanTradeRoutePage01::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定按钮事件
	if (AutoTradeButton)
	{
		AutoTradeButton->OnClicked.AddDynamic(this, &UCaravanTradeRoutePage01::OnAutoTradeClicked);
	}
	
	if (MapSelectButton)
	{
		MapSelectButton->OnClicked.AddDynamic(this, &UCaravanTradeRoutePage01::OnMapSelectClicked);
	}
	
	if (DeleteRouteButton)
	{
		DeleteRouteButton->OnClicked.AddDynamic(this, &UCaravanTradeRoutePage01::OnDeleteRouteClicked);
	}
	
	// 绑定下拉框事件
	if (AddTownComboBox)
	{
		AddTownComboBox->OnSelectionChanged.AddDynamic(this, &UCaravanTradeRoutePage01::OnAddTownSelectionChanged);
	}
}

void UCaravanTradeRoutePage01::InitializePage(const FCaravanData01& InCaravanData)
{
	CaravanData = InCaravanData;
	
	// 初始化下拉框选项
	if (AddTownComboBox)
	{
		AddTownComboBox->ClearOptions();
		
		TArray<FString> TownOptions = GetAllTownOptions();
		for (const FString& Option : TownOptions)
		{
			AddTownComboBox->AddOption(Option);
		}
		
		// 添加默认选项
		AddTownComboBox->AddOption(TEXT("选择城镇..."));
		AddTownComboBox->SetSelectedOption(TEXT("选择城镇..."));
	}
	
	UpdateUI();
}

void UCaravanTradeRoutePage01::UpdateUI()
{
	UpdateAutoTradeStatus();
	UpdateMoneyChangeDisplay();
	CreateTownEntries();
}

void UCaravanTradeRoutePage01::CreateTownEntries()
{
	if (!TownListBox)
	{
		return;
	}
	
	// 清空现有条目
	TownListBox->ClearChildren();
	
	// 如果没有城镇条目类，直接返回
	if (!TownEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CaravanTradeRoutePage] TownEntryWidgetClass 未设置"));
		return;
	}
	
	// 为每个城镇创建条目
	int32 TotalCount = CaravanData.TradeRoute.TownVisitOrder.Num();
	for (int32 i = 0; i < TotalCount; i++)
	{
		int32 TownID = CaravanData.TradeRoute.TownVisitOrder[i];
		
		UCaravanTradeRouteTownEntryWidget01* Entry = CreateWidget<UCaravanTradeRouteTownEntryWidget01>(this, TownEntryWidgetClass);
		if (Entry)
		{
			// 初始化GameMode
			Entry->InitializeUI(GameMode);
			
			Entry->InitializeEntry(TownID, i, this);
			Entry->UpdateButtonStates(TotalCount);
			TownListBox->AddChild(Entry);
			
			// 应用样式
			UDemo01StyleManager* StyleManager = UDemo01StyleManager::GetInstance();
			if (StyleManager)
			{
				UE_LOG(LogTemp, Log, TEXT("[CaravanTradeRoutePage] 为 %s 应用样式"), *Entry->GetName());
				StyleManager->ApplyStyleToWidget(Entry);
			}
		}
	}
}

void UCaravanTradeRoutePage01::UpdateAutoTradeStatus()
{
	if (!AutoTradeStatusText)
	{
		return;
	}
	
	FString StatusText = CaravanData.bIsAutoTrading ? TEXT("自动贸易: 开启") : TEXT("自动贸易: 关闭");
	AutoTradeStatusText->SetText(FText::FromString(StatusText));
}

void UCaravanTradeRoutePage01::UpdateMoneyChangeDisplay()
{
	if (!MoneyChangeText)
	{
		return;
	}
	
	FString MoneyText;
	if (CaravanData.bIsAutoTrading && CaravanData.TradeRoute.bHasCompletedLoop)
	{
		MoneyText = FString::Printf(TEXT("上次循环金钱变化: %.2f 金币"), CaravanData.TradeRoute.RouteMoneyChange);
	}
	else
	{
		MoneyText = TEXT("金钱变化: 暂无数据");
	}
	
	MoneyChangeText->SetText(FText::FromString(MoneyText));
}

void UCaravanTradeRoutePage01::OnAutoTradeClicked()
{
	if (!GameMode || CaravanData.EntityID <= 0)
	{
		return;
	}
	
	// 切换自动贸易状态
	bool bNewState = !CaravanData.bIsAutoTrading;
	
	// 如果要开启自动贸易，检查路线是否有效
	if (bNewState && !CaravanData.TradeRoute.bIsRouteValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CaravanTradeRoute] 无法开启自动贸易：商队 %d 的贸易路线无效"), CaravanData.EntityID);
		return;  // 阻止开启
	}
	
	UTradeRouteManager01* TRM = GameMode->GetTradeRouteManager();
	if (TRM)
	{
		TRM->SetCaravanAutoTrading(CaravanData.EntityID, bNewState);
		CaravanData.bIsAutoTrading = bNewState;
		UpdateAutoTradeStatus();
		
		// 切换路径可视化：开启时显示贸易路线，关闭时清除路径
		ADemo01_PC* PC = Cast<ADemo01_PC>(GameMode->GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			if (bNewState)
			{
				// 开启自动贸易，显示贸易路线
				PC->ShowTradeRoute(CaravanData.EntityID);
				UE_LOG(LogTemp, Log, TEXT("[CaravanTradeRoute] 显示商队 %d 的贸易路线"), CaravanData.EntityID);
			}
			else
			{
				// 关闭自动贸易，清除贸易路线显示
				APathVisualizerActor01* PathVisualizer = GameMode->GetPathVisualizer();
				if (PathVisualizer)
				{
					PathVisualizer->ClearTradeRoute();
					UE_LOG(LogTemp, Log, TEXT("[CaravanTradeRoute] 清除商队 %d 的贸易路线"), CaravanData.EntityID);
				}
			}
		}
		
		// Debug: 打印状态切换
		UE_LOG(LogTemp, Log, TEXT("[CaravanTradeRoute] 商队 %d 自动贸易状态: %s"), 
			CaravanData.EntityID, 
			bNewState ? TEXT("开启") : TEXT("关闭"));
	}
}

void UCaravanTradeRoutePage01::OnAddTownSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// 跳过默认选项和无效选择
	if (SelectedItem.IsEmpty() || SelectedItem == TEXT("选择城镇..."))
	{
		return;
	}
	
	// 解析城镇ID（格式: "[ID] 名称"）
	int32 OpenBracketIndex = SelectedItem.Find(TEXT("["));
	int32 CloseBracketIndex = SelectedItem.Find(TEXT("]"));
	
	if (OpenBracketIndex == -1 || CloseBracketIndex == -1 || CloseBracketIndex <= OpenBracketIndex)
	{
		return;
	}
	
	FString IDStr = SelectedItem.Mid(OpenBracketIndex + 1, CloseBracketIndex - OpenBracketIndex - 1);
	int32 TownID = FCString::Atoi(*IDStr);
	
	if (TownID <= 0)
	{
		return;
	}
	
	// 添加城镇到路线
	AddTownToRoute(TownID);
	
	// 重置下拉框选择
	if (AddTownComboBox)
	{
		AddTownComboBox->SetSelectedOption(TEXT("选择城镇..."));
	}
}

void UCaravanTradeRoutePage01::OnMapSelectClicked()
{
	// Demo01: 仅日志记录，不实现功能
	UE_LOG(LogTemp, Log, TEXT("地图选择功能将在后续版本实现"));
}

void UCaravanTradeRoutePage01::OnDeleteRouteClicked()
{
	// 清空路线
	CaravanData.TradeRoute.TownVisitOrder.Empty();
	CaravanData.TradeRoute.FullPathCoords.Empty();
	CaravanData.TradeRoute.TotalPathLength = 0;
	CaravanData.TradeRoute.CurrentVisitIndex = 0;
	CaravanData.TradeRoute.RouteSegmentValidity.Empty();
	CaravanData.TradeRoute.bIsRouteValid = false;
	
	// 更新到PlayerState
	ADemo01_PS* PS = GetPlayerState();
	if (PS)
	{
		if (FCaravanData01* LiveData = PS->GetCaravanDataPtr(CaravanData.EntityID))
		{
			LiveData->TradeRoute = CaravanData.TradeRoute;
		}
	}
	
	// 刷新UI（无需调用 RecalculateRoute，因为已清空）
	UpdateUI();
	
	UE_LOG(LogTemp, Log, TEXT("商队 %d 的贸易路线已删除"), CaravanData.EntityID);
}

void UCaravanTradeRoutePage01::SwapTownPosition(int32 TownID, int32 CurrentIndex, bool bMoveUp)
{
	if (!GameMode || CaravanData.EntityID <= 0)
	{
		return;
	}
	
	// 检查边界
	if (bMoveUp && CurrentIndex == 0) return;
	if (!bMoveUp && CurrentIndex >= CaravanData.TradeRoute.TownVisitOrder.Num() - 1) return;
	
	// 交换位置
	int32 SwapIndex = bMoveUp ? CurrentIndex - 1 : CurrentIndex + 1;
	CaravanData.TradeRoute.TownVisitOrder.Swap(CurrentIndex, SwapIndex);
	
	// 更新到PlayerState
	ADemo01_PS* PS = GetPlayerState();
	if (PS)
	{
		if (FCaravanData01* LiveData = PS->GetCaravanDataPtr(CaravanData.EntityID))
		{
			LiveData->TradeRoute.TownVisitOrder = CaravanData.TradeRoute.TownVisitOrder;
		}
	}
	
	// 触发路线重算
	UTradeRouteManager01* TRM = GameMode->GetTradeRouteManager();
	if (TRM)
	{
		TRM->RecalculateRoute(CaravanData.EntityID);
		
		// 从 PlayerState 获取最新的路线数据
		if (PS)
		{
			CaravanData.TradeRoute = PS->GetCaravanData(CaravanData.EntityID).TradeRoute;
		}
	}
	
	// 刷新UI
	UpdateUI();
}

void UCaravanTradeRoutePage01::RemoveTownFromRoute(int32 TownID)
{
	if (!GameMode || CaravanData.EntityID <= 0)
	{
		return;
	}
	
	// 从路线中移除
	CaravanData.TradeRoute.TownVisitOrder.Remove(TownID);
	
	// 更新到PlayerState
	ADemo01_PS* PS = GetPlayerState();
	if (PS)
	{
		if (FCaravanData01* LiveData = PS->GetCaravanDataPtr(CaravanData.EntityID))
		{
			LiveData->TradeRoute.TownVisitOrder = CaravanData.TradeRoute.TownVisitOrder;
		}
	}
	
	// 触发路线重算
	UTradeRouteManager01* TRM = GameMode->GetTradeRouteManager();
	if (TRM)
	{
		TRM->RecalculateRoute(CaravanData.EntityID);
		
		// 从 PlayerState 获取最新的路线数据
		if (PS)
		{
			CaravanData.TradeRoute = PS->GetCaravanData(CaravanData.EntityID).TradeRoute;
		}
	}
	
	// 刷新UI
	UpdateUI();
}

void UCaravanTradeRoutePage01::AddTownToRoute(int32 TownID)
{
	if (!GameMode || CaravanData.EntityID <= 0)
	{
		return;
	}
	
	// 允许添加重复城镇（包括已存在的）
	// 只在日志中记录，不阻止添加
	// if (CaravanData.TradeRoute.TownVisitOrder.Contains(TownID))
	// {
	// 	UE_LOG(LogTemp, Log, TEXT("[CaravanTradeRoute] 添加重复城镇 %d 到路线"), TownID);
	// }
	
	// 添加到路线末尾
	CaravanData.TradeRoute.TownVisitOrder.Add(TownID);
	
	// 更新到PlayerState
	ADemo01_PS* PS = GetPlayerState();
	if (PS)
	{
		if (FCaravanData01* LiveData = PS->GetCaravanDataPtr(CaravanData.EntityID))
		{
			LiveData->TradeRoute.TownVisitOrder = CaravanData.TradeRoute.TownVisitOrder;
		}
	}
	
	// 触发路线重算（调用 TradeRouteManager）
	UTradeRouteManager01* TRM = GameMode->GetTradeRouteManager();
	if (TRM)
	{
		TRM->RecalculateRoute(CaravanData.EntityID);
		
		// 从 PlayerState 获取最新的路线数据（包含 FullPathCoords）
		if (PS)
		{
			CaravanData.TradeRoute = PS->GetCaravanData(CaravanData.EntityID).TradeRoute;
		}
	}
	
	// 刷新UI
	UpdateUI();
}

TArray<FString> UCaravanTradeRoutePage01::GetAllTownOptions() const
{
	TArray<FString> Options;
	
	if (!GameMode)
	{
		return Options;
	}
	
	ADemo01_PS* PS = GetPlayerState();
	if (!PS)
	{
		return Options;
	}
	
	// 获取所有城镇
	TArray<int32> TownIDs = PS->GetAllTownIDs();
	for (int32 TownID : TownIDs)
	{
		FTownData01 TownData = PS->GetTownData(TownID);
		FString Option = FString::Printf(TEXT("[%d] %s"), TownID, *TownData.EntityName);
		Options.Add(Option);
	}
	
	return Options;
}

ADemo01_PS* UCaravanTradeRoutePage01::GetPlayerState() const
{
	if (!GameMode)
	{
		return nullptr;
	}
	
	return GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
}
