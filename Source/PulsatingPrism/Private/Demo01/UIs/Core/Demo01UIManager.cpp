// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Demo01/UIs/Core/Demo01MainUI.h"
#include "Demo01/UIs/Core/Demo01StyleManager.h"
#include "Demo01/UIs/Panels/TownInfoPanel.h"
#include "Demo01/UIs/Panels/TownBuildingPage.h"
#include "Demo01/UIs/Panels/TownMarketPage.h"
#include "Demo01/UIs/Panels/CaravanInfoPanel01.h"
#include "Demo01/UIs/Panels/CaravanInventoryPage01.h"
#include "Demo01/UIs/Panels/ColonistInfoPanel01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void UDemo01UIManager::Initialize(ADemo01_GM* InGameMode, const FDemo01UIConfig& InUIConfig)
{
	GameMode = InGameMode;
	UIConfig = InUIConfig;
	
	// 初始化样式系统
	InitializeStyleSystem();
	
	// 创建并添加主UI到Viewport（只添加一次）
	if (UIConfig.MainUIWidgetClass)
	{
		MainUIWidget = CreateWidget<UDemo01MainUI>(GameMode->GetWorld(), UIConfig.MainUIWidgetClass);
		if (MainUIWidget)
		{
			MainUIWidget->AddToViewport();
			MainUIWidget->InitializeMainUI(GameMode, UIConfig);
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 主UI容器已创建并添加到Viewport"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[UIManager] 创建主UI容器失败"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] MainUIWidgetClass 未设置"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] UI 管理器初始化完成"));
	
	// 验证配置
	if (!UIConfig.TownInfoPanelClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] TownInfoPanelClass 未设置"));
	}
	
	if (!UIConfig.CaravanInfoPanelClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] CaravanInfoPanelClass 未设置"));
	}
}

void UDemo01UIManager::OpenTownInfoPanel(ATownActor01* Town)
{
	if (!Town)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] 无法打开城镇面板：城镇为空"));
		return;
	}
	
	if (!MainUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 主UI容器不存在"));
		return;
	}
	
	// 如果已经打开了同一个城镇的面板，不重复打开
	if (CurrentTown == Town && MainUIWidget->GetTownInfoPanel())
	{
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 城镇面板已打开：%s"), *Town->TownData.EntityName);
		return;
	}
	
	// 显示城镇面板
	MainUIWidget->ShowTownInfoPanel(Town);
	
	// 设置当前城镇
	CurrentTown = Town;
	
	// 订阅库存变化 delegate
	Town->OnInventoryChanged.AddUObject(this, &UDemo01UIManager::OnActiveTownInventoryChanged);
	
	// 切换到 GameAndUI 模式
	SetInputMode(EDemo01InputMode::GameAndUI);
	
	// 通知面板已打开
	NotifyTownPanelOpened(Town);
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 城镇面板已打开：%s"), *Town->TownData.EntityName);
}

void UDemo01UIManager::CloseTownInfoPanel()
{
	if (!MainUIWidget)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 关闭城镇面板：%s"), 
		CurrentTown ? *CurrentTown->TownData.EntityName : TEXT("Unknown"));
	
	// 隐藏面板
	MainUIWidget->HideTownInfoPanel();

	// 取消订阅库存变化 delegate
	if (CurrentTown)
	{
		CurrentTown->OnInventoryChanged.RemoveAll(this);
	}
	
	// 清除当前城镇
	CurrentTown = nullptr;
	
	// 通知面板已关闭
	NotifyTownPanelClosed();
}

void UDemo01UIManager::OpenCaravanInfoPanel(AMoveableEntity01* Caravan)
{
	if (!Caravan)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] 无法打开商队面板：商队为空"));
		return;
	}
	
	if (!MainUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 主UI容器不存在"));
		return;
	}
	
	// 如果已经打开了同一个商队的面板，不重复打开
	if (CurrentCaravan == Caravan && MainUIWidget->GetCaravanInfoPanel())
	{
		return;
	}
	
	// 显示商队面板
	MainUIWidget->ShowCaravanInfoPanel(Caravan);
	
	// 设置当前商队
	CurrentCaravan = Caravan;
	
	// 切换到 GameAndUI 模式
	SetInputMode(EDemo01InputMode::GameAndUI);
	
	// 通知面板已打开
	NotifyCaravanPanelOpened(Caravan);
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 商队面板已打开（通过实例）：%s"), *Caravan->EntityName);
}

