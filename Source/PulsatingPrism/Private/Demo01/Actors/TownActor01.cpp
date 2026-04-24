// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"

ATownActor01::ATownActor01()
{
	// 设置默认名称
	EntityName = TEXT("Town");
}

void ATownActor01::BeginPlay()
{
	Super::BeginPlay();
	
	// 城镇是静态的，不自动调整位置
	// 位置应该在spawn时由外部设置
	
	// 根据当前世界位置反向计算网格坐标（如果需要）
	ADemo01_GM* GM = GetDemo01GameMode();
	if (GM)
	{
		GridCoord = GM->WorldToGridCoord(GetActorLocation());
		TownData.GridCoord = GridCoord;
	}
}

void ATownActor01::OnClicked()
{
	Super::OnClicked();
	
	// 通过PlayerController选择城镇
	ADemo01_PC* PC = Cast<ADemo01_PC>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		PC->SelectTown(TownData.EntityID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] 无法获取PlayerController"));
	}
}

// ===== 商队管理 =====

void ATownActor01::AddCaravanByID(int32 CaravanID)
{
	if (CaravanID < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 尝试添加无效商队ID: %d"), *EntityName, CaravanID);
		return;
	}
	
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 无法获取PlayerState"), *EntityName);
		return;
	}
	
	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
	
	if (CurrentData.CaravansInTown.Contains(CaravanID))
	{
		return;
	}
	
	// 添加商队
	CurrentData.CaravansInTown.Add(CaravanID);
	
	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);
	
	// 同步到本地缓存
	TownData.CaravansInTown = CurrentData.CaravansInTown;
}

void ATownActor01::RemoveCaravanByID(int32 CaravanID)
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 无法获取PlayerState"), *EntityName);
		return;
	}
	
	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
	
	// 移除商队
	int32 RemovedCount = CurrentData.CaravansInTown.Remove(CaravanID);
	
	if (RemovedCount > 0)
	{
		// 更新PlayerState
		PS->UpdateTownData(TownData.EntityID, CurrentData);
		
		// 同步到本地缓存
		TownData.CaravansInTown = CurrentData.CaravansInTown;
	}
}

TArray<int32> ATownActor01::GetCaravanIDs() const
{
	// 从PlayerState读取最新数据，而不是使用本地缓存
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		return CurrentData.CaravansInTown;
	}
	
	// 如果无法获取PlayerState，返回本地缓存
	return TownData.CaravansInTown;
}

int32 ATownActor01::GetCaravanCount() const
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		return CurrentData.CaravansInTown.Num();
	}
	
	// 如果无法获取PlayerState，返回本地缓存
	return TownData.CaravansInTown.Num();
}

// ===== 殖民者管理 =====

void ATownActor01::AddColonistByID(int32 ColonistID)
{
	if (ColonistID < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 尝试添加无效殖民者ID: %d"), *EntityName, ColonistID);
		return;
	}
	
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 无法获取PlayerState"), *EntityName);
		return;
	}
	
	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
	
	if (CurrentData.ColonistsInTown.Contains(ColonistID))
	{
		return;
	}
	
	// 添加殖民者
	CurrentData.ColonistsInTown.Add(ColonistID);
	
	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);
	
	// 同步到本地缓存
	TownData.ColonistsInTown = CurrentData.ColonistsInTown;
}

void ATownActor01::RemoveColonistByID(int32 ColonistID)
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 无法获取PlayerState"), *EntityName);
		return;
	}
	
	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
	
	// 移除殖民者
	int32 RemovedCount = CurrentData.ColonistsInTown.Remove(ColonistID);
	
	if (RemovedCount > 0)
	{
		// 更新PlayerState
		PS->UpdateTownData(TownData.EntityID, CurrentData);
		
		// 同步到本地缓存
		TownData.ColonistsInTown = CurrentData.ColonistsInTown;
	}
}

