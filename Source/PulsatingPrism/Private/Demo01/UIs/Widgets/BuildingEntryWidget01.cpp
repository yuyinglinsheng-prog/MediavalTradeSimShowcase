// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Widgets/BuildingEntryWidget01.h"

#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/UIs/Panels/TownBuildingPage.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UBuildingEntryWidget01::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮事件
	if (ToggleActiveButton)
	{
		ToggleActiveButton->OnClicked.AddDynamic(this, &UBuildingEntryWidget01::OnToggleActiveClicked);
	}

	if (UpgradeButton)
	{
		UpgradeButton->OnClicked.AddDynamic(this, &UBuildingEntryWidget01::OnUpgradeClicked);
	}

	if (DeleteButton)
	{
		DeleteButton->OnClicked.AddDynamic(this, &UBuildingEntryWidget01::OnDeleteClicked);
	}

	if (ChangeRecipeButton)
	{
		ChangeRecipeButton->OnClicked.AddDynamic(this, &UBuildingEntryWidget01::OnChangeRecipeClicked);
	}


}

void UBuildingEntryWidget01::InitializeBuildingEntry(ATownActor01* InTown, const FTownBuildingData01& InBuildingData)
{
	Town = InTown;
	BuildingData = InBuildingData;

	RefreshDisplay();
}

void UBuildingEntryWidget01::RefreshDisplay()
{
	if (!Town)
	{
		return;
	}

	// 获取最新的建筑数据
	BuildingData = Town->GetBuildingData(BuildingData.BuildingID);

	// 更新建筑名称
	if (BuildingNameText)
	{
		BuildingNameText->SetText(BuildingData.BuildingName);
	}

	// 更新配方名称
	if (RecipeNameText)
	{
		RecipeNameText->SetText(GetRecipeDisplayName());
	}

	// 更新等级
	if (LevelText)
	{
		FText LevelDisplayText = FText::Format(
			FText::FromString(TEXT("等级 {0}")), 
			FText::AsNumber(BuildingData.ProductionLevel)
		);
		LevelText->SetText(LevelDisplayText);
	}

	// 更新状态
	if (StatusText)
	{
		FText StatusDisplayText = BuildingData.bIsActive ? 
			FText::FromString(TEXT("运行中")) : 
			FText::FromString(TEXT("已停用"));
		StatusText->SetText(StatusDisplayText);
	}

	// 更新按钮状态
	UpdateButtonStates();

	// 触发蓝图事件
	OnBuildingDataUpdated();
}

void UBuildingEntryWidget01::OnToggleActiveClicked()
{
	if (!Town)
	{
		return;
	}

	bool bNewActiveState = !BuildingData.bIsActive;
	if (Town->SetBuildingActive(BuildingData.BuildingID, bNewActiveState))
	{
		RefreshDisplay();
		// UE_LOG(LogTemp, Log, TEXT("[BuildingEntry] 建筑 %s 激活状态切换为: %s"), 
		// 	*BuildingData.BuildingName.ToString(), 
		// 	bNewActiveState ? TEXT("激活") : TEXT("停用"));
	}
}

void UBuildingEntryWidget01::OnUpgradeClicked()
{
	if (!Town)
	{
		return;
	}

	// 检查是否已达到最大等级
	const int32 MaxLevel = 5; // 最大建筑等级
	if (BuildingData.ProductionLevel >= MaxLevel)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[BuildingEntry] 建筑 %s 已达到最大等级"), 
		// 	*BuildingData.BuildingName.ToString());
		return;
	}

	// 获取升级成本
	TMap<FString, int32> UpgradeCost = Town->GetBuildingUpgradeCost(BuildingData.BuildingID);
	
	// 检查资源是否足够（显示给玩家）
	bool bCanAfford = true;
	FString CostDescription;
	for (const auto& CostPair : UpgradeCost)
	{
		int32 RequiredAmount = CostPair.Value;
		int32 AvailableAmount = Town->GetResourceAmount(CostPair.Key);
		
		if (!CostDescription.IsEmpty())
		{
			CostDescription += TEXT(", ");
		}
		CostDescription += FString::Printf(TEXT("%s: %d/%d"), *CostPair.Key, AvailableAmount, RequiredAmount);
		
		if (AvailableAmount < RequiredAmount)
		{
			bCanAfford = false;
		}
	}

	if (!bCanAfford)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[BuildingEntry] 建筑 %s 升级失败，资源不足: %s"), 
		// 	*BuildingData.BuildingName.ToString(), *CostDescription);
		return;
	}

	// 执行升级
	if (Town->UpgradeBuilding(BuildingData.BuildingID))
	{
		RefreshDisplay();
		// UE_LOG(LogTemp, Log, TEXT("[BuildingEntry] 建筑 %s 升级成功，消耗: %s"), 
		// 	*BuildingData.BuildingName.ToString(), *CostDescription);
	}
}