void UDemo01UIManager::OpenCaravanInfoPanelByID(int32 CaravanID)
{
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] ========== OpenCaravanInfoPanelByID =========="));
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] 请求打开商队面板，CaravanID=%d"), CaravanID);
	
	if (CaravanID < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 无法打开商队面板：无效的商队ID %d"), CaravanID);
		return;
	}
	
	if (!MainUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 主UI容器不存在"));
		return;
	}
	
	// 从PlayerState获取商队数据
	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 无法获取PlayerController"));
		return;
	}
	
	ADemo01_PS* PS = PC->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 无法获取PlayerState"));
		return;
	}
	
	// 获取商队数据
	FCaravanData01 CaravanData = PS->GetCaravanData(CaravanID);
	if (CaravanData.EntityID != CaravanID)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 找不到商队数据：ID %d"), CaravanID);
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] 商队数据：Name=%s, State=%d, CurrentTownID=%d"), 
		*CaravanData.EntityName, (int32)CaravanData.State, CaravanData.CurrentTownID);
	
	// 尝试查找商队实例（如果在世界中）
	AMoveableEntity01* CaravanInstance = PS->FindCaravanInstance(CaravanID);
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] 商队实例查找结果：%s"), 
		CaravanInstance ? TEXT("找到实例") : TEXT("无实例（可能在城镇中）"));
	
	// 如果已经打开了同一个商队的面板，不重复打开
	if (MainUIWidget->GetCaravanInfoPanel() && CurrentCaravan && CurrentCaravan->EntityID == CaravanID)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] 商队面板已打开：%s，跳过"), *CaravanData.EntityName);
		return;
	}
	
	// 显示商队面板
	MainUIWidget->ShowCaravanInfoPanelByID(CaravanID);
	
	// 设置当前商队
	CurrentCaravan = CaravanInstance;
	
	// 切换到 GameAndUI 模式
	SetInputMode(EDemo01InputMode::GameAndUI);
	
	// 通知面板已打开
	// 修复：无论商队是否有实例，都要通知城镇面板设置CaravanID
	if (CaravanInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] 调用NotifyCaravanPanelOpened（有实例）"));
		NotifyCaravanPanelOpened(CaravanInstance);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] 商队无实例，但仍需通知面板设置CaravanID"));
		// 创建一个临时的CaravanActor01来传递CaravanID
		// 或者直接调用通知逻辑，传递CaravanID
		NotifyCaravanPanelOpenedByID(CaravanID);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] 商队面板已打开：%s (ID: %d)"), 
		*CaravanData.EntityName, CaravanID);
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] ========== OpenCaravanInfoPanelByID 结束 =========="));
}

void UDemo01UIManager::CloseCaravanInfoPanel()
{
	if (!MainUIWidget)
	{
		return;
	}
	
	// 隐藏面板
	MainUIWidget->HideCaravanInfoPanel();
	
	// 清除当前商队
	CurrentCaravan = nullptr;
	
	// 通知面板已关闭
	NotifyCaravanPanelClosed();
}

void UDemo01UIManager::SetInputMode(EDemo01InputMode Mode)
{
	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}
	
	// 鼠标始终显示（战略游戏特性）
	PC->SetShowMouseCursor(true);
	
	switch (Mode)
	{
	case EDemo01InputMode::GameOnly:
		PC->SetInputMode(FInputModeGameOnly());
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 输入模式：GameOnly"));
		break;
		
	case EDemo01InputMode::UIOnly:
		PC->SetInputMode(FInputModeUIOnly());
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 输入模式：UIOnly"));
		break;
		
	case EDemo01InputMode::GameAndUI:
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 输入模式：GameAndUI"));
		}
		break;
	}
}

bool UDemo01UIManager::HasAnyPanelOpen() const
{
	if (!MainUIWidget)
	{
		return false;
	}
	
	return MainUIWidget->GetTownInfoPanel() != nullptr || 
		   MainUIWidget->GetCaravanInfoPanel() != nullptr ||
		   MainUIWidget->GetColonistInfoPanel() != nullptr;
}

