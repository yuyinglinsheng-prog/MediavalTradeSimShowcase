// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/CaravanInfoPanel01.h"
#include "Demo01/UIs/Panels/CaravanInventoryPage01.h"
#include "Demo01/UIs/Panels/CaravanTradeRoutePage01.h"
#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

void UCaravanInfoPanel01::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定关闭按钮事件
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UCaravanInfoPanel01::OnCloseClicked);
	}
	
	// 绑定删除商队按钮事件
	if (DeleteCaravanButton)
	{
		DeleteCaravanButton->OnClicked.AddDynamic(this, &UCaravanInfoPanel01::OnDeleteCaravanClicked);
	}
	
	// 绑定标签页按钮事件
	if (InventoryTabButton)
	{
		InventoryTabButton->OnClicked.AddDynamic(this, &UCaravanInfoPanel01::OnInventoryTabClicked);
	}
	
	if (TradeRouteTabButton)
	{
		TradeRouteTabButton->OnClicked.AddDynamic(this, &UCaravanInfoPanel01::OnTradeRouteTabClicked);
	}
}

void UCaravanInfoPanel01::InitializePanel(AMoveableEntity01* InCaravan)
{
	if (!InCaravan)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInfoPanel] InitializePanel: 商队为空"));
		return;
	}

	Caravan = InCaravan;
	if (GameMode)
	{
		ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (FCaravanData01* LiveData = PS ? PS->GetCaravanDataPtr(InCaravan->EntityID) : nullptr)
		{
			CaravanData = *LiveData;
		}
	}
	
	// 初始化页面引用
	InitializePageReferences();
	
	// 更新商队基本信息
	UpdateCaravanInfo();
	
	// 初始化库存页面
	if (InventoryPage)
	{
		InventoryPage->InitializePage(Caravan);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInfoPanel] InventoryPage 组件未绑定"));
	}
	
	// 初始化贸易路线页面
	if (TradeRoutePage)
	{
		TradeRoutePage->InitializePage(CaravanData);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInfoPanel] TradeRoutePage 未初始化"));
	}
	
	// 设置默认标签页
	SwitchToTab(ECaravanTabType::Inventory);
	UpdateTabButtonStates();

	UE_LOG(LogTemp, Log, TEXT("[CaravanInfoPanel] 商队面板初始化完成: %s"), *Caravan->EntityName);
}

void UCaravanInfoPanel01::InitializeUI(ADemo01_GM* InGameMode)
{
	Super::InitializeUI(InGameMode);
	
	// 初始化子页面
	InitializePageReferences();
}

void UCaravanInfoPanel01::SetCaravanData(const FCaravanData01& InCaravanData)
{
	// 存储商队数据
	CaravanData = InCaravanData;
	
	// 初始化页面引用（如果还没有）
	if (!TradeRoutePage && ContentSwitcher)
	{
		InitializePageReferences();
	}
	
	// 刷新显示
	RefreshPanel();
	
	// 刷新贸易路线页面
	if (TradeRoutePage)
	{
		TradeRoutePage->InitializePage(CaravanData);
	}
}

void UCaravanInfoPanel01::SetCaravanInstance(AMoveableEntity01* InCaravan)
{
	Caravan = InCaravan;

	if (Caravan && GameMode)
	{
		ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (FCaravanData01* LiveData = PS ? PS->GetCaravanDataPtr(Caravan->EntityID) : nullptr)
		{
			CaravanData = *LiveData;
		}
	}
	
	// 初始化页面引用（如果还没有）
	if (!TradeRoutePage && ContentSwitcher)
	{
		InitializePageReferences();
	}
	
	// 更新商队基本信息
	UpdateCaravanInfo();
	
	// 初始化库存页面
	if (InventoryPage)
	{
		// 注意：InventoryPage需要修改以支持无实例情况
		if (Caravan)
		{
			InventoryPage->InitializePage(Caravan);
		}
		else
		{
			// 无实例时，使用数据初始化
			InventoryPage->InitializePageWithData(CaravanData);
		}
	}
	
	// 刷新贸易路线页面
	if (TradeRoutePage)
	{
		TradeRoutePage->InitializePage(CaravanData);
	}
}

