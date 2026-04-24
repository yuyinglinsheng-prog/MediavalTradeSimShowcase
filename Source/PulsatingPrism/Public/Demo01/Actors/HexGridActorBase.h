// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexGridActorBase.generated.h"

// 前向声明
class UProductionManager01;
struct FProductData01;

/**
 * 六边形网格地块基类
 * 用于可视化显示地块资源，支持动态材质实例
 */
UCLASS(Abstract, Blueprintable)
class PULSATINGPRISM_API AHexGridActorBase : public AActor
{
	GENERATED_BODY()
	
public:
	AHexGridActorBase();

	// ===== 核心属性 =====

	// 地块资源类型（空字符串表示无资源）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FString ResourceType;

	// 网格坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FIntVector2 GridCoord;

	// ===== 视觉组件 =====

	// 六边形网格Mesh组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* HexMesh;

	// ===== 材质配置 =====

	// 基础材质（用于创建材质实例）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	class UMaterialInterface* BaseMaterial;

	// 默认图标（无资源时显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	class UTexture2D* DefaultIcon;

	// ===== 公共接口 =====

	// 更新视觉效果（根据资源类型设置材质参数）
	UFUNCTION(BlueprintCallable, Category = "Visual")
	virtual void UpdateVisuals();

	// 蓝图可重写的事件：当资源类型改变时调用
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnResourceTypeChanged();

protected:
	// 组件初始化
	virtual void BeginPlay() override;

	// 从生产管理器获取产品数据
	UFUNCTION(BlueprintPure, Category = "Data")
	bool GetProductData(const FString& InResourceType, FProductData01& OutProductData);

	// 获取当前GameMode
	UFUNCTION(BlueprintPure, Category = "Game")
	class ADemo01_GM* GetDemo01GameMode() const;
};