void UDemo01UIManager::RefreshTownInfoPanelIfOpen(ATownActor01* Town)
{
	if (!MainUIWidget || !Town)
	{
		return;
	}
	
	UTownInfoPanel* TownInfoPanel = MainUIWidget->GetTownInfoPanel();
	if (!TownInfoPanel)
	{
		return;
	}
	
	// 检查当前打开的面板是否是这个城镇的
	if (CurrentTown == Town)
	{
		// 刷新当前显示的页面数据
		TownInfoPanel->SetTownData(Town->TownData);
		
		// 优化：库存变化时只刷新市场页面，不刷新建筑页面
		// 建筑状态没有变化，只有资源数量变化，不需要刷新建筑条目
		UTownMarketPage* MarketPage = TownInfoPanel->GetMarketPage();
		if (MarketPage)
		{
			// 只刷新市场页面，不调用SetTownInstanceToPages避免刷新建筑页面
			MarketPage->RefreshPage();
			
			// 通过UI管理器获取当前选中的商队ID，保持架构解耦
			int32 SelectedCaravanID = GetCurrentSelectedCaravanID();
			if (SelectedCaravanID >= 0)
			{
				MarketPage->SetCaravanID(SelectedCaravanID);
			}
		}
		
		// UE_LOG(LogTemp, Log, TEXT("[UIManager] 刷新城镇面板：%s"), *Town->TownData.EntityName);
	}
}

void UDemo01UIManager::RefreshCaravanInfoPanelIfOpen(AMoveableEntity01* Caravan)
{
	if (!MainUIWidget || !Caravan)
	{
		return;
	}
	
	UCaravanInfoPanel01* CaravanInfoPanel = MainUIWidget->GetCaravanInfoPanel();
	if (!CaravanInfoPanel)
	{
		return;
	}
	
	// 检查当前打开的面板是否是这个商队的
	if (CurrentCaravan == Caravan)
	{
		CaravanInfoPanel->SetCaravanInstance(Caravan);
	}
}

void UDemo01UIManager::RefreshCaravanInfoPanelIfOpen(int32 CaravanID)
{
	if (!MainUIWidget)
	{
		return;
	}
	
	UCaravanInfoPanel01* CaravanInfoPanel = MainUIWidget->GetCaravanInfoPanel();
	if (!CaravanInfoPanel)
	{
		return;
	}
	
	// 获取PlayerState
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
	
	// 获取商队数据
	FCaravanData01 CaravanData = PS->GetCaravanData(CaravanID);
	if (CaravanData.EntityID != CaravanID)
	{
		return;
	}
	
	// 刷新商队面板数据
	CaravanInfoPanel->SetCaravanData(CaravanData);
	
	// 尝试获取实例并设置
	AMoveableEntity01* CaravanInstance = PS->FindCaravanInstance(CaravanID);
	CaravanInfoPanel->SetCaravanInstance(CaravanInstance);
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 刷新商队面板：%s (ID: %d)"), 
		*CaravanData.EntityName, CaravanID);
}

TSubclassOf<UUnitEntryWidget01> UDemo01UIManager::GetUnitEntryWidgetClass() const
{
	return UIConfig.UnitEntryWidgetClass;
}


// ===== 面板状态通知方法 =====

void UDemo01UIManager::NotifyTownPanelOpened(ATownActor01* Town)
{
	if (!MainUIWidget)
	{
		return;
	}
	
	// 如果商队面板已打开，更新其城镇引用和交易权限
	UCaravanInfoPanel01* CaravanInfoPanel = MainUIWidget->GetCaravanInfoPanel();
	if (CaravanInfoPanel && CaravanInfoPanel->IsInViewport())
	{
		AMoveableEntity01* CurrentCaravanActor = CaravanInfoPanel->GetCaravan();
		if (CurrentCaravanActor)
		{
			// 获取商队库存页面并设置城镇引用
			UCaravanInventoryPage01* InventoryPage = CaravanInfoPanel->GetInventoryPage();
			if (InventoryPage)
			{
				InventoryPage->SetTown(Town);
			}
		}
	}
	
	// 如果城镇面板的市场页面存在，设置商队ID
	UTownInfoPanel* TownInfoPanel = MainUIWidget->GetTownInfoPanel();
	if (TownInfoPanel && TownInfoPanel->IsInViewport() && CurrentCaravan)
	{
		UTownMarketPage* MarketPage = TownInfoPanel->GetMarketPage();
		if (MarketPage)
		{
			MarketPage->SetCaravanID(CurrentCaravan->EntityID);
		}
	}
}