void UCaravanInfoPanel01::HideUI()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UCaravanInfoPanel01::RefreshPanel()
{
	// 更新商队基本信息（使用数据，不依赖实例）
	UpdateCaravanInfo();
	
	// 刷新库存页面
	if (InventoryPage)
	{
		InventoryPage->RefreshInventoryList();
	}
	
	// 刷新贸易路线页面
	if (TradeRoutePage)
	{
		TradeRoutePage->UpdateUI();
	}
}

void UCaravanInfoPanel01::OnCloseClicked()
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

void UCaravanInfoPanel01::UpdateCaravanInfo()
{
	// 如果有实例，从 PS 读取最新数据
	if (Caravan && GameMode)
	{
		ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (FCaravanData01* LiveData = PS ? PS->GetCaravanDataPtr(Caravan->EntityID) : nullptr)
		{
			CaravanData = *LiveData;
		}
	}

	// 更新商队名称
	if (CaravanNameText)
	{
		CaravanNameText->SetText(FText::FromString(CaravanData.EntityName));
	}

	// 更新商队状态
	if (CaravanStatusText)
	{
		CaravanStatusText->SetText(GetCaravanStatusText());
	}

	// 更新商队位置
	if (CaravanLocationText)
	{
		CaravanLocationText->SetText(GetCaravanLocationText());
	}
}

FText UCaravanInfoPanel01::GetCaravanStatusText() const
{
	switch (CaravanData.State)
	{
		case ECaravanState01::Idle:
			return FText::FromString(TEXT("空闲"));
		case ECaravanState01::Moving:
			return FText::FromString(TEXT("移动中"));
		case ECaravanState01::Arrived:
			return FText::FromString(TEXT("已到达"));
		case ECaravanState01::InTown:
			return FText::FromString(TEXT("在城镇中"));
		default:
			return FText::FromString(TEXT("未知"));
	}
}

FText UCaravanInfoPanel01::GetCaravanLocationText() const
{
	FIntVector2 GridCoord = CaravanData.GridCoord;
	return FText::Format(
		FText::FromString(TEXT("位置: ({0}, {1})")),
		FText::AsNumber(GridCoord.X),
		FText::AsNumber(GridCoord.Y)
	);
}

