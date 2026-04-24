// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/UIs/Panels/TownPageBase.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Demo01/Core01/Demo01_GM.h"

void UTownPageBase::SetTownData(const FTownData01& Data)
{
	TownData = Data;
	
	// UE_LOG(LogTemp, Log, TEXT("[TownPageBase] 城镇数据已设置：%s"), *TownData.EntityName);
	
	// 如果页面已初始化且TownInstance不为空，立即刷新
	if (bIsInitialized && TownInstance != nullptr)
	{
		RefreshPage();
	}
}

void UTownPageBase::SetTownInstance(ATownActor01* InTownInstance)
{
	if (TownInstance == InTownInstance)
	{
		return;
	}
	
	TownInstance = InTownInstance;
	
	// UE_LOG(LogTemp, Log, TEXT("[TownPageBase] 城镇实例已设置"));
	
	// 如果页面已初始化，立即刷新
	if (bIsInitialized)
	{
		RefreshPage();
	}
}

void UTownPageBase::RefreshPage()
{
	// 基类默认实现为空，子类可以重写
	// UE_LOG(LogTemp, Log, TEXT("[TownPageBase] 刷新页面：%s"), *GetClass()->GetName());
}

void UTownPageBase::InitializePage(ADemo01_GM* InGameMode)
{
	// 调用基类的InitializeUI方法
	InitializeUI(InGameMode);
	
	// UE_LOG(LogTemp, Log, TEXT("[TownPageBase] 页面初始化完成：%s"), *GetClass()->GetName());
	
	// 如果已有城镇数据，立即刷新
	if (TownData.EntityID >= 0)
	{
		RefreshPage();
	}
}