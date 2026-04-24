// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "MarketManager01.generated.h"

/**
 * 市场运行时商品数据
 * 基于FProductData01创建，添加动态市场信息
 */
USTRUCT(BlueprintType)
struct FCommodityRuntime
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	int32 ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	float BasicPrice;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	int32 BasicNumber;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	float CurrentPrice;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	int32 CurrentNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commodity")
	float Elasticity; 

	FCommodityRuntime()
	{
		ID = 0;
		Name = "Default";
		BasicPrice = 0.0f;
		BasicNumber = 0;
		CurrentPrice = 0.0f;
		CurrentNumber = 0;
		Elasticity = 1.0f;
	}

	// 从FProductData01初始化
	void InitializeFromProductData(const FProductData01& ProductData, int32 NewID, int32 InitialQuantity)
	{
		ID = NewID;
		Name = ProductData.ProductID;
		BasicPrice = static_cast<float>(ProductData.BaseValue);
		BasicNumber = InitialQuantity;
		CurrentPrice = BasicPrice; // 初始化为基础价格
		CurrentNumber = InitialQuantity; // 初始化为基础数量
		Elasticity = ProductData.Elasticity;
	}
};

/**
 * 市场管理器 - Demo01专用
 * 无状态定价服务：输入定价相关参数，输出计算价格
 * 不保存任何状态（玩家金钱由 Demo01_PS 管理）
 */
UCLASS(BlueprintType)
class PULSATINGPRISM_API UMarketManager01 : public UObject
{
	GENERATED_BODY()
	
public:
	// ==============================================
	// 定价计算（当前为stub，直接返回基础价格）
	// ==============================================

	/**
	 * 计算市场价格
	 * @param BasePrice      商品基础价格
	 * @param BaseQuantity   基准库存数量（用于供需比较）
	 * @param CurrentQuantity 当前实际库存数量
	 * @param Elasticity     价格弹性系数
	 * @return 计算得出的当前市场价格
	 */
	UFUNCTION(BlueprintCallable, Category = "Market")
	float CalculateMarketPrice(float BasePrice, int32 BaseQuantity, int32 CurrentQuantity, float Elasticity) const;
	
	// ==============================================
	// 贸易优化计算
	// ==============================================
	
	/**
	 * 计算在当前城镇的最优交易方案
	 * @param TownID 当前城镇ID
	 * @param CaravanID 商队ID
	 * @param MinPriceRatio 最小价格偏差比例（默认0.1=10%）
	 * @return 交易方案（包含买卖清单和预计金钱变化）
	 */
	UFUNCTION(BlueprintCallable, Category = "TradeOptimization")
	FTradePlan CalculateOptimalTrade(int32 TownID, int32 CaravanID, float MinPriceRatio = 0.1f) const;
	
	/**
	 * 执行购买操作
	 * @param TownID 城镇ID
	 * @param CaravanID 商队ID
	 * @param ResourceID 商品ID
	 * @param Amount 购买数量
	 * @return 实际花费的金币（用于记录金钱变化）
	 */
	UFUNCTION(BlueprintCallable, Category = "TradeExecution")
	float ExecutePurchase(int32 TownID, int32 CaravanID, const FString& ResourceID, int32 Amount);
	
	/**
	 * 执行卖出操作
	 * @param TownID 城镇ID
	 * @param CaravanID 商队ID
	 * @param ResourceID 商品ID
	 * @param Amount 卖出数量
	 * @return 实际获得的金币（用于记录金钱变化）
	 */
	UFUNCTION(BlueprintCallable, Category = "TradeExecution")
	float ExecuteSale(int32 TownID, int32 CaravanID, const FString& ResourceID, int32 Amount);
	
	// ==============================================
	// 辅助查询函数
	// ==============================================
	
	/**
	 * 获取商品当前价格
	 * @param ResourceID 商品ID
	 * @param TownID 城镇ID
	 * @return 当前价格
	 */
	UFUNCTION(BlueprintPure, Category = "PriceQuery")
	float GetCurrentPrice(const FString& ResourceID, int32 TownID) const;
	
	/**
	 * 获取商品基础价格
	 * @param ResourceID 商品ID
	 * @return 基础价格
	 */
	UFUNCTION(BlueprintPure, Category = "PriceQuery")
	float GetProductBasePrice(const FString& ResourceID) const;
	
	/**
	 * 获取商品重量
	 * @param ResourceID 商品ID
	 * @return 每单位重量
	 */
	UFUNCTION(BlueprintPure, Category = "ProductInfo")
	float GetProductWeight(const FString& ResourceID) const;
};
