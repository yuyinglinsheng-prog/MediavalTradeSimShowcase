// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Widgets/UnitEntryWidget01.h"

#include "Demo01/UIs/Panels/TownInfoPanel.h"
#include "Demo01/UIs/Panels/TownInfoPage.h"
#include "Demo01/Core01/Demo01_PC.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"

void UUnitEntryWidget01::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 验证BindWidget元素
	if (!UnitNameText || !ClickButton)
	{
		// UE_LOG(LogTemp, Error, TEXT("[UnitEntry] BindWidget元素未绑定！"));
		return;
	}
	
	// 绑定点击事件
	ClickButton->OnClicked.AddDynamic(this, &UUnitEntryWidget01::OnClicked);
	

}

void UUnitEntryWidget01::SetCaravanData(int32 CaravanID, UTownInfoPanel* OwnerPanel)
{
	// OwnerPanel参数已废弃，保留只是为了向后兼容
	SetUnitData(CaravanID, TEXT(""), EUnitType01::Caravan, OwnerPanel);
}

void UUnitEntryWidget01::SetColonistData(int32 ColonistID, UTownInfoPanel* OwnerPanel)
{
	// OwnerPanel参数已废弃，保留只是为了向后兼容
	SetUnitData(ColonistID, TEXT(""), EUnitType01::Colonist, OwnerPanel);
}

void UUnitEntryWidget01::SetUnitData(int32 UnitID, const FString& UnitName, EUnitType01 InUnitType, UTownInfoPanel* OwnerPanel)
{
	UnitIDRef = UnitID;
	UnitType = InUnitType;
	// OwnerPanelRef不再使用，Entry直接通过UIManager打开面板
	// OwnerPanelRef = OwnerPanel;
}

void UUnitEntryWidget01::InitializeUnitData()
{
	if (UnitIDRef < 0)
	{
		return;
	}
	
	// 获取单位数据
	UWorld* World = GetWorld();
	if (!World) return;
	
	ADemo01_GM* GM = Cast<ADemo01_GM>(World->GetAuthGameMode());
	if (!GM) return;
	
	ADemo01_PS* PS = World->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
	if (!PS) return;
	
	FString UnitName;
	
	// 根据单位类型获取数据
	switch (UnitType)
	{
	case EUnitType01::Caravan:
		{
			FCaravanData01 CaravanData = PS->GetCaravanData(UnitIDRef);
			UnitName = CaravanData.EntityName;
		}
		break;
		
	case EUnitType01::Colonist:
		{
			FColonistData01 ColonistData = PS->GetColonistDataCopy(UnitIDRef);
			UnitName = ColonistData.EntityName;
		}
		break;
	}
	
	// 更新显示
	if (UnitNameText)
	{
		UnitNameText->SetText(FText::FromString(UnitName));
	}
	
	// 更新单位类型显示（如果有UnitTypeText组件）
	if (UnitTypeText)
	{
		UnitTypeText->SetText(FText::FromString(GetUnitTypeDisplayText()));
	}
	
	// 初始化为未选中状态
	SetSelected(false);
}

void UUnitEntryWidget01::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	
	// 选中状态的视觉反馈现在通过基类的样式系统处理
	// 或者通过蓝图事件来处理视觉变化
	OnSelectionChanged(bSelected);
}

void UUnitEntryWidget01::OnClicked()
{
	// UE_LOG(LogTemp, Warning, TEXT("[UnitEntry] ========== OnClicked =========="));
	// UE_LOG(LogTemp, Warning, TEXT("[UnitEntry] 点击单位：ID=%d, Type=%d"), UnitIDRef, (int32)UnitType);
	
	if (UnitIDRef < 0)
	{
		// UE_LOG(LogTemp, Error, TEXT("[UnitEntry] 无效的单位ID，退出"));
		return;
	}
	
	// 获取PlayerController并选择单位（让选择系统处理面板打开）
	UWorld* World = GetWorld();
	if (!World)
	{
		// UE_LOG(LogTemp, Error, TEXT("[UnitEntry] 无法获取World"));
		return;
	}
	
	ADemo01_PC* PC = Cast<ADemo01_PC>(World->GetFirstPlayerController());
	if (PC)
	{
		// 使用新的选择方法，让选择系统自动处理面板打开
		EEntityType01 EntityType = (UnitType == EUnitType01::Caravan) ? EEntityType01::Caravan : EEntityType01::Colonist;
		// UE_LOG(LogTemp, Warning, TEXT("[UnitEntry] 调用PC->SelectMoveableUnit(Type=%d, ID=%d)"), (int32)EntityType, UnitIDRef);
		PC->SelectMoveableUnit(UnitIDRef, EntityType);
	}
	else
	{
		// UE_LOG(LogTemp, Error, TEXT("[UnitEntry] 无法获取PlayerController"));
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("[UnitEntry] ========== OnClicked 结束 =========="));
}

FString UUnitEntryWidget01::GetUnitTypeDisplayText() const
{
	switch (UnitType)
	{
	case EUnitType01::Caravan:
		return TEXT("商队");
	case EUnitType01::Colonist:
		return TEXT("殖民者");
	default:
		return TEXT("未知");
	}
}