void UDemo01UIManager::NotifyCaravanPanelOpened(AMoveableEntity01* Caravan)
{
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] ========== NotifyCaravanPanelOpened =========="));
	
	if (!MainUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] MainUIWidget为空"));
		return;
	}
	
	// 获取商队ID
	int32 CaravanID = -1;
	if (Caravan)
	{
		CaravanID = Caravan->EntityID;
	}
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] 传入的Caravan: %s, 计算的CaravanID: %d"), 
		Caravan ? TEXT("有效") : TEXT("nullptr"), CaravanID);
	
	// 如果城镇面板已打开，更新其商队引用和交易权限
	UTownInfoPanel* TownInfoPanel = MainUIWidget->GetTownInfoPanel();
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] TownInfoPanel: %s"), 
		TownInfoPanel ? TEXT("存在") : TEXT("不存在"));
	
	// 移除IsInViewport()检查，只要面板存在就更新
	if (TownInfoPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] TownInfoPanel存在，继续设置CaravanID"));
		UTownMarketPage* MarketPage = TownInfoPanel->GetMarketPage();
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] MarketPage: %s"), 
			MarketPage ? TEXT("存在") : TEXT("不存在"));
		
		if (MarketPage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UIManager] 调用MarketPage->SetCaravanID(%d)"), CaravanID);
			MarketPage->SetCaravanID(CaravanID);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[UIManager] ❌ MarketPage不存在，无法设置CaravanID"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] TownInfoPanel不存在"));
	}
	
	// 如果商队面板的库存页面存在，设置城镇引用
	UCaravanInfoPanel01* CaravanInfoPanel = MainUIWidget->GetCaravanInfoPanel();
	if (CaravanInfoPanel && CaravanInfoPanel->IsInViewport() && CurrentTown)
	{
		UCaravanInventoryPage01* InventoryPage = CaravanInfoPanel->GetInventoryPage();
		if (InventoryPage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UIManager] 设置商队库存页面的城镇引用"));
			InventoryPage->SetTown(CurrentTown);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] ========== NotifyCaravanPanelOpened 结束 =========="));
}

void UDemo01UIManager::NotifyTownPanelClosed()
{
	if (!MainUIWidget)
	{
		return;
	}
	
	// 如果商队面板已打开，清除其城镇引用
	UCaravanInfoPanel01* CaravanInfoPanel = MainUIWidget->GetCaravanInfoPanel();
	if (CaravanInfoPanel && CaravanInfoPanel->IsInViewport())
	{
		UCaravanInventoryPage01* InventoryPage = CaravanInfoPanel->GetInventoryPage();
		if (InventoryPage)
		{
			InventoryPage->SetTown(nullptr);
		}
	}
}

void UDemo01UIManager::NotifyCaravanPanelClosed()
{
	if (!MainUIWidget)
	{
		return;
	}
	
	// 如果城镇面板已打开，清除其商队ID
	UTownInfoPanel* TownInfoPanel = MainUIWidget->GetTownInfoPanel();
	if (TownInfoPanel && TownInfoPanel->IsInViewport())
	{
		UTownMarketPage* MarketPage = TownInfoPanel->GetMarketPage();
		if (MarketPage)
		{
			MarketPage->SetCaravanID(-1);
		}
	}
}

