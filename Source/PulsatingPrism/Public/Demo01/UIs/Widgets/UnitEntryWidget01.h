// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "UnitEntryWidget01.generated.h"

class UTextBlock;
class UBorder;
class UButton;
class UTownInfoPanel;
class UTownInfoPage;
class UDemo01StyleManager;

/**
 * 单位类型枚举
 * 用于区分商队和殖民者
 */
UENUM(BlueprintType)
enum class EUnitType01 : uint8
{
	Caravan     UMETA(DisplayName = "Caravan"),
	Colonist    UMETA(DisplayName = "Colonist")
};

/**
 * 通用单位列表项 Widget
 * 可以显示商队或殖民者的信息，可以被选中
 */
UCLASS()
class PULSATINGPRISM_API UUnitEntryWidget01 : public UDemo01UIBase
{
	GENERATED_BODY()
	
public:
	// 设置单位数据（商队）
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void SetCaravanData(int32 CaravanID, UTownInfoPanel* OwnerPanel);
	
	// 禁用自动样式（动态创建时Slate层未就绪）
	virtual bool ShouldAutoApplyStyle_Implementation() const override { return false; }
	
	// 设置单位数据（殖民者）
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void SetColonistData(int32 ColonistID, UTownInfoPanel* OwnerPanel);
	
	// 通用设置单位数据
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void SetUnitData(int32 UnitID, const FString& UnitName, EUnitType01 UnitType, UTownInfoPanel* OwnerPanel);
	
	// 初始化单位数据（延迟调用）
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void InitializeUnitData();
	
	// 获取单位ID
	UFUNCTION(BlueprintPure, Category = "Unit")
	int32 GetUnitID() const { return UnitIDRef; }
	
	// 获取单位类型
	UFUNCTION(BlueprintPure, Category = "Unit")
	EUnitType01 GetUnitType() const { return UnitType; }
	
	// 设置选中状态
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void SetSelected(bool bSelected);
	
	// 是否被选中
	UFUNCTION(BlueprintPure, Category = "Unit")
	bool IsSelected() const { return bIsSelected; }

protected:
	virtual void NativeConstruct() override;
	
	// ===== UI 元素（在蓝图中绑定） =====
	
	// 单位名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> UnitNameText;
	
	// 单位类型文本（可选，用于显示"商队"或"殖民者"）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> UnitTypeText;
	
	// 点击按钮（覆盖整个entry）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ClickButton;
	
	// ===== 数据 =====
	
	// 单位ID引用
	UPROPERTY()
	int32 UnitIDRef = -1;
	
	// 单位类型
	UPROPERTY()
	EUnitType01 UnitType = EUnitType01::Caravan;
	
	// 所属面板
	UPROPERTY()
	TObjectPtr<UTownInfoPanel> OwnerPanelRef;
	
	// 是否被选中
	bool bIsSelected = false;
	
	// ===== 事件 =====
	
	// 选中状态改变事件（蓝图实现）
	UFUNCTION(BlueprintImplementableEvent, Category = "Unit")
	void OnSelectionChanged(bool bSelected);
	
	// 点击事件
	UFUNCTION()
	void OnClicked();
	
private:
	// 获取单位类型显示文本
	FString GetUnitTypeDisplayText() const;
};