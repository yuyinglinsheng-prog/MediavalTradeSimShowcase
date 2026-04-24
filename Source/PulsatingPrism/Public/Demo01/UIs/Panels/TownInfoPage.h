// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Panels/TownPageBase.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "TownInfoPage.generated.h"

class UScrollBox;
class UButton;
class UUnitEntryWidget01;
class AMoveableEntity01;
class UHorizontalBox;
class UTextBlock;
class UImage;
class USizeBox;

/**
 * 城镇基本信息页面
 * 显示城镇基本信息和进驻单位列表
 */
UCLASS()
class PULSATINGPRISM_API UTownInfoPage : public UTownPageBase
{
	GENERATED_BODY()

public:
	// 重写基类方法
	virtual void RefreshPage() override;
	
	// ===== 商队管理 =====
	
	// 刷新商队列表
	void RefreshCaravanList();
	
	// 商队Entry被点击
	void OnCaravanEntryClicked(UUnitEntryWidget01* Entry);
	
	// 获取当前选中的商队
	UFUNCTION(BlueprintPure, Category = "Town|Caravans")
	AMoveableEntity01* GetSelectedCaravan() const;
	
	// 获取当前选中的商队ID
	UFUNCTION(BlueprintPure, Category = "Town|Caravans")
	int32 GetSelectedCaravanID() const;
	
	// ===== 殖民者管理 =====
	
	// 刷新殖民者列表
	void RefreshColonistList();
	
	// 殖民者Entry被点击
	void OnColonistEntryClicked(UUnitEntryWidget01* Entry);
	
	// 获取当前选中的殖民者
	UFUNCTION(BlueprintPure, Category = "Town|Colonists")
	AMoveableEntity01* GetSelectedColonist() const;
	
	// 获取当前选中的殖民者ID
	UFUNCTION(BlueprintPure, Category = "Town|Colonists")
	int32 GetSelectedColonistID() const;
	
	// 供TownInfoPanel调用的接口
	void HandleCaravanEntryClicked(UUnitEntryWidget01* Entry) { OnCaravanEntryClicked(Entry); }
	void HandleColonistEntryClicked(UUnitEntryWidget01* Entry) { OnColonistEntryClicked(Entry); }
	
	// ===== 基础资源展示方法 =====
	
	// 刷新基础资源图标显示
	void RefreshNativeResourcesIcons();
	
	// 获取城镇的基础资源ID列表
	TArray<FString> GetTownNativeResourceIDs() const;
	
	// 获取资源图标纹理
	UTexture2D* GetResourceIcon(const FString& ResourceID) const;
	
	// 获取资源显示名称
	FText GetResourceDisplayName(const FString& ResourceID) const;

protected:
	virtual void NativeConstruct() override;

	// ===== 商队列表UI =====
	
	// 购买商队按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyCaravanButton;
	
	// 商队列表容器
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UScrollBox> CaravanListScrollBox;
	
	// 当前选中的商队Entry
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UUnitEntryWidget01> SelectedCaravanUnitEntry;
	
	// ===== 殖民者列表UI =====
	
	// 购买殖民者按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyColonistButton;
	
	// 殖民者列表容器
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UScrollBox> ColonistListScrollBox;
	
	// 当前选中的殖民者Entry
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UUnitEntryWidget01> SelectedColonistEntry;
	
	// ===== 基础资源展示 =====
	
	// 基础资源标签
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TB_NativeResourcesLabel;
	
	// 基础资源图标容器
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HB_NativeResources;
	
	// 存储创建的图标控件
	UPROPERTY()
	TArray<TObjectPtr<UImage>> NativeResourceIcons;
	
	// 存储创建的SizeBox容器
	UPROPERTY()
	TArray<TObjectPtr<USizeBox>> NativeResourceSizeBoxes;
	
	// ===== 按钮事件 =====
	
	// 购买商队按钮点击事件
	UFUNCTION()
	void OnBuyCaravanButtonClicked();
	
	// 购买殖民者按钮点击事件
	UFUNCTION()
	void OnBuyColonistButtonClicked();
};