void UDemo01UIManager::NotifyCaravanStateChanged(int32 CaravanID)
{
	if (!MainUIWidget)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 商队状态变化通知：CaravanID=%d"), CaravanID);
	
	// 如果城镇面板已打开，更新其交易权限
	UTownInfoPanel* TownInfoPanel = MainUIWidget->GetTownInfoPanel();
	// 移除IsInViewport()检查，只要面板存在就更新
	if (TownInfoPanel)
	{
		UTownMarketPage* MarketPage = TownInfoPanel->GetMarketPage();
		if (MarketPage && MarketPage->GetCurrentCaravanID() == CaravanID)
		{
			// 当前选中的商队状态变化，更新交易权限
			MarketPage->UpdateTradePermission();
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 更新城镇市场页面交易权限"));
		}
	}
	
	// 如果商队面板已打开，更新其交易权限
	UCaravanInfoPanel01* CaravanInfoPanel = MainUIWidget->GetCaravanInfoPanel();
	// 移除IsInViewport()检查，只要面板存在就更新
	if (CaravanInfoPanel)
	{
		// 用ID比对，不依赖实例（商队进城后实例已Destroy）
		if (CaravanInfoPanel->GetCaravanID() == CaravanID)
		{
			UCaravanInventoryPage01* InventoryPage = CaravanInfoPanel->GetInventoryPage();
			if (InventoryPage)
			{
				// 更新交易权限
				InventoryPage->UpdateTradePermission();
				UE_LOG(LogTemp, Log, TEXT("[UIManager] 更新商队库存页面交易权限"));
			}
		}
	}
}

void UDemo01UIManager::NotifyCaravanPanelOpenedByID(int32 CaravanID)
{
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] ========== NotifyCaravanPanelOpenedByID =========="));
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] CaravanID: %d"), CaravanID);
	
	if (!MainUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] MainUIWidget为空"));
		return;
	}
	
	// 如果城镇面板已打开，更新其商队ID和交易权限
	UTownInfoPanel* TownInfoPanel = MainUIWidget->GetTownInfoPanel();
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] TownInfoPanel: %s"), 
		TownInfoPanel ? TEXT("存在") : TEXT("不存在"));
	
	// 移除IsInViewport()检查，只要面板存在就更新
	if (TownInfoPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] TownInfoPanel存在，继续设置CaravanID"));
		UTownMarketPage* MarketPage = TownInfoPanel->GetMarketPage();
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] MarketPage: %s"), 
			MarketPage ? TEXT("存在") : TEXT("不存在"));
		
		if (MarketPage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UIManager] 调用MarketPage->SetCaravanID(%d)"), CaravanID);
			MarketPage->SetCaravanID(CaravanID);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[UIManager] ❌ MarketPage不存在，无法设置CaravanID"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] TownInfoPanel不存在"));
	}
	
	// 如果商队面板的库存页面存在，设置城镇引用
	UCaravanInfoPanel01* CaravanInfoPanel = MainUIWidget->GetCaravanInfoPanel();
	if (CaravanInfoPanel && CaravanInfoPanel->IsInViewport() && CurrentTown)
	{
		UCaravanInventoryPage01* InventoryPage = CaravanInfoPanel->GetInventoryPage();
		if (InventoryPage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UIManager] 设置商队库存页面的城镇引用"));
			InventoryPage->SetTown(CurrentTown);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] ========== NotifyCaravanPanelOpenedByID 结束 =========="));
}

// ===== 殖民者面板管理 =====

void UDemo01UIManager::OpenColonistInfoPanel(AMoveableEntity01* Colonist)
{
	if (!Colonist)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] 无法打开殖民者面板：殖民者为空"));
		return;
	}
	
	if (!MainUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 主UI容器不存在"));
		return;
	}
	
	// 如果已经打开了同一个殖民者的面板，不重复打开
	if (CurrentColonist == Colonist && MainUIWidget->GetColonistInfoPanel())
	{
		return;
	}
	
	// 统一通过ID初始化，保持架构一致性
	OpenColonistInfoPanelByID(Colonist->EntityID);
	
	// 确保CurrentColonist设置为传入的实例（OpenColonistInfoPanelByID可能设置nullptr）
	CurrentColonist = Colonist;
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 殖民者面板已打开（通过实例转发到ID）：%s"), *Colonist->EntityName);
}

