// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/UIs/Core/Demo01StyleManager.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h"

void UDemo01UIBase::InitializeUI(ADemo01_GM* InGameMode)
{
	GameMode = InGameMode;
	bIsInitialized = true;
	
	// UE_LOG(LogTemp, Log, TEXT("[UIBase] UI 初始化：%s"), *GetName());
}

void UDemo01UIBase::ShowUI()
{
	if (!bIsInitialized)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[UIBase] UI 未初始化，无法显示：%s"), *GetName());
		return;
	}
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	
	// UE_LOG(LogTemp, Log, TEXT("[UIBase] UI 显示：%s"), *GetName());
}

void UDemo01UIBase::HideUI()
{
	SetVisibility(ESlateVisibility::Hidden);
	
	// UE_LOG(LogTemp, Log, TEXT("[UIBase] UI 隐藏：%s"), *GetName());
}

void UDemo01UIBase::UpdateUI()
{
	// 子类实现具体的更新逻辑
}

void UDemo01UIBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 自动应用样式（如果启用）
	if (IDemo01Styleable::Execute_ShouldAutoApplyStyle(this))
	{
		IDemo01Styleable::Execute_ApplyWidgetStyle(this);
	}
}

void UDemo01UIBase::ApplyWidgetStyle_Implementation()
{
	// 基类默认实现：自动应用样式到所有BindWidget组件
	AutoApplyStylesToBindWidgets();
	
	// 递归应用样式到所有非UDemo01UIBase子类的子Widget
	ApplyStyleToChildren(this);
	
	// 添加调试日志
	UE_LOG(LogTemp, Log, TEXT("[UIBase] 已应用样式：%s (类型：%s)"), *GetName(), *IDemo01Styleable::Execute_GetStyleType(this));
}

FString UDemo01UIBase::GetStyleType_Implementation() const
{
	// 基于类名返回样式类型
	FString ClassName = GetClass()->GetName();
	
	// 移除前缀和后缀
	ClassName.RemoveFromStart(TEXT("U"));
	ClassName.RemoveFromEnd(TEXT("_C"));
	
	return ClassName;
}

void UDemo01UIBase::AutoApplyStylesToBindWidgets()
{
	UDemo01StyleManager* StyleManager = UDemo01StyleManager::GetInstance();
	if (!StyleManager)
	{
		return;
	}

	// 使用反射获取所有BindWidget属性
	for (TFieldIterator<FObjectProperty> PropIt(GetClass()); PropIt; ++PropIt)
	{
		FObjectProperty* Property = *PropIt;
		
		// 检查是否有BindWidget元数据
		if (Property->HasMetaData(TEXT("BindWidget")) || Property->HasMetaData(TEXT("BindWidgetOptional")))
		{
			// 获取属性值（Widget指针）
			UObject* WidgetObject = Property->GetObjectPropertyValue_InContainer(this);
			UWidget* Widget = Cast<UWidget>(WidgetObject);
			
			if (Widget)
			{
				// 根据Widget类型应用相应样式
				ApplyStyleToComponent(Widget, Property->GetCPPType(nullptr, 0));
			}
		}
	}
}

void UDemo01UIBase::ApplyStyleToComponent(UWidget* Widget, const FString& ComponentType)
{
	if (!Widget)
	{
		return;
	}

	// EditableTextBox的SetWidgetStyle会重建Slate widget，可能破坏Slate树，跳过
	if (Cast<UEditableTextBox>(Widget))
	{
		return;
	}

	UDemo01StyleManager* StyleManager = UDemo01StyleManager::GetInstance();
	if (!StyleManager)
	{
		return;
	}

	// 根据组件类型应用相应样式
	if (UButton* Button = Cast<UButton>(Widget))
	{
		// 根据按钮名称判断类型
		FString ButtonName = Button->GetName();
		bool bIsPrimary = ButtonName.Contains(TEXT("Primary")) || ButtonName.Contains(TEXT("Confirm")) || ButtonName.Contains(TEXT("Buy"));
		bool bIsClose = ButtonName.Contains(TEXT("Close")) || ButtonName.Contains(TEXT("Cancel")) || ButtonName.Contains(TEXT("Delete"));
		
		StyleManager->ApplyButtonStyle(Button, bIsPrimary, bIsClose);
	}
	else if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
	{
		// 根据文本名称判断类型
		FString TextName = TextBlock->GetName();
		bool bIsHeader = TextName.Contains(TEXT("Title")) || TextName.Contains(TEXT("Header")) || TextName.Contains(TEXT("Name"));
		bool bIsSecondary = TextName.Contains(TEXT("Secondary")) || TextName.Contains(TEXT("Caption")) || TextName.Contains(TEXT("Description"));
		
		StyleManager->ApplyTextStyle(TextBlock, bIsHeader, bIsSecondary);
	}
	else if (UEditableTextBox* InputBox = Cast<UEditableTextBox>(Widget))
	{
		StyleManager->ApplyInputBoxStyle(InputBox);
	}
	else if (UScrollBox* ScrollBox = Cast<UScrollBox>(Widget))
	{
		StyleManager->ApplyScrollBoxStyle(ScrollBox);
	}
	else if (UImage* Image = Cast<UImage>(Widget))
	{
		bool bIsIcon = Image->GetName().Contains(TEXT("Icon"));
		StyleManager->ApplyImageStyle(Image, bIsIcon, false);
	}
	else if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
	{
		StyleManager->ApplyProgressBarStyle(ProgressBar);
	}
	else if (UVerticalBox* VBox = Cast<UVerticalBox>(Widget))
	{
		StyleManager->ApplyContainerStyle(VBox);
	}
	else if (UHorizontalBox* HBox = Cast<UHorizontalBox>(Widget))
	{
		StyleManager->ApplyContainerStyle(HBox);
	}
	else if (UBorder* Border = Cast<UBorder>(Widget))
	{
		bool bIsListItem = Border->GetName().Contains(TEXT("Item")) || Border->GetName().Contains(TEXT("Entry"));
		StyleManager->ApplyBorderStyle(Border, bIsListItem, false);
	}
}

void UDemo01UIBase::ApplyStyleToChildren(UWidget* ParentWidget)
{
	if (!ParentWidget) return;
	
	// 检查是否是面板类型的Widget
	if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(ParentWidget))
	{
		// 遍历所有子Widget
		for (int32 i = 0; i < PanelWidget->GetChildrenCount(); i++)
		{
			UWidget* ChildWidget = PanelWidget->GetChildAt(i);
			if (ChildWidget)
			{
				// 检查子Widget是否是UDemo01UIBase的子类
				// 如果是，跳过，因为它们会在自己的NativeConstruct中自动应用样式
				if (!Cast<UDemo01UIBase>(ChildWidget))
				{
					// 应用样式到子Widget
					ApplyStyleToComponent(ChildWidget, ChildWidget->GetClass()->GetName());
					
					// 递归处理子Widget的子Widget
					ApplyStyleToChildren(ChildWidget);
				}
			}
		}
	}
}
