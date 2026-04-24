// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/TownInfoPage.h"
#include "Demo01/UIs/Widgets/UnitEntryWidget01.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Demo01/UIs/Core/Demo01UIManager.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"

void UTownInfoPage::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定购买按钮事件
	if (BuyCaravanButton)
	{
		BuyCaravanButton->OnClicked.AddDynamic(this, &UTownInfoPage::OnBuyCaravanButtonClicked);
	}
	
	if (BuyColonistButton)
	{
		BuyColonistButton->OnClicked.AddDynamic(this, &UTownInfoPage::OnBuyColonistButtonClicked);
	}

	// 设置滚动条可见并始终显示（避免样式问题导致滚动条不显示）
	if (CaravanListScrollBox)
	{
		CaravanListScrollBox->SetVisibility(ESlateVisibility::Visible);
		CaravanListScrollBox->SetAlwaysShowScrollbar(true);
	}
	
	if (ColonistListScrollBox)
	{
		ColonistListScrollBox->SetVisibility(ESlateVisibility::Visible);
		ColonistListScrollBox->SetAlwaysShowScrollbar(true);
	}
}

void UTownInfoPage::RefreshPage()
{
	Super::RefreshPage();
	
	// 刷新基础资源图标
	RefreshNativeResourcesIcons();
	
	// 刷新商队和殖民者列表
	RefreshCaravanList();
	RefreshColonistList();
}

// ===== 商队管理 =====

void UTownInfoPage::RefreshCaravanList()
{
	if (!CaravanListScrollBox || !GameMode)
	{
		return;
	}
	
	// 清空现有列表
	CaravanListScrollBox->ClearChildren();
	SelectedCaravanUnitEntry = nullptr;
	
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS) return;
	
	// 获取城镇数据
	FTownData01 CurrentTownData = PS->GetTownData(TownData.EntityID);
	TArray<int32> CaravanIDs = CurrentTownData.CaravansInTown;
	
	// 获取 UnitEntryWidget01 类
	if (!GameMode->GetUIManager()) return;
	
	TSubclassOf<UUnitEntryWidget01> EntryClass = GameMode->GetUIManager()->GetUnitEntryWidgetClass();
	if (!EntryClass) return;
	
	// 为每个商队创建Entry
	for (int32 CaravanID : CaravanIDs)
	{
		UUnitEntryWidget01* Entry = CreateWidget<UUnitEntryWidget01>(GetWorld(), EntryClass);
		if (Entry)
		{
			// 1. 先添加到ScrollBox（触发NativeConstruct）
			CaravanListScrollBox->AddChild(Entry);
			
			// 2. AddChild后Slate层就绪，手动应用样式
			UE_LOG(LogTemp, Log, TEXT("[TownInfoPage] 为 %s 应用样式"), *Entry->GetName());
			IDemo01Styleable::Execute_ApplyWidgetStyle(Entry);
			
			// 3. 延迟设置数据（确保Widget完全初始化）
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[Entry, CaravanID, this]()
				{
					Entry->SetCaravanData(CaravanID, nullptr); // 不直接传递TownInfoPanel
					Entry->InitializeUnitData();
				},
				0.1f,
				false
			);
		}
	}
}

AMoveableEntity01* UTownInfoPage::GetSelectedCaravan() const
{
	if (SelectedCaravanUnitEntry && GameMode)
	{
		ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (PS)
		{
			return PS->FindCaravanInstance(SelectedCaravanUnitEntry->GetUnitID());
		}
	}
	return nullptr;
}

int32 UTownInfoPage::GetSelectedCaravanID() const
{
	if (SelectedCaravanUnitEntry)
	{
		return SelectedCaravanUnitEntry->GetUnitID();
	}
	return -1;
}

void UTownInfoPage::OnCaravanEntryClicked(UUnitEntryWidget01* Entry)
{
	if (!Entry || !GameMode)
	{
		return;
	}
	
	// 取消之前选中的商队Entry
	if (SelectedCaravanUnitEntry && SelectedCaravanUnitEntry != Entry)
	{
		SelectedCaravanUnitEntry->SetSelected(false);
	}
	
	// 取消之前选中的殖民者Entry（互斥逻辑）
	if (SelectedColonistEntry)
	{
		SelectedColonistEntry->SetSelected(false);
		SelectedColonistEntry = nullptr;
	}
	
	// 设置新的选中Entry
	SelectedCaravanUnitEntry = Entry;
	Entry->SetSelected(true);
	
	// 通过UIManager打开商队面板（使用ID方式）
	int32 CaravanID = Entry->GetUnitID();
	if (GameMode->GetUIManager())
	{
		GameMode->GetUIManager()->OpenCaravanInfoPanelByID(CaravanID);
		UE_LOG(LogTemp, Log, TEXT("[TownInfoPage] 打开商队面板：ID %d"), CaravanID);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[TownInfoPage] 选中商队ID: %d"), CaravanID);
}

void UTownInfoPage::OnBuyCaravanButtonClicked()
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[TownInfoPage] GameMode 为空"));
		return;
	}
	
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[TownInfoPage] PlayerState 为空"));
		return;
	}
	
	// 检查金钱是否足够
	const int32 CaravanCost = 100;
	if (!PS->HasEnoughGold(CaravanCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TownInfoPage] 金钱不足，无法购买商队（需要 %d，当前 %.0f）"), 
			CaravanCost, PS->GetGold());
		return;
	}
	
	// 扣除金钱
	if (!PS->UpdateGold(-CaravanCost))
	{
		UE_LOG(LogTemp, Error, TEXT("[TownInfoPage] 扣除金钱失败"));
		return;
	}
	
	// 创建商队数据
	int32 NewCaravanID = PS->CreateCaravanData(TownData.EntityID);
	
	// 刷新商队列表
	RefreshCaravanList();
}