void UDemo01UIManager::OpenColonistInfoPanelByID(int32 ColonistID)
{
	if (ColonistID < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 无法打开殖民者面板：无效的殖民者ID %d"), ColonistID);
		return;
	}
	
	if (!MainUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 主UI容器不存在"));
		return;
	}
	
	// 从PlayerState获取殖民者数据
	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 无法获取PlayerController"));
		return;
	}
	
	ADemo01_PS* PS = PC->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 无法获取PlayerState"));
		return;
	}
	
	// 获取殖民者数据
	FColonistData01 ColonistData = PS->GetColonistDataCopy(ColonistID);
	if (ColonistData.EntityID != ColonistID)
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 找不到殖民者数据：ID %d"), ColonistID);
		return;
	}
	
	// 尝试查找殖民者实例（如果在世界中）
	AMoveableEntity01* ColonistInstance = PS->FindColonistInstance(ColonistID);
	
	// 如果已经打开了同一个殖民者的面板，不重复打开
	if (MainUIWidget->GetColonistInfoPanel() && CurrentColonist && CurrentColonist->EntityID == ColonistID)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] 殖民者面板已打开：%s，跳过"), *ColonistData.EntityName);
		return;
	}
	
	// 显示殖民者面板
	MainUIWidget->ShowColonistInfoPanelByID(ColonistID);
	
	// 设置当前殖民者
	CurrentColonist = ColonistInstance;
	
	// 切换到 GameAndUI 模式
	SetInputMode(EDemo01InputMode::GameAndUI);
	
	// 通知面板已打开
	if (ColonistInstance)
	{
		NotifyColonistPanelOpened(ColonistInstance);
	}
	else
	{
		NotifyColonistPanelOpenedByID(ColonistID);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 殖民者面板已打开：%s (ID: %d)"), 
		*ColonistData.EntityName, ColonistID);
}

void UDemo01UIManager::CloseColonistInfoPanel()
{
	if (!MainUIWidget)
	{
		return;
	}
	
	// 隐藏面板
	MainUIWidget->HideColonistInfoPanel();
	
	// 清除当前殖民者
	CurrentColonist = nullptr;
	
	// 通知面板已关闭
	NotifyColonistPanelClosed();
}

void UDemo01UIManager::CloseColonistInfoPanelByID(int32 ColonistID)
{
	if (!MainUIWidget || ColonistID < 0)
	{
		return;
	}
	
	// 获取当前打开的殖民者面板
	UColonistInfoPanel01* ColonistPanel = MainUIWidget->GetColonistInfoPanel();
	if (!ColonistPanel)
	{
		// 没有打开的殖民者面板，无需处理
		return;
	}
	
	// 检查当前面板显示的殖民者ID是否匹配
	FColonistData01 CurrentData = ColonistPanel->GetColonistData();
	if (CurrentData.EntityID != ColonistID)
	{
		// 当前面板显示的是其他殖民者，不关闭
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 不关闭殖民者面板：当前显示ID %d，请求关闭ID %d"), 
			CurrentData.EntityID, ColonistID);
		return;
	}
	
	// ID匹配，关闭面板
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 关闭殖民者面板：ID %d"), ColonistID);
	MainUIWidget->HideColonistInfoPanel();
	
	// 清除当前殖民者
	CurrentColonist = nullptr;
	
	// 通知面板已关闭
	NotifyColonistPanelClosed();
}

void UDemo01UIManager::RefreshColonistInfoPanelIfOpen(AMoveableEntity01* Colonist)
{
	if (!Colonist || !MainUIWidget)
	{
		return;
	}
	
	UColonistInfoPanel01* ColonistPanel = MainUIWidget->GetColonistInfoPanel();
	if (ColonistPanel && CurrentColonist == Colonist)
	{
		ColonistPanel->RefreshPanel();
	}
}

void UDemo01UIManager::RefreshColonistInfoPanelIfOpen(int32 ColonistID)
{
	if (ColonistID < 0 || !MainUIWidget)
	{
		return;
	}
	
	UColonistInfoPanel01* ColonistPanel = MainUIWidget->GetColonistInfoPanel();
	if (ColonistPanel)
	{
		FColonistData01 CurrentData = ColonistPanel->GetColonistData();
		if (CurrentData.EntityID == ColonistID)
		{
			ColonistPanel->RefreshPanel();
		}
	}
}

