// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Tools/RecipeDataProcessor.h"

#include "Demo01/Core01/Demo01DataTypes.h"
#include "Misc/MessageDialog.h"

void ARecipeDataProcessor::ParseAndWriteInputsOutputs()
{
	if (!RecipeDataTable)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::FromString(TEXT("请先在 RecipeDataTable 槽中指定数据表资产！")));
		return;
	}

	TArray<FName> RowNames = RecipeDataTable->GetRowNames();
	if (RowNames.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::FromString(TEXT("DataTable 为空，没有可处理的行。")));
		return;
	}

	int32 SuccessCount = 0;
	int32 SkipCount = 0;
	TArray<FString> Errors;

	for (const FName& RowName : RowNames)
	{
		FRecipeData01* Row = RecipeDataTable->FindRow<FRecipeData01>(RowName, TEXT("RecipeDataProcessor"));
		if (!Row)
		{
			Errors.Add(FString::Printf(TEXT("行 '%s' 无法读取（结构体不匹配？）"), *RowName.ToString()));
			continue;
		}

		if (Row->InputsStr.IsEmpty() && Row->OutputsStr.IsEmpty())
		{
			SkipCount++;
			continue;
		}

		Row->Inputs = ParseResourceString(Row->InputsStr);
		Row->Outputs = ParseResourceString(Row->OutputsStr);
		SuccessCount++;
	}

	// 标记资产为已修改，保存项目时会持久化
	RecipeDataTable->MarkPackageDirty();

	// 汇报结果
	FString Summary = FString::Printf(
		TEXT("处理完成！\n成功：%d 行\n跳过（无字符串）：%d 行"),
		SuccessCount, SkipCount);

	if (Errors.Num() > 0)
	{
		Summary += FString::Printf(TEXT("\n\n错误 (%d 行):\n"), Errors.Num());
		for (const FString& Err : Errors)
		{
			Summary += TEXT("  - ") + Err + TEXT("\n");
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[RecipeDataProcessor] %s"), *Summary);
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Summary));
}

TArray<FResourceAmount01> ARecipeDataProcessor::ParseResourceString(const FString& Str)
{
	TArray<FResourceAmount01> Result;

	if (Str.IsEmpty())
	{
		return Result;
	}

	TArray<FString> Items;
	Str.ParseIntoArray(Items, TEXT(","), true);

	for (const FString& Item : Items)
	{
		FString Trimmed = Item.TrimStartAndEnd();

		TArray<FString> Parts;
		Trimmed.ParseIntoArray(Parts, TEXT(":"), true);

		if (Parts.Num() == 2)
		{
			FResourceAmount01 Amount;
			Amount.ResourceID = Parts[0].TrimStartAndEnd();
			Amount.Quantity = FCString::Atoi(*Parts[1].TrimStartAndEnd());

			if (!Amount.ResourceID.IsEmpty() && Amount.Quantity > 0)
			{
				Result.Add(Amount);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[RecipeDataProcessor] 跳过无效条目: '%s'（ResourceID空或Quantity<=0）"),
					*Trimmed);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[RecipeDataProcessor] 格式错误，无法解析: '%s'（期望 'resource_id:qty'）"),
				*Trimmed);
		}
	}

	return Result;
}