// ===== 殖民者管理 =====

void UTownInfoPage::RefreshColonistList()
{
	if (!ColonistListScrollBox || !GameMode)
	{
		return;
	}
	
	// 清空现有列表
	ColonistListScrollBox->ClearChildren();
	SelectedColonistEntry = nullptr;
	
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS) return;
	
	// 获取城镇数据
	FTownData01 CurrentTownData = PS->GetTownData(TownData.EntityID);
	TArray<int32> ColonistIDs = CurrentTownData.ColonistsInTown;
	
	// 获取 UnitEntryWidget01 类
	if (!GameMode->GetUIManager()) return;
	
	TSubclassOf<UUnitEntryWidget01> EntryClass = GameMode->GetUIManager()->GetUnitEntryWidgetClass();
	if (!EntryClass) return;
	
	// 为每个殖民者创建Entry
	for (int32 ColonistID : ColonistIDs)
	{
		UUnitEntryWidget01* Entry = CreateWidget<UUnitEntryWidget01>(GetWorld(), EntryClass);
		if (Entry)
		{
			// 1. 先添加到ScrollBox（触发NativeConstruct）
			ColonistListScrollBox->AddChild(Entry);
			
			// 2. AddChild后Slate层就绪，手动应用样式
			UE_LOG(LogTemp, Log, TEXT("[TownInfoPage] 为 %s 应用样式"), *Entry->GetName());
			IDemo01Styleable::Execute_ApplyWidgetStyle(Entry);
			
			// 3. 延迟设置数据（确保Widget完全初始化）
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[Entry, ColonistID, this]()
				{
					Entry->SetColonistData(ColonistID, nullptr); // 不直接传递TownInfoPanel
					Entry->InitializeUnitData();
				},
				0.1f,
				false
			);
		}
	}
}

void UTownInfoPage::OnColonistEntryClicked(UUnitEntryWidget01* Entry)
{
	if (!Entry)
	{
		return;
	}
	
	// 取消之前选中的殖民者Entry
	if (SelectedColonistEntry && SelectedColonistEntry != Entry)
	{
		SelectedColonistEntry->SetSelected(false);
	}
	
	// 取消之前选中的商队Entry（互斥逻辑）
	if (SelectedCaravanUnitEntry)
	{
		SelectedCaravanUnitEntry->SetSelected(false);
		SelectedCaravanUnitEntry = nullptr;
	}
	
	// 设置新的选中Entry
	SelectedColonistEntry = Entry;
	Entry->SetSelected(true);
	
	UE_LOG(LogTemp, Log, TEXT("[TownInfoPage] 选中殖民者ID: %d"), Entry->GetUnitID());
}

AMoveableEntity01* UTownInfoPage::GetSelectedColonist() const
{
	if (SelectedColonistEntry && GameMode)
	{
		ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		if (PS)
		{
			return PS->FindColonistInstance(SelectedColonistEntry->GetUnitID());
		}
	}
	return nullptr;
}

int32 UTownInfoPage::GetSelectedColonistID() const
{
	if (SelectedColonistEntry)
	{
		return SelectedColonistEntry->GetUnitID();
	}
	return -1;
}

void UTownInfoPage::OnBuyColonistButtonClicked()
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[TownInfoPage] GameMode 为空"));
		return;
	}
	
	// 通过GameMode购买殖民者
	TownInstance = nullptr;
	ADemo01_PS* PS = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (PS)
	{
		TownInstance = PS->FindTownInstance(TownData.EntityID);
	}
	
	if (!TownInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[TownInfoPage] 无法找到城镇实例"));
		return;
	}
	
	// 调用GameMode的购买殖民者方法
	bool bSuccess = GameMode->PurchaseColonist(TownInstance, TEXT(""));
	
	if (bSuccess)
	{
		// 刷新殖民者列表
		RefreshColonistList();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TownInfoPage] 购买殖民者失败"));
	}
}

// ===== 基础资源展示方法 =====

