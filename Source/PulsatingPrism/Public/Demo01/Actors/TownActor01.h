// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/Actors/SelectableEntity01.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "TownActor01.generated.h"

/** 库存变化通知（AddResource / ConsumeResource / SetResourceAmount 后触发） */
DECLARE_MULTICAST_DELEGATE(FOnTownInventoryChanged);

/**
 * 城镇 Actor
 * 表示地图上的城镇，可以被点击查看信息，也可以作为商队的移动目标
 */
UCLASS()
class PULSATINGPRISM_API ATownActor01 : public ASelectableEntity01
{
	GENERATED_BODY()
	
public:
	ATownActor01();
	
	// 城镇数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town")
	FTownData01 TownData;
	
	// 获取城镇数据
	UFUNCTION(BlueprintPure, Category = "Town")
	FTownData01 GetTownData() const { return TownData; }

	// 库存变化 delegate（AddResource / ConsumeResource / SetResourceAmount 后广播）
	FOnTownInventoryChanged OnInventoryChanged;

	// ===== 商队管理 =====
	
	// 商队进入城镇（通过ID）
	UFUNCTION(BlueprintCallable, Category = "Town|Caravans")
	void AddCaravanByID(int32 CaravanID);
	
	// 商队离开城镇（通过ID）
	UFUNCTION(BlueprintCallable, Category = "Town|Caravans")
	void RemoveCaravanByID(int32 CaravanID);
	
	// 获取停留商队ID列表
	UFUNCTION(BlueprintPure, Category = "Town|Caravans")
	TArray<int32> GetCaravanIDs() const;
	
	// 获取停留商队数量
	UFUNCTION(BlueprintPure, Category = "Town|Caravans")
	int32 GetCaravanCount() const;
	
	// ===== 殖民者管理 =====
	
	// 殖民者进入城镇（通过ID）
	UFUNCTION(BlueprintCallable, Category = "Town|Colonists")
	void AddColonistByID(int32 ColonistID);
	
	// 殖民者离开城镇（通过ID）
	UFUNCTION(BlueprintCallable, Category = "Town|Colonists")
	void RemoveColonistByID(int32 ColonistID);
	
	// 获取停留殖民者ID列表
	UFUNCTION(BlueprintPure, Category = "Town|Colonists")
	TArray<int32> GetColonistIDs() const;
	
	// 获取停留殖民者数量
	UFUNCTION(BlueprintPure, Category = "Town|Colonists")
	int32 GetColonistCount() const;
	
	// ===== 生产系统 =====
	
	// 添加建筑
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	int32 AddBuilding(const FText& BuildingName, const FString& RecipeID);
	
	// 移除建筑
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	bool RemoveBuilding(int32 BuildingID);
	
	// 获取建筑数据
	UFUNCTION(BlueprintPure, Category = "Town|Production")
	FTownBuildingData01 GetBuildingData(int32 BuildingID) const;
	
	// 获取所有建筑
	UFUNCTION(BlueprintPure, Category = "Town|Production")
	TArray<FTownBuildingData01> GetAllBuildings() const;
	
	// 设置建筑激活状态
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	bool SetBuildingActive(int32 BuildingID, bool bActive);
	
	// 设置建筑配方
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	bool SetBuildingRecipe(int32 BuildingID, const FString& RecipeID);
	
	// 升级建筑
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	bool UpgradeBuilding(int32 BuildingID);
	
	// 降级建筑（等级-1，等级为1时删除建筑）
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	bool DowngradeBuilding(int32 BuildingID);
	
	// 获取建筑升级成本
	UFUNCTION(BlueprintPure, Category = "Town|Production")
	TMap<FString, int32> GetBuildingUpgradeCost(int32 BuildingID) const;
	
	// 执行单次生产（手动触发）
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	bool ExecuteProduction(int32 BuildingID, int32 Multiplier = 1);
	
	// 执行所有激活建筑的生产
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	void ExecuteAllProduction();
	
	// ===== 库存管理 =====
	
	// 获取库存数据
	UFUNCTION(BlueprintPure, Category = "Town|Inventory")
	FTownInventoryData01 GetInventory() const;
	
	// 获取指定资源数量
	UFUNCTION(BlueprintPure, Category = "Town|Inventory")
	int32 GetResourceAmount(const FString& ResourceID) const;
	
	// 添加资源
	UFUNCTION(BlueprintCallable, Category = "Town|Inventory")
	void AddResource(const FString& ResourceID, int32 Amount);
	
	// 消耗资源
	UFUNCTION(BlueprintCallable, Category = "Town|Inventory")
	bool ConsumeResource(const FString& ResourceID, int32 Amount);
	
	// 设置资源数量
	UFUNCTION(BlueprintCallable, Category = "Town|Inventory")
	void SetResourceAmount(const FString& ResourceID, int32 Amount);
	
	// 获取所有资源列表
	UFUNCTION(BlueprintPure, Category = "Town|Inventory")
	TArray<FString> GetAllResourceIDs() const;
	
	// ===== 建筑配方管理 =====
	
	// 获取可建造的配方ID列表（未建造的建筑）
	UFUNCTION(BlueprintPure, Category = "Town|Production")
	TArray<FString> GetAvailableRecipeIDs() const;
	
	// 获取已建造的建筑类型列表
	UFUNCTION(BlueprintPure, Category = "Town|Production")
	TArray<FString> GetBuiltBuildingTypes() const;
	
	// 获取已建造的配方ID列表
	UFUNCTION(BlueprintPure, Category = "Town|Production")
	TArray<FString> GetBuiltRecipeIDs() const;
	
	// 初始化可建造配方列表（在城镇创建时调用）
	UFUNCTION(BlueprintCallable, Category = "Town|Production")
	void InitializeAvailableRecipes();
	
protected:
	virtual void BeginPlay() override;
	virtual void OnClicked() override;
	
	// 下一个建筑ID（用于生成唯一ID）
	UPROPERTY()
	int32 NextBuildingID = 1;
	
	// 可建造的配方ID列表（未建造的建筑）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town|Production")
	TArray<FString> AvailableRecipeIDs;
	
	// 获取生产管理器引用
	class UProductionManager01* GetProductionManager() const;
};