TArray<int32> ATownActor01::GetColonistIDs() const
{
	// 从PlayerState读取最新数据，而不是使用本地缓存
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		return CurrentData.ColonistsInTown;
	}
	
	// 如果无法获取PlayerState，返回本地缓存
	return TownData.ColonistsInTown;
}

int32 ATownActor01::GetColonistCount() const
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		return CurrentData.ColonistsInTown.Num();
	}
	
	// 如果无法获取PlayerState，返回本地缓存
	return TownData.ColonistsInTown.Num();
}

// ===== 生产系统 =====

UProductionManager01* ATownActor01::GetProductionManager() const
{
	ADemo01_GM* GM = GetDemo01GameMode();
	return GM ? GM->GetProductionManager() : nullptr;
}

int32 ATownActor01::AddBuilding(const FText& BuildingName, const FString& RecipeID)
{
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): ========== 开始添加建筑 =========="), *EntityName, TownData.EntityID);
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 尝试添加建筑 - 配方: %s"), *EntityName, TownData.EntityID, *RecipeID);

	// 验证配方是否存在
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager || !ProductionManager->IsValidRecipe(RecipeID))
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s (ID:%d): 无效的配方ID: %s"), *EntityName, TownData.EntityID, *RecipeID);
		return -1;
	}

	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s (ID:%d): 无法获取PlayerState"), *EntityName, TownData.EntityID);
		return -1;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 添加前 AvailableRecipeIDs 数量: %d"), 
	// 	*EntityName, TownData.EntityID, CurrentData.AvailableRecipeIDs.Num());
	// for (int32 i = 0; i < CurrentData.AvailableRecipeIDs.Num(); i++)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [Available%d] %s"), *EntityName, TownData.EntityID, i, *CurrentData.AvailableRecipeIDs[i]);
	// }

	// 检查是否已有相同配方的建筑
	int32 ExistingBuildingID = -1;
	for (const FTownBuildingData01& Building : CurrentData.Buildings)
	{
		if (Building.RecipeID == RecipeID)
		{
			ExistingBuildingID = Building.BuildingID;
			break;
		}
	}

	// 如果已有相同配方的建筑，则扩建它
	if (ExistingBuildingID != -1)
	{
		UE_LOG(LogTemp, Log, TEXT("[城镇] %s (ID:%d): 找到相同配方的建筑 ID:%d，执行扩建操作"), 
			*EntityName, TownData.EntityID, ExistingBuildingID);
		
		bool bSuccess = UpgradeBuilding(ExistingBuildingID);
		
		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("[城镇] %s (ID:%d): 扩建成功，返回现有建筑ID: %d"), 
				*EntityName, TownData.EntityID, ExistingBuildingID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 扩建失败，返回-1"), 
				*EntityName, TownData.EntityID);
		}
		
		return ExistingBuildingID;
	}

	// 获取该配方的建筑类型
	FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RecipeID);
	FString BuildingType = RecipeData.ProductionFacility.ToString();

	// 创建新建筑
	FTownBuildingData01 NewBuilding;
	NewBuilding.BuildingID = NextBuildingID++;
	NewBuilding.BuildingName = BuildingName;
	NewBuilding.RecipeID = RecipeID;
	NewBuilding.bIsActive = false;
	NewBuilding.ProductionLevel = 1;

	// 添加到建筑列表
	CurrentData.Buildings.Add(NewBuilding);

	// 不再移除配方，因为一个配方可以被多次建造
	// 配额控制通过 InitializeAvailableRecipes 中的建筑类型检查来实现

	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);

	// 同步到本地缓存
	TownData = CurrentData;

	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 添加完成 - 建筑已添加，配方保留可用"), 
	// 	*EntityName, TownData.EntityID);
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): ========== 添加建筑完成 =========="), *EntityName, TownData.EntityID);

	return NewBuilding.BuildingID;
}

