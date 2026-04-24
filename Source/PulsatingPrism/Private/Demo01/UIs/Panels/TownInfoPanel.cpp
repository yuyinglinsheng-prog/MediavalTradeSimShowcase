// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/TownInfoPanel.h"

#include "Demo01/UIs/Panels/TownInfoPage.h"
#include "Demo01/UIs/Panels/TownBuildingPage.h"
#include "Demo01/UIs/Panels/TownMarketPage.h"
#include "Demo01/UIs/Widgets/UnitEntryWidget01.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Demo01/UIs/Core/Demo01StyleManager.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

void UTownInfoPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定关闭按钮事件
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UTownInfoPanel::OnCloseButtonClicked);
	}
	
	// 绑定标签按钮事件
	if (InfoTabButton)
	{
		InfoTabButton->OnClicked.AddDynamic(this, &UTownInfoPanel::OnInfoTabClicked);
	}
	
	if (BuildingTabButton)
	{
		BuildingTabButton->OnClicked.AddDynamic(this, &UTownInfoPanel::OnBuildingTabClicked);
	}
	
	if (MarketTabButton)
	{
		MarketTabButton->OnClicked.AddDynamic(this, &UTownInfoPanel::OnMarketTabClicked);
	}
	
	// 初始化页面引用
	InitializePageReferences();
	
	// 默认显示第一个标签页
	SwitchToTab(ETownTabType::Info);
}

void UTownInfoPanel::InitializeUI(ADemo01_GM* InGameMode)
{
	// 调用基类实现
	Super::InitializeUI(InGameMode);
	
	// 现在GameMode已设置，可以初始化页面
	if (InfoPage)
	{
		InfoPage->InitializePage(GameMode);
		UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] InfoPage 初始化完成"));
	}
	
	if (BuildingPage)
	{
		BuildingPage->InitializePage(GameMode);
		UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] BuildingPage 初始化完成"));
	}
	
	if (MarketPage)
	{
		MarketPage->InitializePage(GameMode);
		UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] MarketPage 初始化完成"));
	}
}

void UTownInfoPanel::SetTownData(const FTownData01& Data)
{
	TownData = Data;
	
	// 更新基本信息显示
	if (TownNameText)
	{
		TownNameText->SetText(GetTownName());
	}
	
	if (TownIDText)
	{
		TownIDText->SetText(GetTownID());
	}
	
	if (CoordText)
	{
		CoordText->SetText(GetCoord());
	}
	
	if (PopulationText)
	{
		PopulationText->SetText(GetPopulation());
	}
	
	// UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] 城镇数据已设置：%s"), *TownData.EntityName);
	
	// 将数据传递给所有页面
	if (InfoPage)
	{
		InfoPage->SetTownData(Data);
	}
	
	if (BuildingPage)
	{
		BuildingPage->SetTownData(Data);
	}
	
	if (MarketPage)
	{
		MarketPage->SetTownData(Data);
	}
}

void UTownInfoPanel::OnCloseButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] 关闭按钮被点击"));
	
	// 通知UIManager用户主动关闭面板
	if (GameMode && GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->OnTownPanelClosedByUser();
	}
}

FText UTownInfoPanel::GetTownName() const
{
	return FText::FromString(TownData.EntityName);
}

FText UTownInfoPanel::GetTownID() const
{
	return FText::FromString(FString::Printf(TEXT("ID: %d"), TownData.EntityID));
}

FText UTownInfoPanel::GetCoord() const
{
	return FText::FromString(FString::Printf(TEXT("坐标: (%d, %d)"), 
		TownData.GridCoord.X, TownData.GridCoord.Y));
}

FText UTownInfoPanel::GetPopulation() const
{
	return FText::FromString(FString::Printf(TEXT("人口: %d"), TownData.Population));
}

void UTownInfoPanel::SetTownInstanceToPages(ATownActor01* TownInstance)
{
	// 设置城镇实例到所有页面
	if (BuildingPage)
	{
		BuildingPage->SetTownInstance(TownInstance);
	}
	
	if (MarketPage)
	{
		MarketPage->SetTownInstance(TownInstance);
	}
	
	if (InfoPage)
	{
		InfoPage->SetTownInstance(TownInstance);
	}
	
	// UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] 城镇实例已设置到所有页面"));
}

