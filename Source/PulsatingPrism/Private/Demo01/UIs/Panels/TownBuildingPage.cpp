// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/TownBuildingPage.h"

#include "Demo01/UIs/Widgets/BuildingEntryWidget01.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Demo01/UIs/Widgets/RecipeSelectionWidget01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"

void UTownBuildingPage::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定按钮事件
	if (AddBuildingButton)
	{
		AddBuildingButton->OnClicked.AddDynamic(this, &UTownBuildingPage::OnAddBuildingClicked);
	}

	// 绑定配方选择事件
	if (RecipeSelectionWidget)
	{
		RecipeSelectionWidget->OnRecipeSelected.AddDynamic(this, &UTownBuildingPage::OnRecipeSelected);
		RecipeSelectionWidget->SetVisibility(ESlateVisibility::Collapsed); // 默认隐藏
	}

	// 设置页面标题
	if (PageTitleText)
	{
		PageTitleText->SetText(FText::FromString(TEXT("城镇建设")));
	}
	
	// 设置滚动条可见并始终显示（避免样式问题导致滚动条不显示）
	if (BuildingScrollBox)
	{
		BuildingScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
		BuildingScrollBox->SetAlwaysShowScrollbar(true);
	}
}

void UTownBuildingPage::RefreshPage()
{
	Super::RefreshPage();
	
	
	
	// 刷新建筑列表
	RefreshBuildingList();
	
	// 更新统计信息
	UpdateStatistics();
	if (TownInstance != nullptr)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] ========== 开始刷新页面 =========="));
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 城镇: %s, EntityID: %d"), *TownInstance->EntityName, TownInstance->EntityID);

		TArray<FString> AvailableBefore = TownInstance->GetAvailableRecipeIDs();
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] RefreshPage 前 AvailableRecipeIDs 数量: %d"), AvailableBefore.Num());
		TArray<FString> AvailableAfter = TownInstance->GetAvailableRecipeIDs();
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] RefreshPage 后 AvailableRecipeIDs 数量: %d"), AvailableAfter.Num());
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] ========== 刷新页面完成 =========="));
	}
	
}

void UTownBuildingPage::RefreshBuildingList()
{
	if (!BuildingListBox || !TownInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 无法刷新建筑列表 - BuildingListBox: %s, TownInstance: %s"), 
			BuildingListBox ? TEXT("有效") : TEXT("空"), 
			TownInstance ? TEXT("有效") : TEXT("空"));
		return;
	}

	// 清空现有列表
	BuildingListBox->ClearChildren();

	// 获取所有建筑
	TArray<FTownBuildingData01> Buildings = TownInstance->GetAllBuildings();

	// UE_LOG(LogTemp, Log, TEXT("[TownBuildingPage] 城镇建筑数量: %d"), Buildings.Num());

	// 为每个建筑创建条目Widget
	for (const FTownBuildingData01& BuildingData : Buildings)
	{
		UBuildingEntryWidget01* BuildingEntryWidget = CreateBuildingEntryWidget(BuildingData);
		if (BuildingEntryWidget)
		{
			// 1. AddChild，Slate层就绪
			BuildingListBox->AddChild(BuildingEntryWidget);
			// 2. 应用样式
			UE_LOG(LogTemp, Log, TEXT("[TownBuildingPage] 为 %s 应用样式"), *BuildingEntryWidget->GetName());
			IDemo01Styleable::Execute_ApplyWidgetStyle(BuildingEntryWidget);
			// 3. 初始化数据
			BuildingEntryWidget->InitializeBuildingEntry(TownInstance, BuildingData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 创建建筑条目Widget失败: %s"), *BuildingData.BuildingName.ToString());
		}
	}

	// UE_LOG(LogTemp, Log, TEXT("[TownBuildingPage] 建筑列表已刷新，共 %d 个建筑"), Buildings.Num());
}