bool ATownActor01::RemoveBuilding(int32 BuildingID)
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 无法获取PlayerState"), *EntityName);
		return false;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// 查找要删除的建筑，记录其配方ID和建筑类型
	FString RemovedRecipeID;
	FString RemovedBuildingType;
	UProductionManager01* ProductionManager = GetProductionManager();
	
	for (const FTownBuildingData01& Building : CurrentData.Buildings)
	{
		if (Building.BuildingID == BuildingID)
		{
			RemovedRecipeID = Building.RecipeID;
			if (ProductionManager)
			{
				FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RemovedRecipeID);
				RemovedBuildingType = RecipeData.ProductionFacility.ToString();
			}
			break;
		}
	}

	// 查找并移除建筑
	int32 RemovedCount = CurrentData.Buildings.RemoveAll([BuildingID](const FTownBuildingData01& Building)
	{
		return Building.BuildingID == BuildingID;
	});

	if (RemovedCount > 0)
	{
		// 检查是否还有相同建筑类型的其他建筑
		bool bHasOtherSameTypeBuilding = false;
		if (!RemovedBuildingType.IsEmpty() && ProductionManager)
		{
			for (const FTownBuildingData01& Building : CurrentData.Buildings)
			{
				FRecipeData01 RecipeData = ProductionManager->GetRecipeData(Building.RecipeID);
				if (RecipeData.ProductionFacility.ToString() == RemovedBuildingType)
				{
					bHasOtherSameTypeBuilding = true;
					break;
				}
			}
		}

		// 如果没有其他相同类型的建筑，将该建筑类型的所有配方重新加入可建造列表
		if (!bHasOtherSameTypeBuilding && !RemovedBuildingType.IsEmpty() && ProductionManager)
		{
			TArray<FString> AllRecipeIDs = ProductionManager->GetAllRecipeIDs();
			for (const FString& CheckRecipeID : AllRecipeIDs)
			{
				FRecipeData01 CheckRecipeData = ProductionManager->GetRecipeData(CheckRecipeID);
				if (CheckRecipeData.ProductionFacility.ToString() == RemovedBuildingType)
				{
					if (!CurrentData.AvailableRecipeIDs.Contains(CheckRecipeID))
					{
						CurrentData.AvailableRecipeIDs.Add(CheckRecipeID);
					}
				}
			}
		}

		// 更新PlayerState
		PS->UpdateTownData(TownData.EntityID, CurrentData);

		// 同步到本地缓存
		TownData = CurrentData;

		UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 移除建筑 ID:%d，建筑类型 %s %s"), 
			*EntityName, BuildingID, *RemovedBuildingType, 
			bHasOtherSameTypeBuilding ? TEXT("仍有其他建筑") : TEXT("重新可建造"));
		return true;
	}

	return false;
}

FTownBuildingData01 ATownActor01::GetBuildingData(int32 BuildingID) const
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		for (const FTownBuildingData01& Building : CurrentData.Buildings)
		{
			if (Building.BuildingID == BuildingID)
			{
				return Building;
			}
		}
	}

	return FTownBuildingData01();
}

TArray<FTownBuildingData01> ATownActor01::GetAllBuildings() const
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		return CurrentData.Buildings;
	}

	return TownData.Buildings;
}

bool ATownActor01::SetBuildingActive(int32 BuildingID, bool bActive)
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return false;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// 查找并更新建筑
	for (FTownBuildingData01& Building : CurrentData.Buildings)
	{
		if (Building.BuildingID == BuildingID)
		{
			Building.bIsActive = bActive;

			// 更新PlayerState
			PS->UpdateTownData(TownData.EntityID, CurrentData);

			// 同步到本地缓存
			TownData = CurrentData;

			UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 建筑 ID:%d 激活状态: %s"), 
				*EntityName, BuildingID, bActive ? TEXT("激活") : TEXT("停用"));
			return true;
		}
	}

	return false;
}

