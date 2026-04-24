// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Core/Demo01StyleManager.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

// 静态实例
UDemo01StyleManager* UDemo01StyleManager::Instance = nullptr;

UDemo01StyleManager* UDemo01StyleManager::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<UDemo01StyleManager>();
		Instance->AddToRoot(); // 防止被垃圾回收
		Instance->Initialize();
	}
	return Instance;
}

void UDemo01StyleManager::Initialize()
{
	// 创建默认样式配置
	CreateDefaultStyleConfig();
	
	// 尝试从文件加载样式配置
	FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("Demo01UIStyle.json");
	if (FPaths::FileExists(ConfigPath))
	{
		LoadStyleFromFile(ConfigPath);
		// UE_LOG(LogTemp, Log, TEXT("Demo01StyleManager: 从文件加载样式配置: %s"), *ConfigPath);
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("Demo01StyleManager: 样式配置文件不存在，使用默认配置: %s"), *ConfigPath);
	}
}

bool UDemo01StyleManager::LoadStyleFromFile(const FString& FilePath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		// UE_LOG(LogTemp, Error, TEXT("Demo01StyleManager: 无法读取样式配置文件: %s"), *FilePath);
		return false;
	}

	if (ParseStyleFromJson(JsonString))
	{
		// UE_LOG(LogTemp, Log, TEXT("Demo01StyleManager: 成功加载样式配置"));
		RefreshAllStyles();
		return true;
	}

	// UE_LOG(LogTemp, Error, TEXT("Demo01StyleManager: 解析样式配置失败"));
	return false;
}

