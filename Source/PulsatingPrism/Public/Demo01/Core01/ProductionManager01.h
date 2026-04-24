// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "ProductionManager01.generated.h"

/**
 * 生产管理器
 * 管理全局生产数据，提供配方查询、生产计算等功能
 */
UCLASS(BlueprintType, Blueprintable)
class PULSATINGPRISM_API UProductionManager01 : public UObject
{
	GENERATED_BODY()

public:
	UProductionManager01();

	// ===== 初始化 =====
	
	// 初始化生产管理器
	UFUNCTION(BlueprintCallable, Category = "Production")
	void Initialize(UDataTable* InProductTable, UDataTable* InRecipeTable);

	// 检查是否已初始化
	UFUNCTION(BlueprintPure, Category = "Production")
	bool IsInitialized() const { return bIsInitialized; }

	// ===== 产品查询 =====
	
	// 获取产品数据
	UFUNCTION(BlueprintPure, Category = "Production|Products")
	FProductData01 GetProductData(const FString& ProductID) const;

	// 检查产品是否存在
	UFUNCTION(BlueprintPure, Category = "Production|Products")
	bool IsValidProduct(const FString& ProductID) const;

	// 获取所有产品ID列表
	UFUNCTION(BlueprintPure, Category = "Production|Products")
	TArray<FString> GetAllProductIDs() const;

	// 按类别获取产品ID列表
	UFUNCTION(BlueprintPure, Category = "Production|Products")
	TArray<FString> GetProductsByCategory(EProductCategory01 Category) const;

	// ===== 配方查询 =====
	
	// 获取配方数据
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	FRecipeData01 GetRecipeData(const FString& RecipeID) const;

	// 检查配方是否存在
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	bool IsValidRecipe(const FString& RecipeID) const;

	// 获取所有配方ID列表
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	TArray<FString> GetAllRecipeIDs() const;

	// 获取指定建筑类型的所有配方ID
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	TArray<FString> GetRecipeIDsByBuildingType(const FText& BuildingType) const;
	
	// 获取所有建筑类型
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	TArray<FText> GetAllBuildingTypes() const;

	// 获取生产指定产品的所有配方
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	TArray<FString> GetRecipesForProduct(const FString& ProductID) const;

	// 获取使用指定设施的所有配方
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	TArray<FString> GetRecipesForFacility(const FText& FacilityName) const;

	// 按ProductLevel获取配方ID列表
	UFUNCTION(BlueprintPure, Category = "Production|Recipes")
	TArray<FString> GetRecipesByProductLevel(int32 ProductLevel) const;

	// ===== 生产计算 =====
	
	// 检查是否有足够资源执行配方
	UFUNCTION(BlueprintPure, Category = "Production|Calculation")
	bool CanExecuteRecipe(const FString& RecipeID, const FTownInventoryData01& Inventory, int32 Multiplier = 1) const;

	// 计算执行配方后的库存变化
	UFUNCTION(BlueprintCallable, Category = "Production|Calculation")
	FTownInventoryData01 CalculateRecipeResult(const FString& RecipeID, const FTownInventoryData01& InputInventory, int32 Multiplier = 1) const;

	// 验证生产链条（检查是否能从原材料生产到目标产品）
	UFUNCTION(BlueprintCallable, Category = "Production|Calculation")
	bool ValidateProductionChain(const FString& TargetProductID, TArray<FString>& OutRequiredRecipes) const;

	// ===== 调试功能 =====
	
	// 打印所有产品信息
	UFUNCTION(BlueprintCallable, Category = "Production|Debug")
	void DebugPrintAllProducts() const;

	// 打印所有配方信息
	UFUNCTION(BlueprintCallable, Category = "Production|Debug")
	void DebugPrintAllRecipes() const;

	// 打印产品的生产链条
	UFUNCTION(BlueprintCallable, Category = "Production|Debug")
	void DebugPrintProductionChain(const FString& ProductID) const;

protected:
	// 是否已初始化
	UPROPERTY()
	bool bIsInitialized = false;

	// 产品数据表引用
	UPROPERTY()
	TObjectPtr<UDataTable> ProductTable = nullptr;

	// 配方数据表引用
	UPROPERTY()
	TObjectPtr<UDataTable> RecipeTable = nullptr;

	// 缓存的产品数据 (ProductID -> ProductData)
	UPROPERTY()
	TMap<FString, FProductData01> ProductCache;

	// 缓存的配方数据 (RecipeID -> RecipeData)
	UPROPERTY()
	TMap<FString, FRecipeData01> RecipeCache;

	// 产品到配方的映射 (ProductID -> Array of RecipeIDs)
	// 注意：不使用UPROPERTY，因为UE反射系统不支持TMap<FString, TArray<FString>>
	TMap<FString, TArray<FString>> ProductToRecipesMap;

	// ===== 内部方法 =====
	
	// 构建缓存数据
	void BuildCaches();

	// 构建产品到配方的映射
	void BuildProductToRecipesMap();
	
	// 临时：添加硬编码的配方数据
	//void AddHardcodedRecipeData();

	// 递归验证生产链条
	bool ValidateProductionChainRecursive(const FString& ProductID, TSet<FString>& VisitedProducts, TArray<FString>& RecipeChain) const;
};