bool ATownActor01::SetBuildingRecipe(int32 BuildingID, const FString& RecipeID)
{
	// 验证配方是否存在
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager || !ProductionManager->IsValidRecipe(RecipeID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 无效的配方ID: %s"), *EntityName, *RecipeID);
		return false;
	}

	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return false;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// 查找并更新建筑
	for (FTownBuildingData01& Building : CurrentData.Buildings)
	{
		if (Building.BuildingID == BuildingID)
		{
			Building.RecipeID = RecipeID;

			// 更新PlayerState
			PS->UpdateTownData(TownData.EntityID, CurrentData);

			// 同步到本地缓存
			TownData = CurrentData;

			UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 建筑 ID:%d 配方更新为: %s"), 
				*EntityName, BuildingID, *RecipeID);
			return true;
		}
	}

	return false;
}

bool ATownActor01::ExecuteProduction(int32 BuildingID, int32 Multiplier)
{
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 生产管理器未初始化"), *EntityName);
		return false;
	}

	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return false;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// 查找建筑
	FTownBuildingData01* Building = nullptr;
	for (FTownBuildingData01& B : CurrentData.Buildings)
	{
		if (B.BuildingID == BuildingID)
		{
			Building = &B;
			break;
		}
	}

	if (!Building)
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 建筑 ID:%d 不存在"), *EntityName, BuildingID);
		return false;
	}

	if (!Building->bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 建筑 ID:%d 未激活"), *EntityName, BuildingID);
		return false;
	}

	// 检查资源是否足够
	int32 ActualMultiplier = Multiplier * Building->ProductionLevel;
	if (!ProductionManager->CanExecuteRecipe(Building->RecipeID, CurrentData.Inventory, ActualMultiplier))
	{
		//UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 建筑 ID:%d 生产失败，资源不足"), *EntityName, BuildingID);
		return false;
	}

	// 执行生产
	FTownInventoryData01 NewInventory = ProductionManager->CalculateRecipeResult(
		Building->RecipeID, CurrentData.Inventory, ActualMultiplier);

	// 更新库存
	CurrentData.Inventory = NewInventory;

	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);

	// 同步到本地缓存
	TownData = CurrentData;

	// UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 建筑 ID:%d 生产完成 (倍数:%d)"), 
	// 	*EntityName, BuildingID, ActualMultiplier);

	OnInventoryChanged.Broadcast();
	return true;
}

void ATownActor01::ExecuteAllProduction()
{
	TArray<FTownBuildingData01> Buildings = GetAllBuildings();
	int32 SuccessCount = 0;

	for (const FTownBuildingData01& Building : Buildings)
	{
		if (Building.bIsActive)
		{
			if (ExecuteProduction(Building.BuildingID, 1))
			{
				SuccessCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 批量生产完成，成功 %d/%d 个建筑"), 
		*EntityName, SuccessCount, Buildings.Num());
}

// ===== 库存管理 =====

FTownInventoryData01 ATownActor01::GetInventory() const
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		return CurrentData.Inventory;
	}

	return TownData.Inventory;
}

int32 ATownActor01::GetResourceAmount(const FString& ResourceID) const
{
	FTownInventoryData01 Inventory = GetInventory();
	return Inventory.GetResourceAmount(ResourceID);
}

void ATownActor01::AddResource(const FString& ResourceID, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
	CurrentData.Inventory.AddResource(ResourceID, Amount);

	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);

	// 同步到本地缓存
	TownData = CurrentData;

	UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 添加资源 %s x%d"), *EntityName, *ResourceID, Amount);

	OnInventoryChanged.Broadcast();
}

bool ATownActor01::ConsumeResource(const FString& ResourceID, int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return false;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
	bool bSuccess = CurrentData.Inventory.ConsumeResource(ResourceID, Amount);

	if (bSuccess)
	{
		// 更新PlayerState
		PS->UpdateTownData(TownData.EntityID, CurrentData);

		// 同步到本地缓存
		TownData = CurrentData;

		UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 消耗资源 %s x%d"), *EntityName, *ResourceID, Amount);

		OnInventoryChanged.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 资源不足，无法消耗 %s x%d"), *EntityName, *ResourceID, Amount);
	}

	return bSuccess;
}

