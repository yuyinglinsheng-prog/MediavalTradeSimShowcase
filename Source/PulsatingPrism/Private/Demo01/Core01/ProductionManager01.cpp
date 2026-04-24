// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/ProductionManager01.h"
#include "Engine/DataTable.h"

UProductionManager01::UProductionManager01()
{
	bIsInitialized = false;
}

void UProductionManager01::Initialize(UDataTable* InProductTable, UDataTable* InRecipeTable)
{
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] === Initialize 开始 ==="));
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] ProductTable: %s"), InProductTable ? TEXT("有效") : TEXT("空"));
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] RecipeTable: %s"), InRecipeTable ? TEXT("有效") : TEXT("空"));
	
	if (!InProductTable || !InRecipeTable)
	{
		// UE_LOG(LogTemp, Error, TEXT("[ProductionManager01] 初始化失败：数据表为空"));
		return;
	}

	ProductTable = InProductTable;
	RecipeTable = InRecipeTable;

	// 验证数据表结构
	if (!ProductTable->GetRowStruct()->IsChildOf(FProductData01::StaticStruct()))
	{
		// UE_LOG(LogTemp, Error, TEXT("[ProductionManager01] 产品数据表结构不匹配"));
		return;
	}

	if (!RecipeTable->GetRowStruct()->IsChildOf(FRecipeData01::StaticStruct()))
	{
		// UE_LOG(LogTemp, Error, TEXT("[ProductionManager01] 配方数据表结构不匹配"));
		return;
	}

	// 构建缓存
	BuildCaches();
	BuildProductToRecipesMap();
	
	// 临时：添加硬编码的配方输入输出数据
	//AddHardcodedRecipeData();

	bIsInitialized = true;

	// UE_LOG(LogTemp, Log, TEXT("[ProductionManager01] 初始化完成：%d 个产品，%d 个配方"), 
	// 	ProductCache.Num(), RecipeCache.Num());
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] === Initialize 结束 ==="));
}

void UProductionManager01::BuildCaches()
{
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] === BuildCaches 开始 ==="));
	ProductCache.Empty();
	RecipeCache.Empty();

	// 构建产品缓存
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 开始构建产品缓存"));
	if (ProductTable)
	{
		TArray<FName> RowNames = ProductTable->GetRowNames();
		// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 产品数据表行数: %d"), RowNames.Num());
		
		for (const FName& RowName : RowNames)
		{
			FProductData01* ProductData = ProductTable->FindRow<FProductData01>(RowName, TEXT(""));
			if (ProductData)
			{
				// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 加载产品: %s, BaseValue: %.2f"), 
				// 	*ProductData->ProductID, (double)ProductData->BaseValue);
				ProductCache.Add(ProductData->ProductID, *ProductData);
			}
			else
			{
				// UE_LOG(LogTemp, Error, TEXT("[ProductionManager01] 无法加载产品行: %s"), *RowName.ToString());
			}
		}
	}
	else
	{
		// UE_LOG(LogTemp, Error, TEXT("[ProductionManager01] ProductTable 为空"));
	}

	// 构建配方缓存
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 开始构建配方缓存"));
	if (RecipeTable)
	{
		TArray<FName> RowNames = RecipeTable->GetRowNames();
		// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 配方数据表行数: %d"), RowNames.Num());
		
		for (const FName& RowName : RowNames)
		{
			FRecipeData01* RecipeData = RecipeTable->FindRow<FRecipeData01>(RowName, TEXT(""));
			if (RecipeData)
			{
				// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 加载配方: %s"), *RecipeData->RecipeID);
				RecipeCache.Add(RecipeData->RecipeID, *RecipeData);
			}
			else
			{
				// UE_LOG(LogTemp, Error, TEXT("[ProductionManager01] 无法加载配方行: %s"), *RowName.ToString());
			}
		}
	}
	else
	{
		// UE_LOG(LogTemp, Error, TEXT("[ProductionManager01] RecipeTable 为空"));
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] === BuildCaches 结束，产品: %d, 配方: %d ==="), 
	// 	ProductCache.Num(), RecipeCache.Num());
}

void UProductionManager01::BuildProductToRecipesMap()
{
	ProductToRecipesMap.Empty();

	for (const auto& RecipePair : RecipeCache)
	{
		const FRecipeData01& Recipe = RecipePair.Value;
		if (!Recipe.ProductID.IsEmpty())
		{
			if (!ProductToRecipesMap.Contains(Recipe.ProductID))
			{
				ProductToRecipesMap.Add(Recipe.ProductID, TArray<FString>());
			}
			ProductToRecipesMap[Recipe.ProductID].Add(Recipe.RecipeID);
		}
	}
}

// ===== 产品查询 =====