void UDemo01UIManager::UpdateColonistFoundCityButtonState(int32 ColonistID)
{
	if (ColonistID < 0 || !MainUIWidget)
	{
		return;
	}
	
	UColonistInfoPanel01* ColonistPanel = MainUIWidget->GetColonistInfoPanel();
	if (ColonistPanel)
	{
		FColonistData01 CurrentData = ColonistPanel->GetColonistData();
		if (CurrentData.EntityID == ColonistID)
		{
			ColonistPanel->UpdateButtonStates();  // 只更新按钮状态，不刷新整个面板
			UE_LOG(LogTemp, Log, TEXT("[UDemo01UIManager] 殖民者 %d 建城按钮状态已更新"), ColonistID);
		}
	}
}

void UDemo01UIManager::UpdateCurrentColonistIfOpen(AMoveableEntity01* Colonist)
{
	if (!Colonist || !MainUIWidget)
	{
		return;
	}
	
	UColonistInfoPanel01* ColonistPanel = MainUIWidget->GetColonistInfoPanel();
	if (!ColonistPanel)
	{
		return;
	}
	
	// 检查面板显示的是否是这个殖民者
	FColonistData01 CurrentData = ColonistPanel->GetColonistData();
	if (CurrentData.EntityID == Colonist->EntityID)
	{
		// 更新CurrentColonist引用
		CurrentColonist = Colonist;
		UE_LOG(LogTemp, Log, TEXT("[UIManager] UpdateCurrentColonistIfOpen: 已更新CurrentColonist引用为 %s (ID: %d)"),
			*Colonist->EntityName, Colonist->EntityID);
	}
}

void UDemo01UIManager::NotifyColonistPanelOpened(AMoveableEntity01* Colonist)
{
	// 可以在这里添加面板打开时的通知逻辑
	// 例如：通知其他系统、更新UI状态等
}

void UDemo01UIManager::NotifyColonistPanelClosed()
{
	// 检查是否还有其他面板打开，如果没有则切换回游戏模式
	if (!HasAnyPanelOpen())
	{
		SetInputMode(EDemo01InputMode::GameOnly);
	}
}

void UDemo01UIManager::NotifyColonistPanelOpenedByID(int32 ColonistID)
{
	// 无实例版本的面板打开通知
	// 可以在这里添加特殊的处理逻辑
}


// ===== 选择变化监听实现 =====

void UDemo01UIManager::OnTownSelectionChanged(int32 NewTownID, int32 OldTownID)
{
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 城镇选择变化：%d -> %d"), OldTownID, NewTownID);

	// 关闭旧城镇面板
	if (OldTownID != -1 && CurrentTownID == OldTownID)
	{
		CloseTownInfoPanel();
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 关闭旧城镇面板：ID=%d"), OldTownID);
	}

	// 更新选择状态
	CurrentTownID = NewTownID;

	// 打开新城镇面板
	if (NewTownID != -1)
	{
		// 通过PlayerState查找城镇实例
		APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			ADemo01_PS* PS = PC->GetPlayerState<ADemo01_PS>();
			if (PS)
			{
				ATownActor01* Town = PS->FindTownInstance(NewTownID);
				if (Town)
				{
					OpenTownInfoPanel(Town);
					UE_LOG(LogTemp, Log, TEXT("[UIManager] 打开新城镇面板：%s (ID=%d)"),
						*Town->TownData.EntityName, NewTownID);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[UIManager] 找不到城镇实例：ID=%d"), NewTownID);
				}
			}
		}
	}
}