bool ATownActor01::UpgradeBuilding(int32 BuildingID)
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return false;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// 查找建筑
	FTownBuildingData01* Building = nullptr;
	for (FTownBuildingData01& B : CurrentData.Buildings)
	{
		if (B.BuildingID == BuildingID)
		{
			Building = &B;
			break;
		}
	}

	if (!Building)
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 建筑 ID:%d 不存在"), *EntityName, BuildingID);
		return false;
	}

	// 检查是否已达到最大等级
	const int32 MaxLevel = 5;
	if (Building->ProductionLevel >= MaxLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 建筑 ID:%d 已达到最大等级"), *EntityName, BuildingID);
		return false;
	}

	// 获取升级成本
	TMap<FString, int32> UpgradeCost = GetBuildingUpgradeCost(BuildingID);
	
	// 检查资源是否足够
	for (const auto& CostPair : UpgradeCost)
	{
		int32 RequiredAmount = CostPair.Value;
		int32 AvailableAmount = CurrentData.Inventory.GetResourceAmount(CostPair.Key);
		
		if (AvailableAmount < RequiredAmount)
		{
			UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 升级建筑 ID:%d 失败，%s 不足 (需要:%d, 拥有:%d)"), 
				*EntityName, BuildingID, *CostPair.Key, RequiredAmount, AvailableAmount);
			return false;
		}
	}

	// 消耗升级资源
	for (const auto& CostPair : UpgradeCost)
	{
		CurrentData.Inventory.ConsumeResource(CostPair.Key, CostPair.Value);
	}

	// 升级建筑
	Building->ProductionLevel++;

	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);

	// 同步到本地缓存
	TownData = CurrentData;

	UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 建筑 ID:%d 升级到等级 %d"), 
		*EntityName, BuildingID, Building->ProductionLevel);
	return true;
}

bool ATownActor01::DowngradeBuilding(int32 BuildingID)
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return false;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// 查找建筑
	FTownBuildingData01* Building = nullptr;
	for (FTownBuildingData01& B : CurrentData.Buildings)
	{
		if (B.BuildingID == BuildingID)
		{
			Building = &B;
			break;
		}
	}

	if (!Building)
	{
		UE_LOG(LogTemp, Warning, TEXT("[城镇] %s: 建筑 ID:%d 不存在"), *EntityName, BuildingID);
		return false;
	}

	// 如果建筑等级为1，则删除建筑
	if (Building->ProductionLevel <= 1)
	{
		// 记录配方ID和建筑类型用于重新加入可建造列表
		FString RemovedRecipeID = Building->RecipeID;
		FString RemovedBuildingType;
		
		UProductionManager01* ProductionManager = GetProductionManager();
		if (ProductionManager)
		{
			FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RemovedRecipeID);
			RemovedBuildingType = RecipeData.ProductionFacility.ToString();
		}
		
		// 删除建筑
		CurrentData.Buildings.RemoveAll([BuildingID](const FTownBuildingData01& B)
		{
			return B.BuildingID == BuildingID;
		});
		
		// 检查是否还有相同建筑类型的其他建筑
		bool bHasOtherSameTypeBuilding = false;
		if (!RemovedBuildingType.IsEmpty() && ProductionManager)
		{
			for (const FTownBuildingData01& CheckBuilding : CurrentData.Buildings)
			{
				FRecipeData01 RecipeData = ProductionManager->GetRecipeData(CheckBuilding.RecipeID);
				if (RecipeData.ProductionFacility.ToString() == RemovedBuildingType)
				{
					bHasOtherSameTypeBuilding = true;
					break;
				}
			}
		}

		// 如果没有其他相同类型的建筑，将该建筑类型的所有配方重新加入可建造列表
		if (!bHasOtherSameTypeBuilding && !RemovedBuildingType.IsEmpty() && ProductionManager)
		{
			TArray<FString> AllRecipeIDs = ProductionManager->GetAllRecipeIDs();
			for (const FString& CheckRecipeID : AllRecipeIDs)
			{
				FRecipeData01 CheckRecipeData = ProductionManager->GetRecipeData(CheckRecipeID);
				if (CheckRecipeData.ProductionFacility.ToString() == RemovedBuildingType)
				{
					if (!CurrentData.AvailableRecipeIDs.Contains(CheckRecipeID))
					{
						CurrentData.AvailableRecipeIDs.Add(CheckRecipeID);
					}
				}
			}
		}
		
		// 更新PlayerState
		PS->UpdateTownData(TownData.EntityID, CurrentData);
		
		// 同步到本地缓存
		TownData = CurrentData;
		
		UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 建筑 ID:%d 已删除，建筑类型 %s %s"), 
			*EntityName, BuildingID, *RemovedBuildingType,
			bHasOtherSameTypeBuilding ? TEXT("仍有其他建筑") : TEXT("重新可建造"));
		return true;
	}
	else
	{
		// 降级建筑（等级-1）
		Building->ProductionLevel--;
		
		// 更新PlayerState
		PS->UpdateTownData(TownData.EntityID, CurrentData);
		
		// 同步到本地缓存
		TownData = CurrentData;
		
		UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 建筑 ID:%d 降级到等级 %d"), 
			*EntityName, BuildingID, Building->ProductionLevel);
		return true;
	}
}