bool UDemo01StyleManager::ParseStyleFromJson(const FString& JsonString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	// 解析颜色配置
	if (JsonObject->HasField(TEXT("colors")))
	{
		TSharedPtr<FJsonObject> ColorsObj = JsonObject->GetObjectField(TEXT("colors"));
		
		// 面板颜色
		if (ColorsObj->HasField(TEXT("panelBackground")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("panelBackground"));
			StyleConfig.Colors.PanelBackground = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("panelBorder")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("panelBorder"));
			StyleConfig.Colors.PanelBorder = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("panelHeader")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("panelHeader"));
			StyleConfig.Colors.PanelHeader = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}

		// 按钮颜色
		if (ColorsObj->HasField(TEXT("buttonPrimary")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("buttonPrimary"));
			StyleConfig.Colors.ButtonPrimary = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("buttonPrimaryHover")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("buttonPrimaryHover"));
			StyleConfig.Colors.ButtonPrimaryHover = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("buttonSecondary")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("buttonSecondary"));
			StyleConfig.Colors.ButtonSecondary = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("buttonClose")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("buttonClose"));
			StyleConfig.Colors.ButtonClose = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}

		// 文本颜色
		if (ColorsObj->HasField(TEXT("textPrimary")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("textPrimary"));
			StyleConfig.Colors.TextPrimary = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("textSecondary")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("textSecondary"));
			StyleConfig.Colors.TextSecondary = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}

		// 列表项颜色
		if (ColorsObj->HasField(TEXT("listItemBackground")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("listItemBackground"));
			StyleConfig.Colors.ListItemBackground = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("listItemHover")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("listItemHover"));
			StyleConfig.Colors.ListItemHover = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("listItemSelected")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("listItemSelected"));
			StyleConfig.Colors.ListItemSelected = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("listItemBorder")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("listItemBorder"));
			StyleConfig.Colors.ListItemBorder = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}

		// 滚动框颜色
		if (ColorsObj->HasField(TEXT("scrollBoxBackground")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("scrollBoxBackground"));
			StyleConfig.Colors.ScrollBoxBackground = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("scrollBarColor")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("scrollBarColor"));
			StyleConfig.Colors.ScrollBarColor = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("scrollBarHover")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("scrollBarHover"));
			StyleConfig.Colors.ScrollBarHover = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}

		// 输入框颜色
		if (ColorsObj->HasField(TEXT("inputBackground")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("inputBackground"));
			StyleConfig.Colors.InputBackground = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("inputBorder")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("inputBorder"));
			StyleConfig.Colors.InputBorder = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("inputFocusBorder")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("inputFocusBorder"));
			StyleConfig.Colors.InputFocusBorder = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}

		// 图标颜色
		if (ColorsObj->HasField(TEXT("iconTint")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("iconTint"));
			StyleConfig.Colors.IconTint = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("iconDisabledTint")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("iconDisabledTint"));
			StyleConfig.Colors.IconDisabledTint = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}

		// 进度条颜色
		if (ColorsObj->HasField(TEXT("progressBackground")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("progressBackground"));
			StyleConfig.Colors.ProgressBackground = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
		if (ColorsObj->HasField(TEXT("progressFill")))
		{
			FString ColorStr = ColorsObj->GetStringField(TEXT("progressFill"));
			StyleConfig.Colors.ProgressFill = FLinearColor::FromSRGBColor(FColor::FromHex(ColorStr));
		}
	}

	// 解析字体配置
	if (JsonObject->HasField(TEXT("fonts")))
	{
		TSharedPtr<FJsonObject> FontsObj = JsonObject->GetObjectField(TEXT("fonts"));
		
		if (FontsObj->HasField(TEXT("headerSize")))
		{
			StyleConfig.Fonts.HeaderSize = FontsObj->GetIntegerField(TEXT("headerSize"));
		}
		if (FontsObj->HasField(TEXT("subHeaderSize")))
		{
			StyleConfig.Fonts.SubHeaderSize = FontsObj->GetIntegerField(TEXT("subHeaderSize"));
		}
		if (FontsObj->HasField(TEXT("bodySize")))
		{
			StyleConfig.Fonts.BodySize = FontsObj->GetIntegerField(TEXT("bodySize"));
		}
		if (FontsObj->HasField(TEXT("captionSize")))
		{
			StyleConfig.Fonts.CaptionSize = FontsObj->GetIntegerField(TEXT("captionSize"));
		}
		if (FontsObj->HasField(TEXT("buttonSize")))
		{
			StyleConfig.Fonts.ButtonSize = FontsObj->GetIntegerField(TEXT("buttonSize"));
		}
	}

	// 解析间距配置
	if (JsonObject->HasField(TEXT("spacing")))
	{
		TSharedPtr<FJsonObject> SpacingObj = JsonObject->GetObjectField(TEXT("spacing"));
		
		if (SpacingObj->HasField(TEXT("panelPadding")))
		{
			StyleConfig.Spacing.PanelPadding = SpacingObj->GetNumberField(TEXT("panelPadding"));
		}
		if (SpacingObj->HasField(TEXT("panelHeaderPadding")))
		{
			StyleConfig.Spacing.PanelHeaderPadding = SpacingObj->GetNumberField(TEXT("panelHeaderPadding"));
		}
		if (SpacingObj->HasField(TEXT("buttonPaddingHorizontal")))
		{
			StyleConfig.Spacing.ButtonPaddingHorizontal = SpacingObj->GetNumberField(TEXT("buttonPaddingHorizontal"));
		}
		if (SpacingObj->HasField(TEXT("buttonPaddingVertical")))
		{
			StyleConfig.Spacing.ButtonPaddingVertical = SpacingObj->GetNumberField(TEXT("buttonPaddingVertical"));
		}
		if (SpacingObj->HasField(TEXT("listItemPaddingHorizontal")))
		{
			StyleConfig.Spacing.ListItemPaddingHorizontal = SpacingObj->GetNumberField(TEXT("listItemPaddingHorizontal"));
		}
		if (SpacingObj->HasField(TEXT("listItemPaddingVertical")))
		{
			StyleConfig.Spacing.ListItemPaddingVertical = SpacingObj->GetNumberField(TEXT("listItemPaddingVertical"));
		}
		if (SpacingObj->HasField(TEXT("cornerRadius")))
		{
			StyleConfig.Spacing.CornerRadius = SpacingObj->GetNumberField(TEXT("cornerRadius"));
		}
		if (SpacingObj->HasField(TEXT("borderWidth")))
		{
			StyleConfig.Spacing.BorderWidth = SpacingObj->GetNumberField(TEXT("borderWidth"));
		}

		// 滚动框间距
		if (SpacingObj->HasField(TEXT("scrollBoxPadding")))
		{
			StyleConfig.Spacing.ScrollBoxPadding = SpacingObj->GetNumberField(TEXT("scrollBoxPadding"));
		}
		if (SpacingObj->HasField(TEXT("scrollBarWidth")))
		{
			StyleConfig.Spacing.ScrollBarWidth = SpacingObj->GetNumberField(TEXT("scrollBarWidth"));
		}

		// 输入框间距
		if (SpacingObj->HasField(TEXT("inputPaddingHorizontal")))
		{
			StyleConfig.Spacing.InputPaddingHorizontal = SpacingObj->GetNumberField(TEXT("inputPaddingHorizontal"));
		}
		if (SpacingObj->HasField(TEXT("inputPaddingVertical")))
		{
			StyleConfig.Spacing.InputPaddingVertical = SpacingObj->GetNumberField(TEXT("inputPaddingVertical"));
		}

		// 标签页间距
		if (SpacingObj->HasField(TEXT("tabButtonSpacing")))
		{
			StyleConfig.Spacing.TabButtonSpacing = SpacingObj->GetNumberField(TEXT("tabButtonSpacing"));
		}
		if (SpacingObj->HasField(TEXT("tabContentPadding")))
		{
			StyleConfig.Spacing.TabContentPadding = SpacingObj->GetNumberField(TEXT("tabContentPadding"));
		}
	}

	return true;
}