// ===== 标签页控制 =====

void UTownInfoPanel::SwitchToTab(ETownTabType TabType)
{
	CurrentTab = TabType;
	
	if (ContentSwitcher)
	{
		// 切换到对应的页面索引
		int32 TabIndex = static_cast<int32>(TabType);
		ContentSwitcher->SetActiveWidgetIndex(TabIndex);
	}
	
	// 更新按钮状态
	UpdateTabButtonStates();
	
	UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] 切换到标签页: %d"), static_cast<int32>(TabType));
}

void UTownInfoPanel::OnInfoTabClicked()
{
	SwitchToTab(ETownTabType::Info);
}

void UTownInfoPanel::OnBuildingTabClicked()
{
	SwitchToTab(ETownTabType::Building);
}

void UTownInfoPanel::OnMarketTabClicked()
{
	SwitchToTab(ETownTabType::Market);
}

void UTownInfoPanel::UpdateTabButtonStates()
{
	UDemo01StyleManager* StyleManager = UDemo01StyleManager::GetInstance();
	
	// 选中状态用主色，未选中用次色
	auto GetTabColor = [&](bool bActive) -> FLinearColor
	{
		if (StyleManager)
		{
			const FDemo01StyleConfig& Cfg = StyleManager->GetStyleConfig();
			return bActive ? Cfg.Colors.ButtonPrimary : Cfg.Colors.ButtonSecondary;
		}
		// fallback
		return bActive ? FLinearColor(1.0f, 1.0f, 0.5f, 1.0f) : FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
	};

	if (InfoTabButton)
		InfoTabButton->SetBackgroundColor(GetTabColor(CurrentTab == ETownTabType::Info));

	if (BuildingTabButton)
		BuildingTabButton->SetBackgroundColor(GetTabColor(CurrentTab == ETownTabType::Building));

	if (MarketTabButton)
		MarketTabButton->SetBackgroundColor(GetTabColor(CurrentTab == ETownTabType::Market));
}

void UTownInfoPanel::InitializePageReferences()
{
	if (!ContentSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("[TownInfoPanel] ContentSwitcher 未找到"));
		return;
	}
	
	// 获取WidgetSwitcher的子页面引用
	if (ContentSwitcher->GetChildrenCount() >= 3)
	{
		InfoPage = Cast<UTownInfoPage>(ContentSwitcher->GetChildAt(0));
		BuildingPage = Cast<UTownBuildingPage>(ContentSwitcher->GetChildAt(1));
		MarketPage = Cast<UTownMarketPage>(ContentSwitcher->GetChildAt(2));
		
		UE_LOG(LogTemp, Log, TEXT("[TownInfoPanel] 页面引用已获取，等待GameMode设置后初始化"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TownInfoPanel] ContentSwitcher 子页面数量不足，需要3个页面，当前: %d"), ContentSwitcher->GetChildrenCount());
	}
}

// ===== 兼容性方法实现 =====

void UTownInfoPanel::RefreshCaravanList()
{
	// 桥接到InfoPage的刷新方法
	if (InfoPage)
	{
		InfoPage->RefreshCaravanList();
	}
}

void UTownInfoPanel::RefreshColonistList()
{
	// 桥接到InfoPage的刷新方法
	if (InfoPage)
	{
		InfoPage->RefreshColonistList();
	}
}

void UTownInfoPanel::OnCaravanEntryClicked(UUnitEntryWidget01* Entry)
{
	// 桥接到InfoPage的处理方法
	if (InfoPage)
	{
		InfoPage->HandleCaravanEntryClicked(Entry);
	}
}

void UTownInfoPanel::OnColonistEntryClicked(UUnitEntryWidget01* Entry)
{
	// 桥接到InfoPage的处理方法
	if (InfoPage)
	{
		InfoPage->HandleColonistEntryClicked(Entry);
	}
}