void UTownBuildingPage::ShowAddBuildingDialog()
{
	if (!RecipeSelectionWidget || !TownInstance)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 无法显示添加建筑对话框 - RecipeSelectionWidget: %s, TownInstance: %s"), 
		// 	RecipeSelectionWidget ? TEXT("有效") : TEXT("空"), 
		// 	TownInstance ? TEXT("有效") : TEXT("空"));
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] ========== 开始显示添加建筑对话框 =========="));
	// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 城镇: %s, EntityID: %d"), *TownInstance->EntityName, TownInstance->EntityID);
	
	TArray<FString> AvailableBefore = TownInstance->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 调用前 AvailableRecipeIDs 数量: %d"), AvailableBefore.Num());

	// 初始化配方选择器
	RecipeSelectionWidget->InitializeRecipeSelection(TownInstance);
	
	// 显示配方选择Widget
	RecipeSelectionWidget->SetVisibility(ESlateVisibility::Visible);

	// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] ========== 显示添加建筑对话框完成 =========="));
}

void UTownBuildingPage::OnAddBuildingClicked()
{
	ShowAddBuildingDialog();
}

void UTownBuildingPage::OnRecipeSelected(const FString& RecipeID)
{
	if (!TownInstance || RecipeID.IsEmpty())
	{
		return;
	}

	// 隐藏配方选择Widget
	if (RecipeSelectionWidget)
	{
		RecipeSelectionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 获取配方数据以生成建筑名称
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[TownBuildingPage] 生产管理器未初始化"));
		return;
	}

	FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RecipeID);
	
	// 生成建筑名称
	FText BuildingName = RecipeData.ProductionFacility.IsEmpty() ? 
		FText::FromString(TEXT("生产建筑")) : RecipeData.ProductionFacility;

	// 添加建筑到城镇
	// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] ========== 开始添加建筑 =========="));
	// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 城镇: %s, 配方: %s"), *TownInstance->EntityName, *RecipeID);
	
	TArray<FString> AvailableBeforeBuild = TownInstance->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 添加建筑前 AvailableRecipeIDs 数量: %d"), AvailableBeforeBuild.Num());
	
	int32 NewBuildingID = TownInstance->AddBuilding(BuildingName, RecipeID);
	
	if (NewBuildingID != -1)
	{
		TArray<FString> AvailableAfterBuild = TownInstance->GetAvailableRecipeIDs();
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 添加建筑后 AvailableRecipeIDs 数量: %d"), AvailableAfterBuild.Num());
		
		// 刷新建筑列表
		RefreshBuildingList();
		
		// 更新统计信息
		UpdateStatistics();

		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] ========== 添加建筑成功 =========="));
		// UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] 成功添加建筑：%s (ID: %d, 配方: %s)"), 
		// 	*BuildingName.ToString(), NewBuildingID, *RecipeID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TownBuildingPage] ========== 添加建筑失败 =========="));
		UE_LOG(LogTemp, Error, TEXT("[TownBuildingPage] 添加建筑失败：配方 %s"), *RecipeID);
	}
}

UBuildingEntryWidget01* UTownBuildingPage::CreateBuildingEntryWidget(const FTownBuildingData01& BuildingData)
{
	if (!BuildingEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TownBuildingPage] BuildingEntryWidgetClass 未设置"));
		return nullptr;
	}

	// 只创建widget，不在这里初始化（AddChild之前GetWorld()不稳定）
	UBuildingEntryWidget01* BuildingEntryWidget = CreateWidget<UBuildingEntryWidget01>(this, BuildingEntryWidgetClass);
	return BuildingEntryWidget;
}

void UTownBuildingPage::UpdateStatistics()
{
	if (!BuildingCountText || !TownInstance)
	{
		return;
	}

	TArray<FTownBuildingData01> Buildings = TownInstance->GetAllBuildings();
	
	// 统计激活的建筑数量
	int32 ActiveBuildings = 0;
	for (const FTownBuildingData01& Building : Buildings)
	{
		if (Building.bIsActive)
		{
			ActiveBuildings++;
		}
	}

	// 更新显示文本
	FText StatText = FText::Format(
		FText::FromString(TEXT("建筑总数: {0} | 运行中: {1}")), 
		FText::AsNumber(Buildings.Num()),
		FText::AsNumber(ActiveBuildings)
	);
	BuildingCountText->SetText(StatText);
}

UProductionManager01* UTownBuildingPage::GetProductionManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetProductionManager() : nullptr;
}
