// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TradeRouteManager01.generated.h"

// 前向声明
class ADemo01_GM;
class ADemo01_PS;
class UMarketManager01;
class UPathfindingService01;
class UUnitControlManager01;

// 委托：商队完成贸易路线时触发
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnTradeRouteMoneyChange,
	int32, CaravanID,
	float, MoneyChange
);

/**
 * 贸易路线管理器
 * 管理所有自动贸易商队的路线、决策和执行
 * 由TimeManager驱动（每天一次）
 */
UCLASS()
class PULSATINGPRISM_API UTradeRouteManager01 : public UObject
{
	GENERATED_BODY()
	
public:
	// ===== 委托事件 =====
	
	// 商队完成贸易路线时触发（用于UI更新）
	UPROPERTY(BlueprintAssignable, Category = "TradeRoute|Events")
	FOnTradeRouteMoneyChange OnTradeRouteMoneyChange;
	
	// ===== 生命周期 =====
	
	// 初始化（由GameMode调用）
	void Initialize(ADemo01_GM* InGameMode);
	
	// 由TimeManager::OnDayTick()调用（每天一次）
	void UpdateTradeRoutes(float DeltaDays);
	
	// ===== 路线管理（UI调用） =====
	
	// 为商队设置贸易路线
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void SetCaravanTradeRoute(int32 CaravanID, const TArray<int32>& TownVisitOrder);
	
	// 清除商队的贸易路线
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void ClearCaravanTradeRoute(int32 CaravanID);
	
	// 启用/禁用商队的自动贸易
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void SetCaravanAutoTrading(int32 CaravanID, bool bEnable);
	
	// 重新计算商队的贸易路线（在城镇添加/删除/交换后调用）
	UFUNCTION(BlueprintCallable, Category = "TradeRoute")
	void RecalculateRoute(int32 CaravanID);
	
	// ===== 查询接口（UI调用） =====
	
	// 获取所有自动贸易商队数据（用于UI显示路线列表）
	UFUNCTION(BlueprintCallable, Category = "TradeRoute|Query")
	TArray<FCaravanData01> GetAllAutoTradeCaravans() const;
	
	// 获取商队的贸易路线（用于UI编辑）
	UFUNCTION(BlueprintPure, Category = "TradeRoute|Query")
	FTradeRouteConfig GetCaravanTradeRoute(int32 CaravanID) const;
	
	// 计算路线总长度（用于UI显示）
	UFUNCTION(BlueprintPure, Category = "TradeRoute|Query")
	int32 GetRouteTotalLength(int32 CaravanID) const;
	
	// 获取路线中的城镇名称列表（用于UI显示）
	UFUNCTION(BlueprintPure, Category = "TradeRoute|Query")
	TArray<FString> GetRouteTownNames(int32 CaravanID) const;
	
	// 获取商队的贸易路线路径坐标（用于路径可视化）
	UFUNCTION(BlueprintPure, Category = "TradeRoute|Query")
	TArray<FIntVector2> GetTradeRoutePath(int32 CaravanID) const;
	
	// ===== 内部方法 =====
	
	// 执行单个商队的贸易决策
	void ExecuteTradeDecision(int32 CaravanID);
	
	// 在指定城镇执行买卖操作
	void ExecuteTradeAtTown(int32 CaravanID, int32 TownID);
	
	// 发送商队到路线中的下一个城镇
	void SendCaravanToNextTown(int32 CaravanID);
	
	// 计算并缓存完整路径（用于显示）
	void CalculateFullPath(int32 CaravanID);
	
	// 检查商队是否在路线中的某个城镇
	bool IsCaravanAtRouteTown(int32 CaravanID) const;
	
	// 获取路线中的下一个城镇ID
	int32 GetNextTownInRoute(int32 CaravanID) const;
	
	// 注册/注销自动贸易商队（内部使用）
	void RegisterAutoTradeCaravan(int32 CaravanID);
	void UnregisterAutoTradeCaravan(int32 CaravanID);
	
private:
	// 缓存：GameMode引用
	UPROPERTY()
	ADemo01_GM* GameMode = nullptr;
	
	// 缓存：PlayerState指针（快速访问）
	UPROPERTY()
	ADemo01_PS* PlayerState = nullptr;
	
	// 缓存：相关服务
	UPROPERTY()
	UMarketManager01* MarketManager = nullptr;
	
	UPROPERTY()
	UPathfindingService01* PathfindingService = nullptr;
	
	UPROPERTY()
	UUnitControlManager01* UnitControlManager = nullptr;
	
	// 缓存：自动贸易商队ID列表（优化性能，避免遍历所有商队）
	// 注意：这个缓存需要在商队删除时同步更新
	TArray<int32> AutoTradeCaravanIDs;
};