TMap<FString, int32> ATownActor01::GetBuildingUpgradeCost(int32 BuildingID) const
{
	TMap<FString, int32> UpgradeCost;

	// 获取建筑数据
	FTownBuildingData01 BuildingData = GetBuildingData(BuildingID);
	if (BuildingData.BuildingID == -1)
	{
		return UpgradeCost; // 建筑不存在，返回空成本
	}

	// 简化的升级成本计算：每级需要更多资源
	int32 NextLevel = BuildingData.ProductionLevel + 1;
	int32 BaseCost = NextLevel * 10; // 基础成本随等级增加

	// 根据建筑类型设置不同的升级材料
	// 这里简化处理，所有建筑都需要石材和木材
	UpgradeCost.Add(TEXT("stone"), BaseCost);
	UpgradeCost.Add(TEXT("wood"), BaseCost);

	// 高级建筑还需要铁锭
	if (NextLevel >= 3)
	{
		UpgradeCost.Add(TEXT("iron_ingot"), BaseCost / 2);
	}

	return UpgradeCost;
}

void ATownActor01::SetResourceAmount(const FString& ResourceID, int32 Amount)
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		return;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
	CurrentData.Inventory.SetResourceAmount(ResourceID, Amount);

	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);

	// 同步到本地缓存
	TownData = CurrentData;

	UE_LOG(LogTemp, Log, TEXT("[城镇] %s: 设置资源 %s = %d"), *EntityName, *ResourceID, Amount);

	OnInventoryChanged.Broadcast();
}

TArray<FString> ATownActor01::GetAllResourceIDs() const
{
	FTownInventoryData01 Inventory = GetInventory();
	TArray<FString> ResourceIDs;
	Inventory.Resources.GetKeys(ResourceIDs);
	return ResourceIDs;
}

// ===== 建筑配方管理 =====

TArray<FString> ATownActor01::GetAvailableRecipeIDs() const
{
	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);
		// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): GetAvailableRecipeIDs - 从 PlayerState 读取, 数量: %d"), 
		// 	*EntityName, TownData.EntityID, CurrentData.AvailableRecipeIDs.Num());
		// for (int32 i = 0; i < CurrentData.AvailableRecipeIDs.Num(); i++)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [配方%d] %s"), *EntityName, TownData.EntityID, i, *CurrentData.AvailableRecipeIDs[i]);
		// }
		// if (CurrentData.AvailableRecipeIDs.Num() == 0)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 警告 - 没有可用配方"), *EntityName, TownData.EntityID);
		// }
		return CurrentData.AvailableRecipeIDs;
	}

	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): GetAvailableRecipeIDs - 从本地缓存读取, 数量: %d"), 
	// 	*EntityName, TownData.EntityID, TownData.AvailableRecipeIDs.Num());
	return TownData.AvailableRecipeIDs;
}

