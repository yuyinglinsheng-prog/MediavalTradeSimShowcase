// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01PanelBase.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "ColonistInfoPanel01.generated.h"

class UTextBlock;
class UButton;
class AMoveableEntity01;

/**
 * 殖民者信息面板
 * 显示殖民者基本信息和操作按钮（建城、删除）
 */
UCLASS()
class PULSATINGPRISM_API UColonistInfoPanel01 : public UDemo01PanelBase
{
	GENERATED_BODY()

public:
	// 初始化面板（有实例版本）
	UFUNCTION(BlueprintCallable, Category = "Colonist Panel")
	void InitializePanel(AMoveableEntity01* InColonist);
	
	// 初始化面板（无实例版本，使用数据）
	UFUNCTION(BlueprintCallable, Category = "Colonist Panel")
	void InitializePanelWithData(const FColonistData01& InColonistData);
	
	// 初始化UI（UIManager调用）
	virtual void InitializeUI(class ADemo01_GM* InGameMode) override;
	
	// 隐藏UI
	void HideUI();
	
	// 刷新面板数据
	UFUNCTION(BlueprintCallable, Category = "Colonist Panel")
	void RefreshPanel();

	// 获取当前殖民者
	UFUNCTION(BlueprintPure, Category = "Colonist Panel")
	AMoveableEntity01* GetColonist() const { return Colonist; }

	// 获取殖民者数据
	UFUNCTION(BlueprintPure, Category = "Colonist Panel")
	FColonistData01 GetColonistData() const { return ColonistData; }

protected:
	virtual void NativeConstruct() override;

	// ===== UI组件绑定 =====
	
	// 殖民者名称文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ColonistNameText = nullptr;
	
	// 殖民者状态文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ColonistStatusText = nullptr;
	
	// 殖民者位置文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ColonistLocationText = nullptr;
	
	// 建立新城市按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FoundCityButton = nullptr;
	
	// 删除殖民者按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteColonistButton = nullptr;
	
	// 关闭按钮
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton = nullptr;

	// ===== 数据 =====
	
	// 关联的殖民者（可能为空，如果殖民者在城镇中）
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> Colonist = nullptr;
	
	// 殖民者数据（始终有效）
	UPROPERTY()
	FColonistData01 ColonistData;
	
	// ===== 事件处理 =====

	// 建立新城市按钮点击
	UFUNCTION()
	void OnFoundCityClicked();

	// 删除殖民者按钮点击
	UFUNCTION()
	void OnDeleteColonistClicked();
	
	// 关闭按钮点击
	UFUNCTION()
	void OnCloseClicked();

private:
	// 更新殖民者基本信息显示
	void UpdateColonistInfo();

	// 获取殖民者状态文本
	FText GetColonistStatusText() const;
	
	// 获取殖民者位置文本
	FText GetColonistLocationText() const;

public:
	// 更新按钮状态
	void UpdateButtonStates();

private:
	// 获取当前所在城镇名称（如果在城镇中）
	FString GetCurrentTownName() const;

public:
	// ===== 蓝图事件 =====
	
	// 面板关闭事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Colonist Panel")
	void OnPanelClosed();
	
	// 建城确认事件（供蓝图重写，可用于显示确认对话框）
	UFUNCTION(BlueprintImplementableEvent, Category = "Colonist Panel")
	void OnFoundCityConfirm();
	
	// 删除确认事件（供蓝图重写，可用于显示确认对话框）
	UFUNCTION(BlueprintImplementableEvent, Category = "Colonist Panel")
	void OnDeleteColonistConfirm();
	
	// 建城成功事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Colonist Panel")
	void OnCityFounded(const FString& CityName);
	
	// 殖民者删除成功事件（供蓝图重写）
	UFUNCTION(BlueprintImplementableEvent, Category = "Colonist Panel")
	void OnColonistDeleted();
};