void UDemo01StyleManager::ApplyPanelStyle(UDemo01PanelBase* Panel)
{
	if (!Panel)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(Panel);

	// 应用样式到面板及其子组件
	ApplyStyleToWidget(Panel);
}

void UDemo01StyleManager::ApplyButtonStyle(UButton* Button, bool bIsPrimary, bool bIsCloseButton)
{
	if (!Button)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(Button);

	// 选择按钮颜色
	FLinearColor ButtonColor = StyleConfig.Colors.ButtonSecondary;
	if (bIsCloseButton)
	{
		ButtonColor = StyleConfig.Colors.ButtonClose;
	}
	else if (bIsPrimary)
	{
		ButtonColor = StyleConfig.Colors.ButtonPrimary;
	}

	// 设置按钮样式
	SetWidgetVisualProperties(Button, ButtonColor, FLinearColor::Transparent, 0.0f);

	// 设置按钮内边距
	FMargin ButtonPadding(
		StyleConfig.Spacing.ButtonPaddingHorizontal,
		StyleConfig.Spacing.ButtonPaddingVertical,
		StyleConfig.Spacing.ButtonPaddingHorizontal,
		StyleConfig.Spacing.ButtonPaddingVertical
	);

	// 如果按钮有文本子组件，应用文本样式
	ApplyStyleToChildren(Button);
}

void UDemo01StyleManager::ApplyTabButtonStyle(UButton* Button, bool bIsSelected)
{
	if (!Button) return;

	if (bIsSelected)
	{
		// 选中状态：金色
		Button->SetBackgroundColor(StyleConfig.Colors.ListItemSelected);
	}
	else
	{
		// 未选中状态：深蓝色
		Button->SetBackgroundColor(StyleConfig.Colors.ListItemBackground);
	}

	// 设置按钮文本样式
	if (UWidget* ButtonContent = Button->GetContent())
	{
		ApplyStyleToWidget(ButtonContent);
	}
}

void UDemo01StyleManager::ApplyResourceProgressStyle(UProgressBar* ProgressBar)
{
	if (!ProgressBar) return;

	// 设置进度条样式
	FProgressBarStyle WidgetStyle = ProgressBar->GetWidgetStyle();
	
	// 设置背景颜色
	WidgetStyle.BackgroundImage.TintColor = FSlateColor(StyleConfig.Colors.ProgressBackground);
	
	// 设置填充颜色
	WidgetStyle.FillImage.TintColor = FSlateColor(StyleConfig.Colors.ProgressFill);
	
	ProgressBar->SetWidgetStyle(WidgetStyle);
}