void UCaravanInfoPanel01::OnDeleteCaravanClicked()
{
	// 触发蓝图确认事件（可选）
	OnDeleteCaravanConfirm();
	
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInfoPanel] GameMode为空，无法删除商队"));
		return;
	}
	
	UWorld* World = GameMode->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInfoPanel] 无法获取World"));
		return;
	}
	
	ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInfoPanel] 无法获取PlayerState"));
		return;
	}
	
	// 执行删除
	int32 CaravanID = CaravanData.EntityID;
	
	if (Caravan)
	{
		// 有实例版本：销毁Actor并从PlayerState移除数据
		
		// 如果商队在城镇中，需要从城镇列表移除并刷新城镇UI
		if (CaravanData.State == ECaravanState01::InTown && CaravanData.CurrentTownID != -1)
		{
			ATownActor01* Town = PS->FindTownInstance(CaravanData.CurrentTownID);
			if (Town)
			{
				Town->RemoveCaravanByID(CaravanID);
				if (GameMode->GetUIManager())
				{
					GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
				}
			}
		}
		
		// 销毁Actor实例（不在城镇时实体在世界中）
		Caravan->Destroy();
		Caravan = nullptr;
		
		// 从PlayerState移除数据
		PS->RemoveCaravan(CaravanID);
		
		UE_LOG(LogTemp, Log, TEXT("[CaravanInfoPanel] 商队已删除: %s (ID: %d)"), 
			*CaravanData.EntityName, CaravanID);
	}
	else
	{
		// 无实例版本：从PlayerState和城镇列表移除数据
		
		// +++ 新增：查找并销毁实体实例 +++
		AMoveableEntity01* CaravanInstance = PS->FindCaravanInstance(CaravanID);
		if (CaravanInstance)
		{
			// 如果商队在城镇中，需要从城镇列表移除
			if (CaravanData.State == ECaravanState01::InTown && CaravanData.CurrentTownID != -1)
			{
				ATownActor01* Town = PS->FindTownInstance(CaravanData.CurrentTownID);
				if (Town)
				{
					Town->RemoveCaravanByID(CaravanID);
					
					// 刷新城镇UI
					if (GameMode->GetUIManager())
					{
						GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
					}
				}
			}
			
			// 销毁Actor实例
			CaravanInstance->Destroy();
			UE_LOG(LogTemp, Log, TEXT("[CaravanInfoPanel] 商队实体已销毁: %s (ID: %d)"), 
				*CaravanData.EntityName, CaravanID);
		}
		// --- 新增结束 ---
		else
		{
			// 如果没有实例，仅处理数据移除（商队在城镇中或无实体）
			// 如果商队在城镇中，需要从城镇列表移除
			if (CaravanData.State == ECaravanState01::InTown && CaravanData.CurrentTownID != -1)
			{
				ATownActor01* Town = PS->FindTownInstance(CaravanData.CurrentTownID);
				if (Town)
				{
					Town->RemoveCaravanByID(CaravanID);
					
					// 刷新城镇UI
					if (GameMode->GetUIManager())
					{
						GameMode->GetUIManager()->RefreshTownInfoPanelIfOpen(Town);
					}
				}
			}
		}
		
		// 从PlayerState移除数据
		PS->RemoveCaravan(CaravanID);
		
		UE_LOG(LogTemp, Log, TEXT("[CaravanInfoPanel] 商队数据已删除: %s (ID: %d)"), 
			*CaravanData.EntityName, CaravanID);
	}
	
	// 关闭面板
	HideUI();
	OnPanelClosed();
	
	// 通知UIManager面板已关闭
	if (GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->OnMoveableUnitPanelClosedByUser();
	}
}

void UCaravanInfoPanel01::SwitchToTab(ECaravanTabType TabType)
{
	CurrentTab = TabType;
	
	if (!ContentSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("[CaravanInfoPanel] ContentSwitcher 未绑定"));
		return;
	}
	
	// 根据标签类型切换WidgetSwitcher的活跃子项
	switch (TabType)
	{
	case ECaravanTabType::Inventory:
		ContentSwitcher->SetActiveWidget(InventoryPage);
		break;
		
	case ECaravanTabType::TradeRoute:
		ContentSwitcher->SetActiveWidget(TradeRoutePage);
		break;
	}
	
	UpdateTabButtonStates();
}

void UCaravanInfoPanel01::OnInventoryTabClicked()
{
	SwitchToTab(ECaravanTabType::Inventory);
}

void UCaravanInfoPanel01::OnTradeRouteTabClicked()
{
	SwitchToTab(ECaravanTabType::TradeRoute);
}

void UCaravanInfoPanel01::UpdateTabButtonStates()
{
	// 更新按钮样式（在蓝图中通过绑定实现）
	// 这里可以添加C++逻辑来设置按钮的视觉状态
}

void UCaravanInfoPanel01::InitializePageReferences()
{
	if (!ContentSwitcher)
	{
		return;
	}
	
	// 从WidgetSwitcher获取子页面引用
	int32 ChildCount = ContentSwitcher->GetChildrenCount();
	for (int32 i = 0; i < ChildCount; i++)
	{
		UWidget* Child = ContentSwitcher->GetChildAt(i);
		if (!Child)
		{
			continue;
		}
		
		// 尝试转换为贸易路线页面
		if (!TradeRoutePage)
		{
			TradeRoutePage = Cast<UCaravanTradeRoutePage01>(Child);
		}
	}
	
	// 如果找到了贸易路线页面且未初始化，初始化它
	if (TradeRoutePage && GameMode && !TradeRoutePage->IsInitialized())
	{
		TradeRoutePage->InitializeUI(GameMode);
		
		// 如果已经有商队数据，也初始化页面
		if (CaravanData.EntityID > 0)
		{
			TradeRoutePage->InitializePage(CaravanData);
		}
	}
}