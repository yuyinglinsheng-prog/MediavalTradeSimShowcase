// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "RecipeDataProcessor.generated.h"

/**
 * 配方数据处理工具（Editor-only 逻辑，但类本身对所有目标可见）
 * 将 DT_Recipes01 DataTable 中的 InputsStr/OutputsStr 字符串字段
 * 解析并写入对应的 Inputs/Outputs 数组字段，然后持久化保存。
 *
 * 使用方式：
 * 1. Content Browser 右键 → Blueprint Class → 搜索 RecipeDataProcessor 作为父类
 * 2. 双击打开，在 Class Defaults 里把 DT_Recipes01 拖入 RecipeDataTable 槽
 * 3. Details 面板会出现 "Parse And Write Inputs Outputs" 按钮，点击运行
 * 4. 弹窗确认后保存项目，DataTable 数组字段持久化完成
 */
UCLASS(BlueprintType, Blueprintable)
class PULSATINGPRISM_API ARecipeDataProcessor : public AActor
{
	GENERATED_BODY()

public:
	// 目标配方数据表（DT_Recipes01）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<UDataTable> RecipeDataTable = nullptr;

	/**
	 * 解析 InputsStr/OutputsStr 并写入 Inputs/Outputs 数组
	 * 按下此按钮后 DataTable 会被标记为已修改，保存项目即可持久化
	 */
	UFUNCTION(CallInEditor, Category = "Data Tools")
	void ParseAndWriteInputsOutputs();

private:
	// 解析 "resource_id:qty,resource_id2:qty2" 格式的字符串
	static TArray<struct FResourceAmount01> ParseResourceString(const FString& Str);
};
