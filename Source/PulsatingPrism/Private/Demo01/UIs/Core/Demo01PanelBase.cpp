// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "Demo01/UIs/Core/Demo01StyleManager.h"
#include "Components/CanvasPanelSlot.h"

void UDemo01PanelBase::OpenPanel()
{
	bIsOpen = true;
	ShowUI();
	
	UE_LOG(LogTemp, Log, TEXT("[PanelBase] 面板打开：%s"), *GetName());
}

void UDemo01PanelBase::ClosePanel()
{
	bIsOpen = false;
	HideUI();
	
	UE_LOG(LogTemp, Log, TEXT("[PanelBase] 面板关闭：%s"), *GetName());
}

void UDemo01PanelBase::MinimizePanel()
{
	bIsMinimized = true;
	// 在蓝图中实现最小化动画
	
	UE_LOG(LogTemp, Log, TEXT("[PanelBase] 面板最小化：%s"), *GetName());
}

void UDemo01PanelBase::RestorePanel()
{
	bIsMinimized = false;
	// 在蓝图中实现恢复动画
	
	UE_LOG(LogTemp, Log, TEXT("[PanelBase] 面板恢复：%s"), *GetName());
}

void UDemo01PanelBase::SetPanelPosition(FVector2D Position)
{
	PanelPosition = Position;
	
	// 更新 Widget 位置
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetPosition(Position);
	}
}
void UDemo01PanelBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 自动应用样式
	ApplyPanelStyle();
}

void UDemo01PanelBase::ApplyPanelStyle()
{
	// 获取样式管理器并应用面板样式
	UDemo01StyleManager* StyleManager = UDemo01StyleManager::GetInstance();
	if (StyleManager)
	{
		StyleManager->ApplyPanelStyle(this);
		UE_LOG(LogTemp, Log, TEXT("[PanelBase] 已应用面板样式：%s"), *GetName());
	}
}