FProductData01 UProductionManager01::GetProductData(const FString& ProductID) const
{
	const FProductData01* Found = ProductCache.Find(ProductID);
	return Found ? *Found : FProductData01();
}

bool UProductionManager01::IsValidProduct(const FString& ProductID) const
{
	return ProductCache.Contains(ProductID);
}

TArray<FString> UProductionManager01::GetAllProductIDs() const
{
	TArray<FString> ProductIDs;
	ProductCache.GetKeys(ProductIDs);
	return ProductIDs;
}

TArray<FString> UProductionManager01::GetProductsByCategory(EProductCategory01 Category) const
{
	TArray<FString> Result;
	for (const auto& ProductPair : ProductCache)
	{
		if (ProductPair.Value.Category == Category)
		{
			Result.Add(ProductPair.Key);
		}
	}
	return Result;
}

// ===== 配方查询 =====

FRecipeData01 UProductionManager01::GetRecipeData(const FString& RecipeID) const
{
	const FRecipeData01* Found = RecipeCache.Find(RecipeID);
	return Found ? *Found : FRecipeData01();
}

bool UProductionManager01::IsValidRecipe(const FString& RecipeID) const
{
	return RecipeCache.Contains(RecipeID);
}

TArray<FString> UProductionManager01::GetAllRecipeIDs() const
{
	TArray<FString> RecipeIDs;
	RecipeCache.GetKeys(RecipeIDs);
	return RecipeIDs;
}

TArray<FString> UProductionManager01::GetRecipesForProduct(const FString& ProductID) const
{
	const TArray<FString>* Found = ProductToRecipesMap.Find(ProductID);
	return Found ? *Found : TArray<FString>();
}

TArray<FString> UProductionManager01::GetRecipesForFacility(const FText& FacilityName) const
{
	TArray<FString> Result;
	FString FacilityString = FacilityName.ToString();
	
	for (const auto& RecipePair : RecipeCache)
	{
		if (RecipePair.Value.ProductionFacility.ToString() == FacilityString)
		{
			Result.Add(RecipePair.Key);
		}
	}
	return Result;
}