void UDemo01StyleManager::ApplyTextStyle(UTextBlock* TextBlock, bool bIsHeader, bool bIsSecondary)
{
	if (!TextBlock)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(TextBlock);

	// 选择文本颜色
	FLinearColor TextColor = bIsSecondary ? StyleConfig.Colors.TextSecondary : StyleConfig.Colors.TextPrimary;
	
	// 选择字体大小
	int32 FontSize = StyleConfig.Fonts.BodySize;
	if (bIsHeader)
	{
		FontSize = StyleConfig.Fonts.HeaderSize;
		TextColor = StyleConfig.Colors.TextHeader;
	}

	// 应用文本样式
	TextBlock->SetColorAndOpacity(FSlateColor(TextColor));
	
	// 设置字体大小
	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = FontSize;
	TextBlock->SetFont(FontInfo);
}

void UDemo01StyleManager::ApplyBorderStyle(UBorder* Border, bool bIsListItem, bool bIsSelected)
{
	if (!Border)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(Border);

	// 选择背景颜色
	FLinearColor BackgroundColor = StyleConfig.Colors.PanelBackground;
	FLinearColor BorderColor = StyleConfig.Colors.PanelBorder;
	
	if (bIsListItem)
	{
		BackgroundColor = bIsSelected ? StyleConfig.Colors.ListItemSelected : StyleConfig.Colors.ListItemBackground;
		BorderColor = StyleConfig.Colors.ListItemBorder;
	}

	// 设置边框样式
	SetWidgetVisualProperties(Border, BackgroundColor, BorderColor, StyleConfig.Spacing.BorderWidth);
}

void UDemo01StyleManager::ApplyListItemStyle(UWidget* ListItem, bool bIsSelected, bool bIsHovered)
{
	if (!ListItem)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(ListItem);

	// 选择背景颜色
	FLinearColor BackgroundColor = StyleConfig.Colors.ListItemBackground;
	if (bIsSelected)
	{
		BackgroundColor = StyleConfig.Colors.ListItemSelected;
	}
	else if (bIsHovered)
	{
		BackgroundColor = StyleConfig.Colors.ListItemHover;
	}

	// 根据组件类型设置样式
	if (UUserWidget* UserWidget = Cast<UUserWidget>(ListItem))
	{
		// 对于UserWidget，尝试找到其根Widget并设置样式
		UWidget* RootWidget = UserWidget->GetRootWidget();
		if (UBorder* RootBorder = Cast<UBorder>(RootWidget))
		{
			RootBorder->SetBrushColor(BackgroundColor);
			// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 设置UserWidget根Border颜色：%s"), *UserWidget->GetName());
		}
		else if (UButton* RootButton = Cast<UButton>(RootWidget))
		{
			RootButton->SetBackgroundColor(BackgroundColor);
			// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 设置UserWidget根Button颜色：%s"), *UserWidget->GetName());
		}
		else
		{
			// UE_LOG(LogTemp, Warning, TEXT("[StyleManager] UserWidget根组件类型不支持：%s"), 
			// 	RootWidget ? *RootWidget->GetClass()->GetName() : TEXT("NULL"));
		}
	}
	else
	{
		// 对于其他类型的Widget，使用通用方法
		SetWidgetVisualProperties(ListItem, BackgroundColor, StyleConfig.Colors.ListItemBorder, StyleConfig.Spacing.BorderWidth);
	}

	// 应用样式到子组件
	ApplyStyleToChildren(ListItem);
}

void UDemo01StyleManager::ApplyStyleToWidget(UWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	// 根据组件类型应用相应样式
	if (UDemo01PanelBase* Panel = Cast<UDemo01PanelBase>(Widget))
	{
		// 面板样式
		SetWidgetVisualProperties(Panel, StyleConfig.Colors.PanelBackground, 
			StyleConfig.Colors.PanelBorder, StyleConfig.Spacing.PanelBorderWidth);
	}
	else if (UButton* Button = Cast<UButton>(Widget))
	{
		// 按钮样式
		ApplyButtonStyle(Button, false, false);
	}
	else if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
	{
		// 文本样式
		ApplyTextStyle(TextBlock, false, false);
	}
	else if (UBorder* Border = Cast<UBorder>(Widget))
	{
		// 边框样式
		ApplyBorderStyle(Border, false, false);
	}

	// 递归应用到子组件
	ApplyStyleToChildren(Widget);
}

