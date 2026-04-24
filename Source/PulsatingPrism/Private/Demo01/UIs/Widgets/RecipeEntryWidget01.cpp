// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Widgets/RecipeEntryWidget01.h"

#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void URecipeEntryWidget01::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮事件
	if (RecipeButton)
	{
		RecipeButton->OnClicked.AddDynamic(this, &URecipeEntryWidget01::OnButtonClicked);
	}


}

void URecipeEntryWidget01::InitializeRecipeEntry(const FRecipeData01& InRecipeData)
{
	RecipeData = InRecipeData;

	// 更新UI显示
	if (RecipeNameText)
	{
		FText DisplayName = RecipeData.AlternativeName.IsEmpty() ? 
			FText::FromString(RecipeData.RecipeID) : RecipeData.AlternativeName;
		RecipeNameText->SetText(DisplayName);
	}

	if (RecipeDescriptionText)
	{
		// 构建输入输出描述
		FString Description;
		
		// 输入资源
		if (RecipeData.Inputs.Num() > 0)
		{
			Description += TEXT("输入: ");
			for (int32 i = 0; i < RecipeData.Inputs.Num(); i++)
			{
				if (i > 0) Description += TEXT(", ");
				Description += FString::Printf(TEXT("%s x%d"), 
					*RecipeData.Inputs[i].ResourceID, RecipeData.Inputs[i].Quantity);
			}
		}
		else
		{
			Description += TEXT("输入: 无");
		}

		Description += TEXT(" → ");

		// 输出资源
		if (RecipeData.Outputs.Num() > 0)
		{
			Description += TEXT("输出: ");
			for (int32 i = 0; i < RecipeData.Outputs.Num(); i++)
			{
				if (i > 0) Description += TEXT(", ");
				Description += FString::Printf(TEXT("%s x%d"), 
					*RecipeData.Outputs[i].ResourceID, RecipeData.Outputs[i].Quantity);
			}
		}
		else
		{
			Description += TEXT("输出: 无");
		}

		RecipeDescriptionText->SetText(FText::FromString(Description));
	}

	if (FacilityText)
	{
		FacilityText->SetText(RecipeData.ProductionFacility);
	}

	// 触发蓝图事件
	OnRecipeDataUpdated();

	// UE_LOG(LogTemp, Log, TEXT("[RecipeEntry] 初始化配方条目: %s"), *RecipeData.RecipeID);
}

void URecipeEntryWidget01::SetSelected(bool bInSelected)
{
	if (bIsSelected != bInSelected)
	{
		bIsSelected = bInSelected;
		OnSelectionChanged(bIsSelected);
		
		// 样式会通过基类自动应用，选中状态通过蓝图事件处理
	}
}

void URecipeEntryWidget01::OnButtonClicked()
{
	// 触发点击事件
	OnRecipeEntryClicked.Broadcast(RecipeData.RecipeID);

	// UE_LOG(LogTemp, Log, TEXT("[RecipeEntry] 配方条目被点击: %s"), *RecipeData.RecipeID);
}