void UDemo01UIManager::OnMoveableUnitSelectionChanged(int32 NewUnitID, EEntityType01 NewType,
                                                     int32 OldUnitID, EEntityType01 OldType)
{
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 可移动单位选择变化：Type=%d,ID=%d -> Type=%d,ID=%d"),
		(int32)OldType, OldUnitID, (int32)NewType, NewUnitID);

	// 关闭旧单位面板
	if (OldUnitID != -1 && CurrentMoveableUnitID == OldUnitID && CurrentMoveableUnitType == OldType)
	{
		if (OldType == EEntityType01::Caravan)
		{
			CloseCaravanInfoPanel();
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 关闭旧商队面板：ID=%d"), OldUnitID);
		}
		else if (OldType == EEntityType01::Colonist)
		{
			CloseColonistInfoPanel();
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 关闭旧殖民者面板：ID=%d"), OldUnitID);
		}
	}

	// 更新选择状态
	CurrentMoveableUnitID = NewUnitID;
	CurrentMoveableUnitType = NewType;

	// 打开新单位面板
	if (NewUnitID != -1)
	{
		if (NewType == EEntityType01::Caravan)
		{
			OpenCaravanInfoPanelByID(NewUnitID);
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 打开新商队面板：ID=%d"), NewUnitID);
		}
		else if (NewType == EEntityType01::Colonist)
		{
			OpenColonistInfoPanelByID(NewUnitID);
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 打开新殖民者面板：ID=%d"), NewUnitID);
		}
	}
}

void UDemo01UIManager::OnTownPanelClosedByUser()
{
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 用户主动关闭城镇面板"));
	
	// 清除选择状态
	CurrentTownID = -1;
	
	// 通知PlayerController取消城镇选择
	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		ADemo01_PC* DemoPC = Cast<ADemo01_PC>(PC);
		if (DemoPC)
		{
			DemoPC->ClearTownSelection();
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 已通知PlayerController取消城镇选择"));
		}
	}
	
	// 实际关闭面板（这是之前缺少的部分）
	if (MainUIWidget)
	{
		MainUIWidget->HideTownInfoPanel();
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 已隐藏城镇面板"));
	}

	// 取消订阅库存变化 delegate
	if (CurrentTown)
	{
		CurrentTown->OnInventoryChanged.RemoveAll(this);
	}
	
	// 清除当前城镇引用
	CurrentTown = nullptr;
}

void UDemo01UIManager::OnMoveableUnitPanelClosedByUser()
{
	UE_LOG(LogTemp, Log, TEXT("[UIManager] 用户主动关闭可移动单位面板"));
	
	// 清除选择状态
	CurrentMoveableUnitID = -1;
	CurrentMoveableUnitType = EEntityType01::None;
	
	// 通知PlayerController取消可移动单位选择
	APlayerController* PC = GameMode->GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		ADemo01_PC* DemoPC = Cast<ADemo01_PC>(PC);
		if (DemoPC)
		{
			DemoPC->ClearMoveableUnitSelection();
			UE_LOG(LogTemp, Log, TEXT("[UIManager] 已通知PlayerController取消可移动单位选择"));
		}
	}
	
	// 实际关闭面板（这是之前缺少的部分）
	if (MainUIWidget)
	{
		// 根据当前面板类型隐藏相应面板
		MainUIWidget->HideCaravanInfoPanel();
		MainUIWidget->HideColonistInfoPanel();
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 已隐藏可移动单位面板"));
	}
	
	// 清除当前单位引用
	CurrentCaravan = nullptr;
	CurrentColonist = nullptr;
}

void UDemo01UIManager::OnActiveTownInventoryChanged()
{
	if (CurrentTown)
	{
		RefreshTownInfoPanelIfOpen(CurrentTown);
	}
}

void UDemo01UIManager::InitializeStyleSystem()
{
	// 获取样式管理器实例并初始化
	UDemo01StyleManager* StyleManager = UDemo01StyleManager::GetInstance();
	if (StyleManager)
	{
		UE_LOG(LogTemp, Log, TEXT("[UIManager] 样式系统初始化成功"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] 样式系统初始化失败"));
	}
}

int32 UDemo01UIManager::GetCurrentSelectedCaravanID()
{
	if (!GameMode)
	{
		return -1;
	}
	
	// 通过GameMode获取PlayerController，保持架构解耦
	ADemo01_PC* DemoPC = Cast<ADemo01_PC>(GameMode->GetWorld()->GetFirstPlayerController());
	if (!DemoPC)
	{
		return -1;
	}
	
	// 检查当前选中的单位是否为商队
	if (DemoPC->GetSelectedMoveableUnitType() == EEntityType01::Caravan)
	{
		return DemoPC->GetSelectedMoveableUnitID();
	}
	
	return -1;
}