void UDemo01StyleManager::ApplyStyleToChildren(UWidget* ParentWidget)
{
	if (!ParentWidget)
	{
		return;
	}

	// 如果是面板组件，遍历其子组件
	if (UPanelWidget* Panel = Cast<UPanelWidget>(ParentWidget))
	{
		for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
		{
			UWidget* Child = Panel->GetChildAt(i);
			// EditableTextBox的SetWidgetStyle会重建Slate widget，跳过
			if (Child && !Cast<UEditableTextBox>(Child))
			{
				ApplyStyleToWidget(Child);
			}
		}
	}
}

void UDemo01StyleManager::RefreshAllStyles()
{
	// 重新应用所有已样式化的组件
	for (int32 i = StyledWidgets.Num() - 1; i >= 0; --i)
	{
		if (StyledWidgets[i].IsValid())
		{
			ApplyStyleToWidget(StyledWidgets[i].Get());
		}
		else
		{
			// 移除无效的弱引用
			StyledWidgets.RemoveAt(i);
		}
	}
}

void UDemo01StyleManager::SetWidgetVisualProperties(UWidget* Widget, const FLinearColor& BackgroundColor, 
	const FLinearColor& BorderColor, float BorderWidth)
{
	if (!Widget)
	{
		return;
	}

	// 根据具体的UE组件类型设置视觉属性
	
	// 对于Button组件
	if (UButton* Button = Cast<UButton>(Widget))
	{
		// 设置按钮背景颜色
		Button->SetBackgroundColor(BackgroundColor);
		
		// 设置按钮样式（如果需要更复杂的样式，可能需要自定义ButtonStyle）
		// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 设置按钮颜色：%s, 颜色：R=%.2f G=%.2f B=%.2f A=%.2f"), 
		// 	*Button->GetName(), BackgroundColor.R, BackgroundColor.G, BackgroundColor.B, BackgroundColor.A);
	}
	// 对于Border组件
	else if (UBorder* Border = Cast<UBorder>(Widget))
	{
		Border->SetBrushColor(BackgroundColor);
		// 注意：UE的Border组件边框设置比较复杂，可能需要自定义Brush
		// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 设置边框颜色：%s"), *Border->GetName());
	}
	// 对于其他Panel组件，尝试设置背景色
	else if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
	{
		// 大多数Panel组件没有直接的背景色设置，可能需要通过Slate样式处理
		// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 尝试设置面板样式：%s (可能需要自定义实现)"), *Panel->GetName());
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("[StyleManager] 未知组件类型，无法设置视觉属性：%s"), *Widget->GetName());
	}
}

void UDemo01StyleManager::CreateDefaultStyleConfig()
{
	// 使用头文件中定义的默认值
	// 这些值已经在结构体中设置了合理的默认值
	
	// UE_LOG(LogTemp, Log, TEXT("Demo01StyleManager: 使用默认样式配置"));
}
bool UDemo01StyleManager::ReloadStyleConfiguration()
{
	FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("Demo01UIStyle.json");
	if (LoadStyleFromFile(ConfigPath))
	{
		// UE_LOG(LogTemp, Log, TEXT("Demo01StyleManager: 样式配置重新加载成功"));
		return true;
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("Demo01StyleManager: 样式配置重新加载失败，使用默认配置"));
		CreateDefaultStyleConfig();
		RefreshAllStyles();
		return false;
	}
}
void UDemo01StyleManager::ApplyScrollBoxStyle(UScrollBox* ScrollBox)
{
	if (!ScrollBox)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(ScrollBox);

	// 对于ScrollBox，我们主要通过其父容器或背景Border来设置样式
	// ScrollBox本身的样式设置比较复杂，通常需要自定义Slate样式
	
	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用滚动框样式：%s"), *ScrollBox->GetName());
}

