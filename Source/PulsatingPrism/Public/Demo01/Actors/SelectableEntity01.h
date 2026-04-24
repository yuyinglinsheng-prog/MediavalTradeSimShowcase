// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "SelectableEntity01.generated.h"

class ADemo01_GM;

/**
 * 可选中实体基类
 * 提供网格坐标管理、选中状态管理、点击检测等基础功能
 */
UCLASS(Abstract)
class PULSATINGPRISM_API ASelectableEntity01 : public AActor
{
	GENERATED_BODY()
	
public:	
	ASelectableEntity01();

	// ===== 网格坐标管理 =====
	
	// 获取网格坐标
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FIntVector2 GetGridCoord() const { return GridCoord; }
	
	// 设置网格坐标（会同步更新世界位置）
	UFUNCTION(BlueprintCallable, Category = "Grid")
	virtual void SetGridCoord(FIntVector2 NewCoord);
	
	// 初始化网格坐标（不更新世界位置，用于spawn时设置）
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void InitializeGridCoord(FIntVector2 NewCoord);
	
	// ===== 选中状态管理 =====
	
	// 设置选中状态
	UFUNCTION(BlueprintCallable, Category = "Selection")
	virtual void SetSelected(bool bSelected);
	
	// 是否被选中
	UFUNCTION(BlueprintPure, Category = "Selection")
	bool IsSelected() const { return bIsSelected; }
	
	// ===== 点击检测 =====
	
	// 点击事件回调（由 PlayerController 调用）
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void OnClicked();
	
	// ===== 数据容器 =====
	
	// 实体 ID（由子类定义具体含义）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	int32 EntityID = 0;
	
	// 实体名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	FString EntityName = TEXT("Entity");

protected:
	virtual void BeginPlay() override;
	
	// 网格坐标到世界坐标的转换（通过 GameMode 的 GridService）
	FVector GridCoordToWorld(FIntVector2 Coord) const;
	
	// 获取 GameMode 引用
	ADemo01_GM* GetDemo01GameMode() const;
	
	// 调整Z轴位置以贴合地面
	FVector AdjustZPosition(const FVector& WorldPosition) const;
	
	// 网格坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FIntVector2 GridCoord = FIntVector2::ZeroValue;
	
	// 是否被选中
	UPROPERTY(BlueprintReadOnly, Category = "Selection")
	bool bIsSelected = false;
	
	// 静态网格组件（占位符，可在蓝图中配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 选中高亮组件（可选，可在蓝图中配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HighlightComponent;
};
