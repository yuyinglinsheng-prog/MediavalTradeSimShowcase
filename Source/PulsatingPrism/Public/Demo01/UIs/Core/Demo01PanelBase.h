// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01/UIs/Core/Demo01UIBase.h"
#include "Demo01PanelBase.generated.h"

class UDemo01StyleManager;

/**
 * Demo01 面板基类
 * 所有弹窗面板的基类，提供拖拽、收起、关闭等功能
 */
UCLASS(Abstract)
class PULSATINGPRISM_API UDemo01PanelBase : public UDemo01UIBase
{
	GENERATED_BODY()
	
public:
	// 打开面板
	UFUNCTION(BlueprintCallable, Category = "Demo01|Panel")
	virtual void OpenPanel();
	
	// 关闭面板
	UFUNCTION(BlueprintCallable, Category = "Demo01|Panel")
	virtual void ClosePanel();
	
	// 收起面板（最小化到侧边栏）
	UFUNCTION(BlueprintCallable, Category = "Demo01|Panel")
	virtual void MinimizePanel();
	
	// 恢复面板
	UFUNCTION(BlueprintCallable, Category = "Demo01|Panel")
	virtual void RestorePanel();
	
	// 设置面板位置
	UFUNCTION(BlueprintCallable, Category = "Demo01|Panel")
	void SetPanelPosition(FVector2D Position);
	
	// 获取面板位置
	UFUNCTION(BlueprintPure, Category = "Demo01|Panel")
	FVector2D GetPanelPosition() const { return PanelPosition; }
	
	// 应用样式到面板
	UFUNCTION(BlueprintCallable, Category = "Demo01|Panel")
	virtual void ApplyPanelStyle();

protected:
	virtual void NativeConstruct() override;
	
	// 面板是否打开
	UPROPERTY(BlueprintReadOnly, Category = "Panel")
	bool bIsOpen = false;
	
	// 面板是否最小化
	UPROPERTY(BlueprintReadOnly, Category = "Panel")
	bool bIsMinimized = false;
	
	// 面板位置（记忆功能）
	UPROPERTY()
	FVector2D PanelPosition = FVector2D::ZeroVector;
	
	// ===== 拖拽功能（在蓝图中实现） =====
	
	// 开始拖拽
	UFUNCTION(BlueprintImplementableEvent, Category = "Panel")
	void OnStartDrag();
	
	// 拖拽中
	UFUNCTION(BlueprintImplementableEvent, Category = "Panel")
	void OnDragging(FVector2D DragDelta);
	
	// 结束拖拽
	UFUNCTION(BlueprintImplementableEvent, Category = "Panel")
	void OnEndDrag();
};