TArray<FString> ATownActor01::GetBuiltBuildingTypes() const
{
	TArray<FString> BuiltBuildingTypes;
	TArray<FTownBuildingData01> Buildings = GetAllBuildings();
	
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): GetAllBuildings 返回建筑数量: %d"), *EntityName, TownData.EntityID, Buildings.Num());
	// for (int32 i = 0; i < Buildings.Num(); ++i)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [建筑%d] ID:%d, RecipeID:%s, BuildingName:%s"), 
	// 		*EntityName, TownData.EntityID, i, Buildings[i].BuildingID, *Buildings[i].RecipeID, *Buildings[i].BuildingName.ToString());
	// }
	
	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		return BuiltBuildingTypes;
	}
	
	for (const FTownBuildingData01& Building : Buildings)
	{
		if (!Building.RecipeID.IsEmpty())
		{
			// 通过配方ID获取建筑类型
			FRecipeData01 RecipeData = ProductionManager->GetRecipeData(Building.RecipeID);
			FString BuildingType = RecipeData.ProductionFacility.ToString();
			
			// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 配方 %s → 建筑类型 %s"), *EntityName, TownData.EntityID, *Building.RecipeID, *BuildingType);
			
			if (!BuildingType.IsEmpty() && !BuiltBuildingTypes.Contains(BuildingType))
			{
				BuiltBuildingTypes.Add(BuildingType);
			}
		}
	}
	
	return BuiltBuildingTypes;
}

TArray<FString> ATownActor01::GetBuiltRecipeIDs() const
{
	TArray<FString> BuiltRecipeIDs;
	TArray<FTownBuildingData01> Buildings = GetAllBuildings();
	
	for (const FTownBuildingData01& Building : Buildings)
	{
		if (!Building.RecipeID.IsEmpty() && !BuiltRecipeIDs.Contains(Building.RecipeID))
		{
			BuiltRecipeIDs.Add(Building.RecipeID);
		}
	}
	
	return BuiltRecipeIDs;
}