void UTownInfoPage::RefreshNativeResourcesIcons()
{
	if (!HB_NativeResources || !TownInstance) return;
	
	// 清空现有图标
	for (UImage* Icon : NativeResourceIcons)
	{
		if (Icon && Icon->IsValidLowLevel())
		{
			HB_NativeResources->RemoveChild(Icon);
		}
	}
	NativeResourceIcons.Empty();
	
	// 清空SizeBox容器
	for (USizeBox* SizeBox : NativeResourceSizeBoxes)
	{
		if (SizeBox && SizeBox->IsValidLowLevel())
		{
			HB_NativeResources->RemoveChild(SizeBox);
		}
	}
	NativeResourceSizeBoxes.Empty();
	
	// 获取城镇的基础资源
	TArray<FString> ResourceIDs = GetTownNativeResourceIDs();
	
	// 创建图标控件
	for (const FString& ResourceID : ResourceIDs)
	{
		UImage* ResourceIcon = NewObject<UImage>(this);
		if (ResourceIcon)
		{
			// 设置图标纹理
			UTexture2D* IconTexture = GetResourceIcon(ResourceID);
			if (IconTexture)
			{
				ResourceIcon->SetBrushFromTexture(IconTexture);
			}
			
			// 设置图标大小
			FSlateBrush Brush = ResourceIcon->GetBrush();
			Brush.ImageSize = FVector2D(32.0f, 32.0f); // 32x32像素
			ResourceIcon->SetBrush(Brush);
			
			// 添加边距容器
			USizeBox* SizeBox = NewObject<USizeBox>(this);
			if (SizeBox)
			{
				SizeBox->SetWidthOverride(40.0f);
				SizeBox->SetHeightOverride(40.0f);
				SizeBox->AddChild(ResourceIcon);
				
				// 添加到水平框
				HB_NativeResources->AddChild(SizeBox);
				NativeResourceIcons.Add(ResourceIcon);
				NativeResourceSizeBoxes.Add(SizeBox);
				
				// 设置工具提示
				FText TooltipText = GetResourceDisplayName(ResourceID);
				ResourceIcon->SetToolTipText(TooltipText);
			}
		}
	}
	
	// 控制显示/隐藏
	bool bHasResources = ResourceIDs.Num() > 0;
	if (HB_NativeResources)
	{
		HB_NativeResources->SetVisibility(bHasResources ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (TB_NativeResourcesLabel)
	{
		TB_NativeResourcesLabel->SetVisibility(bHasResources ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[TownInfoPage] 刷新基础资源图标: %d 个资源"), ResourceIDs.Num());
}

TArray<FString> UTownInfoPage::GetTownNativeResourceIDs() const
{
	TArray<FString> Result;
	if (TownInstance)
	{
		// 获取城镇的基础配方ID - 从TownData中获取
		ADemo01_PS* PS = GameMode ? GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>() : nullptr;
		if (PS)
		{
			FTownData01 CurrentTownData = PS->GetTownData(TownInstance->EntityID);
			TArray<FString> NativeRecipeIDs = CurrentTownData.NativeRecipeIDs;
			
			// 转换为资源ID
			for (const FString& RecipeID : NativeRecipeIDs)
			{
				if (GameMode && GameMode->GetProductionManager())
				{
					FRecipeData01 RecipeData = GameMode->GetProductionManager()->GetRecipeData(RecipeID);
					if (!RecipeData.ProductID.IsEmpty())
					{
						Result.Add(RecipeData.ProductID);
					}
				}
			}
		}
	}
	return Result;
}

UTexture2D* UTownInfoPage::GetResourceIcon(const FString& ResourceID) const
{
	if (GameMode && GameMode->GetProductionManager())
	{
		// 从ProductionManager获取产品数据
		FProductData01 ProductData = GameMode->GetProductionManager()->GetProductData(ResourceID);
		
		// 检查图标是否有效
		if (ProductData.Icon && ProductData.Icon->IsValidLowLevel())
		{
			UE_LOG(LogTemp, Log, TEXT("[TownInfoPage] 成功加载资源图标: %s"), *ResourceID);
			return ProductData.Icon;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[TownInfoPage] 资源图标无效或为空: %s"), *ResourceID);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TownInfoPage] GameMode或ProductionManager为空"));
	}
	
	return nullptr;
}

FText UTownInfoPage::GetResourceDisplayName(const FString& ResourceID) const
{
	if (GameMode && GameMode->GetProductionManager())
	{
		// 从ProductionManager获取产品数据
		FProductData01 ProductData = GameMode->GetProductionManager()->GetProductData(ResourceID);
		
		// 检查显示名称是否有效
		if (!ProductData.ProductName.IsEmpty())
		{
			return ProductData.ProductName;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[TownInfoPage] 产品显示名称为空: %s"), *ResourceID);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TownInfoPage] GameMode或ProductionManager为空"));
	}
	
	// 备用：返回资源ID作为显示名称
	return FText::FromString(ResourceID);
}