void UBuildingEntryWidget01::OnDeleteClicked()
{
	if (!Town)
	{
		// UE_LOG(LogTemp, Error, TEXT("[BuildingEntry] Town为空，无法删除建筑"));
		return;
	}

	// 获取当前建筑数据
	FTownBuildingData01 CurrentBuildingData = Town->GetBuildingData(BuildingData.BuildingID);
	
	// 根据建筑等级决定操作类型
	bool bWillDelete = (CurrentBuildingData.ProductionLevel <= 1);
	
	// 通知蓝图即将进行降级操作（仅用于UI更新，不影响逻辑）
	OnBuildingDowngradeNotify(bWillDelete);
	
	// 直接执行降级操作
	if (Town->DowngradeBuilding(BuildingData.BuildingID))
	{
		if (bWillDelete)
		{
			// UE_LOG(LogTemp, Log, TEXT("[BuildingEntry] 建筑 %s 已删除"), 
			// 	*BuildingData.BuildingName.ToString());
			
			// 通知建筑被删除（触发蓝图事件用于UI更新）
			OnBuildingDeleted();
			
			// 通知父页面更新统计信息
			NotifyParentPageUpdate();
			
			// 从父Widget中移除自己
			RemoveFromParent();
		}
		else
		{
			// UE_LOG(LogTemp, Log, TEXT("[BuildingEntry] 建筑 %s 已降级"), 
			// 	*BuildingData.BuildingName.ToString());
			
			// 刷新显示数据
			RefreshDisplay();
			
			// 通知父页面更新统计信息（降级也会影响激活建筑数量）
			NotifyParentPageUpdate();
		}
	}
	else
	{
		// UE_LOG(LogTemp, Error, TEXT("[BuildingEntry] 建筑降级失败"));
	}
}

void UBuildingEntryWidget01::OnChangeRecipeClicked()
{
	// 触发蓝图事件，让蓝图处理配方选择UI
	OnRequestRecipeSelection();
}

void UBuildingEntryWidget01::UpdateButtonStates()
{
	// 更新激活/停用按钮文本
	if (ToggleActiveButton)
	{
		FText ButtonText = BuildingData.bIsActive ? 
			FText::FromString(TEXT("停用")) : 
			FText::FromString(TEXT("激活"));
		
		// 注意：UButton没有直接设置文本的方法，需要在蓝图中处理
		// 或者使用子Widget包含TextBlock
	}

	// 升级按钮状态
	if (UpgradeButton)
	{
		const int32 MaxLevel = 5;
		bool bCanUpgrade = BuildingData.ProductionLevel < MaxLevel;
		UpgradeButton->SetIsEnabled(bCanUpgrade);
	}
	
	// 降级/删除按钮状态（始终可用）
	if (DeleteButton)
	{
		DeleteButton->SetIsEnabled(true);
		
		// 触发蓝图事件来更新按钮文本
		OnUpdateDeleteButtonText(BuildingData.ProductionLevel <= 1);
	}
}

FText UBuildingEntryWidget01::GetRecipeDisplayName() const
{
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager || !ProductionManager->IsValidRecipe(BuildingData.RecipeID))
	{
		return FText::FromString(TEXT("无效配方"));
	}

	FRecipeData01 RecipeData = ProductionManager->GetRecipeData(BuildingData.RecipeID);
	
	// 如果有别名，显示别名，否则显示配方ID
	if (!RecipeData.AlternativeName.IsEmpty())
	{
		return RecipeData.AlternativeName;
	}
	
	return FText::FromString(RecipeData.RecipeID);
}

UProductionManager01* UBuildingEntryWidget01::GetProductionManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetProductionManager() : nullptr;
}

void UBuildingEntryWidget01::NotifyParentPageUpdate()
{
	// 向上查找父Widget，寻找TownBuildingPage
	UWidget* ParentWidget = GetParent();
	while (ParentWidget)
	{
		UTownBuildingPage* BuildingPage = Cast<UTownBuildingPage>(ParentWidget);
		if (BuildingPage)
		{
			// 找到了TownBuildingPage，调用其UpdateStatistics方法
			BuildingPage->RefreshBuildingList(); // 这会同时调用UpdateStatistics
			return;
		}
		ParentWidget = ParentWidget->GetParent();
	}
}