void UDemo01StyleManager::ApplyInputBoxStyle(UEditableTextBox* InputBox)
{
	if (!InputBox)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(InputBox);

	// 设置输入框样式
	FEditableTextBoxStyle WidgetStyle = InputBox->GetWidgetStyle();
	
	// 设置文本颜色
	WidgetStyle.ForegroundColor = FSlateColor(StyleConfig.Colors.TextPrimary);
	
	// 设置背景颜色
	WidgetStyle.BackgroundColor = FSlateColor(StyleConfig.Colors.InputBackground);
	
	// 设置字体大小（通过TextStyle）
	if (WidgetStyle.TextStyle.Font.Size != StyleConfig.Fonts.BodySize)
	{
		FSlateFontInfo NewFont = WidgetStyle.TextStyle.Font;
		NewFont.Size = StyleConfig.Fonts.BodySize;
		WidgetStyle.TextStyle.Font = NewFont;
	}
	
	InputBox->SetWidgetStyle(WidgetStyle);

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用输入框样式：%s"), *InputBox->GetName());
}

void UDemo01StyleManager::ApplyImageStyle(UImage* Image, bool bIsIcon, bool bIsDisabled)
{
	if (!Image)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(Image);

	// 选择图像色调
	FLinearColor ImageTint = StyleConfig.Colors.IconTint;
	if (bIsDisabled)
	{
		ImageTint = StyleConfig.Colors.IconDisabledTint;
	}

	// 设置图像色调
	Image->SetColorAndOpacity(ImageTint);

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用图像样式：%s"), *Image->GetName());
}

void UDemo01StyleManager::ApplyProgressBarStyle(UProgressBar* ProgressBar)
{
	if (!ProgressBar)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(ProgressBar);

	// 设置进度条样式
	// 注意：UE的ProgressBar样式设置比较复杂，这里提供基础实现
	// 实际项目中可能需要使用自定义的Slate样式

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用进度条样式：%s"), *ProgressBar->GetName());
}

void UDemo01StyleManager::ApplyContainerStyle(UPanelWidget* Container)
{
	if (!Container)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(Container);

	// 对于容器组件，主要是应用样式到其子组件
	ApplyStyleToChildren(Container);

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用容器样式：%s"), *Container->GetName());
}

void UDemo01StyleManager::ApplyResourceEntryStyle(UUserWidget* ResourceEntry)
{
	if (!ResourceEntry)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(ResourceEntry);

	// 应用列表项样式
	ApplyListItemStyle(ResourceEntry, false, false);

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用资源条目样式：%s"), *ResourceEntry->GetName());
}

void UDemo01StyleManager::ApplyBuildingEntryStyle(UUserWidget* BuildingEntry)
{
	if (!BuildingEntry)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(BuildingEntry);

	// 应用列表项样式
	ApplyListItemStyle(BuildingEntry, false, false);

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用建筑条目样式：%s"), *BuildingEntry->GetName());
}

void UDemo01StyleManager::ApplyRecipeEntryStyle(UUserWidget* RecipeEntry, bool bIsSelected)
{
	if (!RecipeEntry)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(RecipeEntry);

	// 应用列表项样式，支持选中状态
	ApplyListItemStyle(RecipeEntry, bIsSelected, false);

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用配方条目样式：%s，选中状态：%s"), 
	// 	*RecipeEntry->GetName(), bIsSelected ? TEXT("是") : TEXT("否"));
}

void UDemo01StyleManager::ApplyUnitEntryStyle(UUserWidget* UnitEntry, bool bIsSelected)
{
	if (!UnitEntry)
	{
		return;
	}

	// 添加到已样式化组件列表
	StyledWidgets.AddUnique(UnitEntry);

	// 应用列表项样式，支持选中状态
	ApplyListItemStyle(UnitEntry, bIsSelected, false);

	// UE_LOG(LogTemp, Log, TEXT("[StyleManager] 已应用单位条目样式：%s，选中状态：%s"), 
	// 	*UnitEntry->GetName(), bIsSelected ? TEXT("是") : TEXT("否"));
}