void ATownActor01::InitializeAvailableRecipes()
{
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): ========== 开始初始化可建造配方 =========="), *EntityName, TownData.EntityID);

	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 无法初始化可建造配方 - 生产管理器未初始化"), *EntityName);
		return;
	}

	// 从PlayerState读取最新数据
	ADemo01_PS* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[城镇] %s: 无法获取PlayerState"), *EntityName);
		return;
	}

	FTownData01 CurrentData = PS->GetTownData(TownData.EntityID);

	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 初始化前 AvailableRecipeIDs 数量: %d, NativeRecipeIDs 数量: %d"), 
	// 	*EntityName, TownData.EntityID, CurrentData.AvailableRecipeIDs.Num(), CurrentData.NativeRecipeIDs.Num());

	// 获取所有配方ID
	TArray<FString> AllRecipeIDs = ProductionManager->GetAllRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 所有配方数量: %d"), *EntityName, TownData.EntityID, AllRecipeIDs.Num());
	
	// 获取已建造的建筑类型
	TArray<FString> BuiltBuildingTypes = GetBuiltBuildingTypes();
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): 已建造建筑类型数量: %d"), *EntityName, TownData.EntityID, BuiltBuildingTypes.Num());
	// for (int32 i = 0; i < BuiltBuildingTypes.Num(); ++i)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [已建造建筑类型%d] %s"), *EntityName, TownData.EntityID, i, *BuiltBuildingTypes[i]);
	// }
	
	// 初始化可建造配方列表
	CurrentData.AvailableRecipeIDs.Empty();
	int32 AddedCount = 0;
	int32 SkippedByType = 0;
	int32 SkippedByNative = 0;
	for (const FString& RecipeID : AllRecipeIDs)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): [配方循环%d] 开始处理配方: %s"), *EntityName, TownData.EntityID, AddedCount + SkippedByType + SkippedByNative, *RecipeID);
		
		FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RecipeID);
		FString BuildingType = RecipeData.ProductionFacility.ToString();

		// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   配方信息 - 类型:%s, ProductLevel:%d, 是否Native:%d"), 
		// 	*EntityName, TownData.EntityID, *BuildingType, RecipeData.ProductLevel, CurrentData.NativeRecipeIDs.Contains(RecipeID) ? 1 : 0);

		if (BuildingType.IsEmpty())
		{
			// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   跳过 - 建筑类型为空"), *EntityName, TownData.EntityID);
			continue;
		}

		// 同类型建筑已建造则跳过
		if (BuiltBuildingTypes.Contains(BuildingType))
		{
			// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   跳过 - 类型已存在: %s"), *EntityName, TownData.EntityID, *BuildingType);
			SkippedByType++;
			continue;
		}

		// ProductLevel==0：必须在本城镇的天然配方白名单中
		if (RecipeData.ProductLevel == 0 && !CurrentData.NativeRecipeIDs.Contains(RecipeID))
		{
			// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   跳过 - 资源建筑且不在Native白名单中"), *EntityName, TownData.EntityID);
			SkippedByNative++;
			continue;
		}

		// ProductLevel>0：所有城镇都可以建造，不需要解锁
		// （用户要求：只要不是资源建筑就是所有城镇都可以建造的）

		CurrentData.AvailableRecipeIDs.Add(RecipeID);
		AddedCount++;
		// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   添加成功 - AvailableRecipeIDs 现在有 %d 个"), *EntityName, TownData.EntityID, AddedCount);
	}

	// 更新PlayerState
	PS->UpdateTownData(TownData.EntityID, CurrentData);

	// 同步到本地缓存
	TownData = CurrentData;

	// Debug: 打印 UpdateTownData 时的 CurrentData 情况
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): UpdateTownData - AvailableRecipeIDs 数量: %d, NativeRecipeIDs 数量: %d"), 
	// 	*EntityName, TownData.EntityID, CurrentData.AvailableRecipeIDs.Num(), CurrentData.NativeRecipeIDs.Num());
	// for (int32 i = 0; i < CurrentData.AvailableRecipeIDs.Num(); i++)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [AvailableRecipeIDs%d] %s"), *EntityName, TownData.EntityID, i, *CurrentData.AvailableRecipeIDs[i]);
	// }
	// for (int32 i = 0; i < CurrentData.NativeRecipeIDs.Num(); i++)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [NativeRecipeIDs%d] %s"), *EntityName, TownData.EntityID, i, *CurrentData.NativeRecipeIDs[i]);
	// }
	// for (int32 i = 0; i < AllRecipeIDs.Num(); i++)
	// {
	// 	FRecipeData01 RecipeData = ProductionManager->GetRecipeData(AllRecipeIDs[i]);
	// 	FString BuildingType = RecipeData.ProductionFacility.ToString();
	// 	bool bSkippedByType = BuiltBuildingTypes.Contains(BuildingType);
	// 	bool bSkippedByNative = (RecipeData.ProductLevel == 0 && !CurrentData.NativeRecipeIDs.Contains(AllRecipeIDs[i]));
	// 	UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [配方%d] %s - 类型:%s, 跳过(类型已存在):%d, 跳过(非Native):%d"), 
	// 		*EntityName, TownData.EntityID, i, *AllRecipeIDs[i], *BuildingType, bSkippedByType, bSkippedByNative);
	// }

	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d):   [可用配方%d] %s"), *EntityName, TownData.EntityID, i, *CurrentData.AvailableRecipeIDs[i]);
	// UE_LOG(LogTemp, Warning, TEXT("[城镇] %s (ID:%d): ========== 初始化可建造配方完成 =========="), *EntityName, TownData.EntityID);
}

