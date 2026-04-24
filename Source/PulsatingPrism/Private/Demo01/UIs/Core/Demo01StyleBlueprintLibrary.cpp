// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Core/Demo01StyleBlueprintLibrary.h"
#include "Demo01/UIs/Core/Demo01StyleManager.h"
#include "Components/Widget.h"

UDemo01StyleManager* UDemo01StyleBlueprintLibrary::GetStyleManager()
{
	return UDemo01StyleManager::GetInstance();
}

bool UDemo01StyleBlueprintLibrary::ReloadStyleConfiguration()
{
	UDemo01StyleManager* StyleManager = GetStyleManager();
	if (StyleManager)
	{
		return StyleManager->ReloadStyleConfiguration();
	}
	return false;
}

void UDemo01StyleBlueprintLibrary::RefreshAllStyles()
{
	UDemo01StyleManager* StyleManager = GetStyleManager();
	if (StyleManager)
	{
		StyleManager->RefreshAllStyles();
	}
}

void UDemo01StyleBlueprintLibrary::ApplyStyleToWidget(UWidget* Widget)
{
	UDemo01StyleManager* StyleManager = GetStyleManager();
	if (StyleManager && Widget)
	{
		StyleManager->ApplyStyleToWidget(Widget);
	}
}

bool UDemo01StyleBlueprintLibrary::IsStyleSystemInitialized()
{
	UDemo01StyleManager* StyleManager = GetStyleManager();
	return StyleManager != nullptr;
}

void UDemo01StyleBlueprintLibrary::TestButtonColors()
{
	UDemo01StyleManager* StyleManager = GetStyleManager();
	if (StyleManager)
	{
		// 输出当前的按钮颜色配置到日志
		const FDemo01StyleConfig& Config = StyleManager->GetStyleConfig();
		
		UE_LOG(LogTemp, Warning, TEXT("=== 按钮颜色测试 ==="));
		UE_LOG(LogTemp, Warning, TEXT("主要按钮颜色: R=%.2f G=%.2f B=%.2f A=%.2f"), 
			Config.Colors.ButtonPrimary.R, Config.Colors.ButtonPrimary.G, 
			Config.Colors.ButtonPrimary.B, Config.Colors.ButtonPrimary.A);
		UE_LOG(LogTemp, Warning, TEXT("次要按钮颜色: R=%.2f G=%.2f B=%.2f A=%.2f"), 
			Config.Colors.ButtonSecondary.R, Config.Colors.ButtonSecondary.G, 
			Config.Colors.ButtonSecondary.B, Config.Colors.ButtonSecondary.A);
		UE_LOG(LogTemp, Warning, TEXT("关闭按钮颜色: R=%.2f G=%.2f B=%.2f A=%.2f"), 
			Config.Colors.ButtonClose.R, Config.Colors.ButtonClose.G, 
			Config.Colors.ButtonClose.B, Config.Colors.ButtonClose.A);
		UE_LOG(LogTemp, Warning, TEXT("==================="));
	}
}

void UDemo01StyleBlueprintLibrary::TestUIComponentStyles()
{
	UE_LOG(LogTemp, Warning, TEXT("=== UI组件样式测试 ==="));
	UE_LOG(LogTemp, Warning, TEXT("新的样式系统已启用："));
	UE_LOG(LogTemp, Warning, TEXT("- 所有继承自UDemo01UIBase的UI类会自动应用样式"));
	UE_LOG(LogTemp, Warning, TEXT("- 使用反射自动识别BindWidget组件"));
	UE_LOG(LogTemp, Warning, TEXT("- 根据组件名称智能判断样式类型"));
	UE_LOG(LogTemp, Warning, TEXT("- 子类可以重写ApplyWidgetStyle()方法自定义样式"));
	UE_LOG(LogTemp, Warning, TEXT("==================="));
}