// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Widgets/RecipeSelectionWidget01.h"

#include "Demo01/UIs/Widgets/RecipeEntryWidget01.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Blueprint/UserWidget.h"

void URecipeSelectionWidget01::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮事件
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &URecipeSelectionWidget01::OnConfirmClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &URecipeSelectionWidget01::OnCancelClicked);
	}
	

}

void URecipeSelectionWidget01::InitializeRecipeSelection(ATownActor01* InTown, const FString& InCurrentRecipeID)
{
	Town = InTown;
	OriginalRecipeID = InCurrentRecipeID;
	SelectedRecipeID = InCurrentRecipeID;

	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] ========== 开始初始化配方选择 =========="));
	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] 城镇: %s, EntityID: %d"), *Town->EntityName, Town->EntityID);

	// 设置标题
	if (TitleText)
	{
		FText TitleDisplayText = InCurrentRecipeID.IsEmpty() ? 
			FText::FromString(TEXT("选择生产配方")) : 
			FText::FromString(TEXT("更换生产配方"));
		TitleText->SetText(TitleDisplayText);
	}

	TArray<FString> AvailableBefore = Town->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] InitializeRecipeSelection 前 AvailableRecipeIDs 数量: %d"), AvailableBefore.Num());

	RefreshRecipeList();

	TArray<FString> AvailableAfter = Town->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] InitializeRecipeSelection 后 AvailableRecipeIDs 数量: %d"), AvailableAfter.Num());
	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] ========== 初始化配方选择完成 =========="));
}

void URecipeSelectionWidget01::RefreshRecipeList()
{
	if (!RecipeListBox || !Town)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] RefreshRecipeList 失败 - RecipeListBox: %s, Town: %s"), 
		// 	RecipeListBox ? TEXT("有效") : TEXT("空"), 
		// 	Town ? TEXT("有效") : TEXT("空"));
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] ========== 开始刷新配方列表 =========="));
	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] 城镇: %s, EntityID: %d"), *Town->EntityName, Town->EntityID);

	// 清空现有列表
	RecipeListBox->ClearChildren();

	UProductionManager01* ProductionManager = GetProductionManager();
	if (!ProductionManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[RecipeSelectionWidget] 生产管理器未初始化"));
		return;
	}

	// 获取可建造的配方ID列表
	TArray<FString> AvailableRecipeIDs = Town->GetAvailableRecipeIDs();
	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] 获取到 AvailableRecipeIDs 数量: %d"), AvailableRecipeIDs.Num());

	for (const FString& RecipeID : AvailableRecipeIDs)
	{
		FRecipeData01 RecipeData = ProductionManager->GetRecipeData(RecipeID);
		
		UUserWidget* RecipeEntryWidget = CreateRecipeEntryWidget(RecipeData);
		if (RecipeEntryWidget)
		{
			// 1. AddChild，Slate层就绪
			RecipeListBox->AddChild(RecipeEntryWidget);
			// 2. 应用样式
			UE_LOG(LogTemp, Log, TEXT("[RecipeSelectionWidget] 为 %s 应用样式"), *RecipeEntryWidget->GetName());
			IDemo01Styleable::Execute_ApplyWidgetStyle(RecipeEntryWidget);
			// 3. 初始化数据并绑定事件
			URecipeEntryWidget01* TypedWidget = Cast<URecipeEntryWidget01>(RecipeEntryWidget);
			if (TypedWidget)
			{
				TypedWidget->InitializeRecipeEntry(RecipeData);
				TypedWidget->OnRecipeEntryClicked.AddDynamic(this, &URecipeSelectionWidget01::OnRecipeEntryClicked);
			}
		}
	}

	// 更新选中状态显示
	UpdateSelectionDisplay();

	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] 配方列表已刷新，共 %d 个可建造配方"), AvailableRecipeIDs.Num());
	// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelectionWidget] ========== 刷新配方列表完成 =========="));
}

void URecipeSelectionWidget01::OnConfirmClicked()
{
	if (SelectedRecipeID.IsEmpty())
	{
		// UE_LOG(LogTemp, Warning, TEXT("[RecipeSelection] 未选择配方"));
		return;
	}

	// 触发选择事件
	OnRecipeSelected.Broadcast(SelectedRecipeID);
	OnRecipeSelectionConfirmed(SelectedRecipeID);

	// UE_LOG(LogTemp, Log, TEXT("[RecipeSelection] 确认选择配方: %s"), *SelectedRecipeID);
}

void URecipeSelectionWidget01::OnCancelClicked()
{
	// 恢复原始选择
	SelectedRecipeID = OriginalRecipeID;
	
	// 隐藏配方选择Widget
	SetVisibility(ESlateVisibility::Collapsed);
	
	// 触发取消事件
	OnRecipeSelectionCancelled();

	// UE_LOG(LogTemp, Log, TEXT("[RecipeSelection] 取消配方选择"));
}

void URecipeSelectionWidget01::OnRecipeEntryClicked(const FString& RecipeID)
{
	SelectedRecipeID = RecipeID;
	UpdateSelectionDisplay();

	// UE_LOG(LogTemp, Log, TEXT("[RecipeSelection] 选中配方: %s"), *RecipeID);
}

void URecipeSelectionWidget01::OnGenericButtonClicked()
{
	// 这是一个简化的处理方法，实际项目中应该有更好的方式来传递RecipeID
	// UE_LOG(LogTemp, Log, TEXT("[RecipeSelection] 通用按钮被点击"));
}

UUserWidget* URecipeSelectionWidget01::CreateRecipeEntryWidget(const FRecipeData01& RecipeData)
{
	// 只创建widget，不在这里初始化（AddChild之前GetWorld()不稳定）
	if (RecipeEntryWidgetClass)
	{
		return CreateWidget<UUserWidget>(this, RecipeEntryWidgetClass);
	}
	return CreateWidget<URecipeEntryWidget01>(this);
}

void URecipeSelectionWidget01::UpdateSelectionDisplay()
{
	// 更新所有配方条目的选中状态显示
	if (RecipeListBox)
	{
		for (int32 i = 0; i < RecipeListBox->GetChildrenCount(); i++)
		{
			UWidget* ChildWidget = RecipeListBox->GetChildAt(i);
			URecipeEntryWidget01* RecipeEntry = Cast<URecipeEntryWidget01>(ChildWidget);
			if (RecipeEntry)
			{
				bool bShouldBeSelected = (RecipeEntry->GetRecipeID() == SelectedRecipeID);
				RecipeEntry->SetSelected(bShouldBeSelected);
			}
		}
	}

	// 更新确认按钮状态
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(!SelectedRecipeID.IsEmpty());
	}
}

UProductionManager01* URecipeSelectionWidget01::GetProductionManager() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	return GM ? GM->GetProductionManager() : nullptr;
}