TArray<FString> UProductionManager01::GetRecipesByProductLevel(int32 ProductLevel) const
{
	TArray<FString> Result;
	for (const auto& Pair : RecipeCache)
	{
		if (Pair.Value.ProductLevel == ProductLevel)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

// ===== 生产计算 =====

bool UProductionManager01::CanExecuteRecipe(const FString& RecipeID, const FTownInventoryData01& Inventory, int32 Multiplier) const
{
	const FRecipeData01* Recipe = RecipeCache.Find(RecipeID);
	if (!Recipe)
	{
		// UE_LOG(LogTemp, Log, TEXT("[ProductionManager01] 配方不存在: %s"), * RecipeID);
		return false;
	}

	// 检查所有输入资源是否足够
	for (const FResourceAmount01& Input : Recipe->Inputs)
	{
		int32 RequiredAmount = Input.Quantity * Multiplier;
		int32 AvailableAmount = Inventory.GetResourceAmount(Input.ResourceID);
		
		if (AvailableAmount < RequiredAmount)
		{
			// UE_LOG(LogTemp, Log, TEXT("[ProductionManager01] 资源不足，无法执行配方: %s"), *RecipeID);
			return false;
		}
	}

	return true;
}

FTownInventoryData01 UProductionManager01::CalculateRecipeResult(const FString& RecipeID, const FTownInventoryData01& InputInventory, int32 Multiplier) const
{
	FTownInventoryData01 ResultInventory = InputInventory;
	
	const FRecipeData01* Recipe = RecipeCache.Find(RecipeID);
	if (!Recipe)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 配方不存在: %s"), *RecipeID);
		return ResultInventory;
	}

	// 检查是否能执行配方
	if (!CanExecuteRecipe(RecipeID, InputInventory, Multiplier))
	{
		// UE_LOG(LogTemp, Warning, TEXT("[ProductionManager01] 资源不足，无法执行配方: %s"), *RecipeID);
		return ResultInventory;
	}

	// 消耗输入资源
	for (const FResourceAmount01& Input : Recipe->Inputs)
	{
		int32 ConsumeAmount = Input.Quantity * Multiplier;
		ResultInventory.ConsumeResource(Input.ResourceID, ConsumeAmount);
	}

	// 添加输出资源
	for (const FResourceAmount01& Output : Recipe->Outputs)
	{
		int32 ProduceAmount = Output.Quantity * Multiplier;
		ResultInventory.AddResource(Output.ResourceID, ProduceAmount);
	}

	return ResultInventory;
}

bool UProductionManager01::ValidateProductionChain(const FString& TargetProductID, TArray<FString>& OutRequiredRecipes) const
{
	OutRequiredRecipes.Empty();
	TSet<FString> VisitedProducts;
	
	return ValidateProductionChainRecursive(TargetProductID, VisitedProducts, OutRequiredRecipes);
}

bool UProductionManager01::ValidateProductionChainRecursive(const FString& ProductID, TSet<FString>& VisitedProducts, TArray<FString>& RecipeChain) const
{
	// 防止循环依赖
	if (VisitedProducts.Contains(ProductID))
	{
		return false;
	}

	// 检查产品是否存在
	const FProductData01* Product = ProductCache.Find(ProductID);
	if (!Product)
	{
		return false;
	}

	// 如果是原材料，不需要进一步验证
	if (Product->bIsRaw)
	{
		return true;
	}

	// 获取生产该产品的配方
	TArray<FString> Recipes = GetRecipesForProduct(ProductID);
	if (Recipes.Num() == 0)
	{
		return false; // 没有配方可以生产该产品
	}

	VisitedProducts.Add(ProductID);

	// 尝试找到一个可行的配方
	for (const FString& RecipeID : Recipes)
	{
		const FRecipeData01* Recipe = RecipeCache.Find(RecipeID);
		if (!Recipe)
		{
			continue;
		}

		bool bCanProduceAllInputs = true;
		TArray<FString> TempRecipeChain = RecipeChain;

		// 递归验证所有输入资源
		for (const FResourceAmount01& Input : Recipe->Inputs)
		{
			if (!ValidateProductionChainRecursive(Input.ResourceID, VisitedProducts, TempRecipeChain))
			{
				bCanProduceAllInputs = false;
				break;
			}
		}

		if (bCanProduceAllInputs)
		{
			// 找到可行的配方
			TempRecipeChain.Add(RecipeID);
			RecipeChain = TempRecipeChain;
			VisitedProducts.Remove(ProductID);
			return true;
		}
	}

	VisitedProducts.Remove(ProductID);
	return false;
}

// ===== 调试功能 =====

void UProductionManager01::DebugPrintAllProducts() const
{
	// UE_LOG(LogTemp, Log, TEXT("[ProductionManager01] === 所有产品 ==="));
	for (const auto& ProductPair : ProductCache)
	{
		const FProductData01& Product = ProductPair.Value;
		FString CategoryStr;
		switch (Product.Category)
		{
		case EProductCategory01::Raw: CategoryStr = TEXT("原材料"); break;
		case EProductCategory01::Processed: CategoryStr = TEXT("加工材料"); break;
		case EProductCategory01::Product: CategoryStr = TEXT("最终产品"); break;
		}

		// UE_LOG(LogTemp, Log, TEXT("  %s: %s (%s) - 价值:%d, 消费:%.2f"), 
		// 	*Product.ProductID, 
		// 	*Product.ProductName.ToString(), 
		// 	*CategoryStr, 
		// 	Product.BaseValue, 
		// 	Product.PopulationConsumption);
	}
}

void UProductionManager01::DebugPrintAllRecipes() const
{
	// UE_LOG(LogTemp, Log, TEXT("[ProductionManager01] === 所有配方 ==="));
	for (const auto& RecipePair : RecipeCache)
	{
		const FRecipeData01& Recipe = RecipePair.Value;
		
		FString InputsStr;
		for (int32 i = 0; i < Recipe.Inputs.Num(); i++)
		{
			if (i > 0) InputsStr += TEXT(", ");
			InputsStr += FString::Printf(TEXT("%s x%d"), *Recipe.Inputs[i].ResourceID, Recipe.Inputs[i].Quantity);
		}

		FString OutputsStr;
		for (int32 i = 0; i < Recipe.Outputs.Num(); i++)
		{
			if (i > 0) OutputsStr += TEXT(", ");
			OutputsStr += FString::Printf(TEXT("%s x%d"), *Recipe.Outputs[i].ResourceID, Recipe.Outputs[i].Quantity);
		}

		// UE_LOG(LogTemp, Log, TEXT("  %s (%s): [%s] -> [%s] @ %s"), 
		// 	*Recipe.RecipeID,
		// 	*Recipe.AlternativeName.ToString(),
		// 	*InputsStr,
		// 	*OutputsStr,
		// 	*Recipe.ProductionFacility.ToString());
	}
}

void UProductionManager01::DebugPrintProductionChain(const FString& ProductID) const
{
	TArray<FString> RequiredRecipes;
	bool bValid = ValidateProductionChain(ProductID, RequiredRecipes);
	
	// UE_LOG(LogTemp, Log, TEXT("[ProductionManager01] === %s 生产链条 ==="), *ProductID);
	// UE_LOG(LogTemp, Log, TEXT("  可生产: %s"), bValid ? TEXT("是") : TEXT("否"));
	
	if (bValid && RequiredRecipes.Num() > 0)
	{
		// UE_LOG(LogTemp, Log, TEXT("  需要配方:"));
		for (const FString& RecipeID : RequiredRecipes)
		{
			// UE_LOG(LogTemp, Log, TEXT("    - %s"), *RecipeID);
		}
	}
}

//void UProductionManager01::AddHardcodedRecipeData()
//{
//	// 临时硬编码配方数据，直到我们有更好的数据导入方案
//	
//	// 基础伐木 - 不需要输入，产出木材
//	if (RecipeCache.Contains(TEXT("basic_logging")))
//	{
//		FRecipeData01& Recipe = RecipeCache[TEXT("basic_logging")];
//		Recipe.Inputs.Empty();
//		Recipe.Outputs.Empty();
//		Recipe.Outputs.Add({TEXT("wood"), 2});
//	}
//	
//	// 基础采石 - 不需要输入，产出石材
//	if (RecipeCache.Contains(TEXT("basic_mining")))
//	{
//		FRecipeData01& Recipe = RecipeCache[TEXT("basic_mining")];
//		Recipe.Inputs.Empty();
//		Recipe.Outputs.Empty();
//		Recipe.Outputs.Add({TEXT("stone"), 2});
//	}
//	
//	// 面包烘焙 - 需要面粉，产出面包
//	if (RecipeCache.Contains(TEXT("bread_baking")))
//	{
//		FRecipeData01& Recipe = RecipeCache[TEXT("bread_baking")];
//		Recipe.Inputs.Empty();
//		Recipe.Outputs.Empty();
//		Recipe.Inputs.Add({TEXT("flour"), 1});
//		Recipe.Outputs.Add({TEXT("bread"), 2});
//	}
//	
//	// 面粉研磨 - 需要谷物，产出面粉
//	if (RecipeCache.Contains(TEXT("flour_milling")))
//	{
//		FRecipeData01& Recipe = RecipeCache[TEXT("flour_milling")];
//		Recipe.Inputs.Empty();
//		Recipe.Outputs.Empty();
//		Recipe.Inputs.Add({TEXT("grain"), 2});
//		Recipe.Outputs.Add({TEXT("flour"), 1});
//	}
//	
//	// 谷物种植 - 不需要输入，产出谷物
//	if (RecipeCache.Contains(TEXT("grain_farming")))
//	{
//		FRecipeData01& Recipe = RecipeCache[TEXT("grain_farming")];
//		Recipe.Inputs.Empty();
//		Recipe.Outputs.Empty();
//		Recipe.Outputs.Add({TEXT("grain"), 3});
//	}
//	
//	// 工具制作 - 需要铁锭和木料，产出工具
//	if (RecipeCache.Contains(TEXT("tool_making")))
//	{
//		FRecipeData01& Recipe = RecipeCache[TEXT("tool_making")];
//		Recipe.Inputs.Empty();
//		Recipe.Outputs.Empty();
//		Recipe.Inputs.Add({TEXT("iron_ingot"), 1});
//		Recipe.Inputs.Add({TEXT("lumber"), 1});
//		Recipe.Outputs.Add({TEXT("tools"), 1});
//	}
//	
//	// UE_LOG(LogTemp, Log, TEXT("[ProductionManager01] 硬编码配方数据已添加"));
//}

// ===== 建筑类型相关方法 =====

TArray<FString> UProductionManager01::GetRecipeIDsByBuildingType(const FText& BuildingType) const
{
	TArray<FString> RecipeIDs;
	
	for (const auto& RecipePair : RecipeCache)
	{
		const FRecipeData01& Recipe = RecipePair.Value;
		if (Recipe.ProductionFacility.EqualTo(BuildingType))
		{
			RecipeIDs.Add(Recipe.RecipeID);
		}
	}
	
	return RecipeIDs;
}

TArray<FText> UProductionManager01::GetAllBuildingTypes() const
{
	TArray<FText> BuildingTypes;
	
	for (const auto& RecipePair : RecipeCache)
	{
		const FRecipeData01& Recipe = RecipePair.Value;
		if (!Recipe.ProductionFacility.IsEmpty())
		{
			// 检查是否已经存在该建筑类型
			bool bAlreadyExists = false;
			for (const FText& ExistingType : BuildingTypes)
			{
				if (ExistingType.EqualTo(Recipe.ProductionFacility))
				{
					bAlreadyExists = true;
					break;
				}
			}
			
			if (!bAlreadyExists)
			{
				BuildingTypes.Add(Recipe.ProductionFacility);
			}
		}
	}
	
	return BuildingTypes;
}