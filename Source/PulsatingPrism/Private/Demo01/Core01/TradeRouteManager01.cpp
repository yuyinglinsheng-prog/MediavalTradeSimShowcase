// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/TradeRouteManager01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Core01/MarketManager01.h"
#include "Demo01/Core01/PathfindingService01.h"
#include "Demo01/Core01/UnitControlManager01.h"
#include "Demo01/Actors/TownActor01.h"

// ===== 生命周期 =====

void UTradeRouteManager01::Initialize(ADemo01_GM* InGameMode)
{
	GameMode = InGameMode;
	
	if (GameMode)
	{
		PlayerState = GameMode->GetWorld()->GetFirstPlayerController()->GetPlayerState<ADemo01_PS>();
		MarketManager = GameMode->GetMarketManager();
		PathfindingService = GameMode->GetPathfindingService();
		UnitControlManager = GameMode->GetUnitControlManager();
	}
	
	AutoTradeCaravanIDs.Empty();
}

void UTradeRouteManager01::UpdateTradeRoutes(float DeltaDays)
{
	if (!PlayerState) return;
	
	// 运行时检查并缓存MarketManager（解决初始化时序问题）
	if (!MarketManager && GameMode)
	{
		MarketManager = GameMode->GetMarketManager();
	}
	if (!MarketManager) return;
	
	// 遍历所有自动贸易商队
	for (int32 CaravanID : AutoTradeCaravanIDs)
	{
		FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
		if (!CaravanData || !CaravanData->bIsAutoTrading) continue;
		
		// 添加详细状态日志
		UE_LOG(LogTemp, Verbose, TEXT("[TradeRoute] 处理商队 %d - 状态:%d 城镇:%d 路线索引:%d 路径长度:%d"),
			CaravanID, 
			(int32)CaravanData->State, 
			CaravanData->CurrentTownID,
			CaravanData->TradeRoute.CurrentVisitIndex,
			CaravanData->TradeRoute.TownVisitOrder.Num());
		
		// 检查是否在城镇中
		if (CaravanData->CurrentTownID == -1) continue;
		
		// 检查是否在路线中的城镇
		if (!IsCaravanAtRouteTown(CaravanID)) continue;
		
		// 执行贸易决策
		ExecuteTradeDecision(CaravanID);
		
		// 发送到下一个城镇
		SendCaravanToNextTown(CaravanID);
	}
}

// ===== 路线管理（UI调用） =====

void UTradeRouteManager01::SetCaravanTradeRoute(int32 CaravanID, const TArray<int32>& TownVisitOrder)
{
	if (!PlayerState) return;
	
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData) return;
	
	// 设置贸易路线数据
	CaravanData->TradeRoute.TownVisitOrder = TownVisitOrder;
	CaravanData->TradeRoute.CurrentVisitIndex = 0;
	CaravanData->TradeRoute.bIsActive = true;
	
	// 计算完整路径
	CalculateFullPath(CaravanID);
	
	// 注册为自动贸易商队
	RegisterAutoTradeCaravan(CaravanID);
}

void UTradeRouteManager01::ClearCaravanTradeRoute(int32 CaravanID)
{
	if (!PlayerState) return;
	
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData) return;
	
	// 清空贸易路线数据
	CaravanData->TradeRoute.TownVisitOrder.Empty();
	CaravanData->TradeRoute.FullPathCoords.Empty();
	CaravanData->TradeRoute.CurrentVisitIndex = 0;
	CaravanData->TradeRoute.bIsActive = false;
	CaravanData->TradeRoute.TotalPathLength = 0;
	CaravanData->TradeRoute.RouteSegmentValidity.Empty();
	CaravanData->TradeRoute.bIsRouteValid = false;
	
	// 注销自动贸易商队
	UnregisterAutoTradeCaravan(CaravanID);
}

void UTradeRouteManager01::SetCaravanAutoTrading(int32 CaravanID, bool bEnable)
{
	if (!PlayerState) return;
	
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData) return;
	
	CaravanData->bIsAutoTrading = bEnable;
	
	if (bEnable)
	{
		RegisterAutoTradeCaravan(CaravanID);
	}
	else
	{
		UnregisterAutoTradeCaravan(CaravanID);
	}
}

// ===== 查询接口（UI调用） =====

TArray<FCaravanData01> UTradeRouteManager01::GetAllAutoTradeCaravans() const
{
	TArray<FCaravanData01> Result;
	
	if (!PlayerState) return Result;
	
	for (int32 CaravanID : AutoTradeCaravanIDs)
	{
		const FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
		if (CaravanData && CaravanData->bIsAutoTrading)
		{
			Result.Add(*CaravanData);
		}
	}
	
	return Result;
}

FTradeRouteConfig UTradeRouteManager01::GetCaravanTradeRoute(int32 CaravanID) const
{
	if (!PlayerState) return FTradeRouteConfig();
	
	const FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (CaravanData)
	{
		return CaravanData->TradeRoute;
	}
	
	return FTradeRouteConfig();
}

int32 UTradeRouteManager01::GetRouteTotalLength(int32 CaravanID) const
{
	if (!PlayerState) return 0;
	
	const FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (CaravanData)
	{
		return CaravanData->TradeRoute.TotalPathLength;
	}
	
	return 0;
}

TArray<FString> UTradeRouteManager01::GetRouteTownNames(int32 CaravanID) const
{
	TArray<FString> Result;
	
	if (!PlayerState) return Result;
	
	const FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData) return Result;
	
	for (int32 TownID : CaravanData->TradeRoute.TownVisitOrder)
	{
		const FTownData01* TownData = PlayerState->GetTownDataPtr(TownID);
		if (TownData)
		{
			Result.Add(TownData->EntityName);
		}
		else
		{
			Result.Add(TEXT("未知城镇"));
		}
	}
	
	return Result;
}

TArray<FIntVector2> UTradeRouteManager01::GetTradeRoutePath(int32 CaravanID) const
{
	TArray<FIntVector2> Result;
	
	if (!PlayerState || !GameMode) return Result;
	
	const FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData) return Result;
	
	// 获取贸易路线中的城镇访问顺序
	const TArray<int32>& TownVisitOrder = CaravanData->TradeRoute.TownVisitOrder;
	
	if (TownVisitOrder.Num() < 2)
	{
		// 至少需要2个城镇才能形成路线
		return Result;
	}
	
	// 转换为网格坐标（闭环：最后一个点会连回第一个点）
	for (int32 TownID : TownVisitOrder)
	{
		const FTownData01* TownData = PlayerState->GetTownDataPtr(TownID);
		if (TownData)
		{
			Result.Add(TownData->GridCoord);
		}
	}
	
	// 确保闭环（如果TradeRouteManager没有自动添加的话）
	/*if (Result.Num() > 0 && Result.Last() != Result[0])
	{
		Result.Add(Result[0]);
	}*/
	
	return Result;
}

// ===== 内部方法 =====

void UTradeRouteManager01::ExecuteTradeDecision(int32 CaravanID)
{
	if (!PlayerState) return;
	
	// 运行时检查并缓存MarketManager（解决初始化时序问题）
	if (!MarketManager && GameMode)
	{
		MarketManager = GameMode->GetMarketManager();
	}
	if (!MarketManager) return;
	
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData || !CaravanData->bIsAutoTrading) return;
	
	// 检查是否在城镇中
	if (CaravanData->CurrentTownID == -1) return;
	
	// 检查是否在路线中的城镇
	if (!IsCaravanAtRouteTown(CaravanID)) return;
	
	// 执行交易
	ExecuteTradeAtTown(CaravanID, CaravanData->CurrentTownID);
}

void UTradeRouteManager01::ExecuteTradeAtTown(int32 CaravanID, int32 TownID)
{
	if (!PlayerState) return;
	
	// 运行时检查并缓存MarketManager（解决初始化时序问题）
	if (!MarketManager && GameMode)
	{
		MarketManager = GameMode->GetMarketManager();
	}
	if (!MarketManager) return;
	
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData) return;
	
	// 计算最优交易方案
	FTradePlan Plan = MarketManager->CalculateOptimalTrade(TownID, CaravanID, 0.1f);
	
	if (!Plan.bIsExecutable)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[贸易路线] 商队 %d 在城镇 %d 无法执行交易: %s"),
			CaravanID, TownID, *Plan.FailureReason);
		return;
	}
	
	float TotalMoneyChange = 0.0f;
	
	// 执行购买
	for (const auto& BuyPair : Plan.BuyPlan)
	{
		float Cost = MarketManager->ExecutePurchase(TownID, CaravanID, BuyPair.Key, BuyPair.Value);
		TotalMoneyChange -= Cost;
	}
	
	// 执行卖出
	for (const auto& SellPair : Plan.SellPlan)
	{
		float Revenue = MarketManager->ExecuteSale(TownID, CaravanID, SellPair.Key, SellPair.Value);
		TotalMoneyChange += Revenue;
	}
	
	// 更新金钱变化追踪
	if (CaravanData->TradeRoute.bIsTrackingMoneyChange)
	{
		CaravanData->TradeRoute.RouteMoneyChange += TotalMoneyChange;
	}
	
	// 检查是否完成循环
	if (CaravanData->TradeRoute.bIsTrackingMoneyChange && 
		TownID == CaravanData->TradeRoute.TrackingStartTownID &&
		!CaravanData->TradeRoute.bHasCompletedLoop)
	{
		CaravanData->TradeRoute.bHasCompletedLoop = true;
		
		// 广播事件，通知UI更新
		OnTradeRouteMoneyChange.Broadcast(CaravanID, CaravanData->TradeRoute.RouteMoneyChange);
		
		UE_LOG(LogTemp, Log, TEXT("[贸易路线] 商队 %d 完成循环，金钱变化: %.2f 金币"),
			CaravanID, CaravanData->TradeRoute.RouteMoneyChange);
		
		// 重置追踪（为下一次循环准备）
		CaravanData->TradeRoute.RouteMoneyChange = 0.0f;
	}
}

void UTradeRouteManager01::SendCaravanToNextTown(int32 CaravanID)
{
	if (!PlayerState || !UnitControlManager) return;
	
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData || !CaravanData->bIsAutoTrading) return;
	
	int32 NextTownID = GetNextTownInRoute(CaravanID);
	if (NextTownID == -1) return;
	
	// 获取目标城镇坐标
	const FTownData01* TownData = PlayerState->GetTownDataPtr(NextTownID);
	if (!TownData) return;
	
	// 通过UCM发送移动命令
	UnitControlManager->HandleMoveCommandByData(CaravanID, EEntityType01::Caravan, TownData->GridCoord, false);
	
	// 更新路线索引
	int32 CurrentIndex = CaravanData->TradeRoute.CurrentVisitIndex;
	int32 NextIndex = (CurrentIndex + 1) % CaravanData->TradeRoute.TownVisitOrder.Num();
	CaravanData->TradeRoute.CurrentVisitIndex = NextIndex;
	
	// 如果是第一次移动，开始金钱追踪
	if (!CaravanData->TradeRoute.bIsTrackingMoneyChange)
	{
		CaravanData->TradeRoute.bIsTrackingMoneyChange = true;
		CaravanData->TradeRoute.TrackingStartTownID = NextTownID;
		CaravanData->TradeRoute.RouteMoneyChange = 0.0f;
		CaravanData->TradeRoute.bHasCompletedLoop = false;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[贸易路线] 商队 %d 正在前往下一个城镇: %s"),
		CaravanID, *TownData->EntityName);
}

void UTradeRouteManager01::CalculateFullPath(int32 CaravanID)
{
	if (!PlayerState || !PathfindingService) return;
	
	FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData || CaravanData->TradeRoute.TownVisitOrder.Num() < 2) return;
	
	TArray<FIntVector2>& FullPath = CaravanData->TradeRoute.FullPathCoords;
	FullPath.Empty();
	
	// 遍历所有城镇，计算每段路径
	for (int32 i = 0; i < CaravanData->TradeRoute.TownVisitOrder.Num(); i++)
	{
		int32 CurrentTownID = CaravanData->TradeRoute.TownVisitOrder[i];
		int32 NextIndex = (i + 1) % CaravanData->TradeRoute.TownVisitOrder.Num();
		int32 NextTownID = CaravanData->TradeRoute.TownVisitOrder[NextIndex];
		
		// 获取城镇坐标
		const FTownData01* CurrentTownData = PlayerState->GetTownDataPtr(CurrentTownID);
		const FTownData01* NextTownData = PlayerState->GetTownDataPtr(NextTownID);
		
		if (!CurrentTownData || !NextTownData) continue;
		
		// 调用寻路服务
		TArray<FIntVector2> SegmentPath = PathfindingService->FindPath(
			CurrentTownData->GridCoord, 
			NextTownData->GridCoord
		);
		
		// 添加到完整路径（除第一段外，去掉起点避免重复）
		if (i > 0 && SegmentPath.Num() > 0)
		{
			SegmentPath.RemoveAt(0);
		}
		
		FullPath.Append(SegmentPath);
	}
	
	// 更新总长度
	CaravanData->TradeRoute.TotalPathLength = FullPath.Num();
}

bool UTradeRouteManager01::IsCaravanAtRouteTown(int32 CaravanID) const
{
	if (!PlayerState) return false;
	
	const FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData || !CaravanData->bIsAutoTrading) return false;
	
	// 检查是否在城镇中
	if (CaravanData->CurrentTownID == -1) return false;
	
	// 检查当前城镇是否在路线中
	for (int32 TownID : CaravanData->TradeRoute.TownVisitOrder)
	{
		if (TownID == CaravanData->CurrentTownID)
		{
			return true;
		}
	}
	
	return false;
}

int32 UTradeRouteManager01::GetNextTownInRoute(int32 CaravanID) const
{
	if (!PlayerState) return -1;
	
	const FCaravanData01* CaravanData = PlayerState->GetCaravanDataPtr(CaravanID);
	if (!CaravanData || CaravanData->TradeRoute.TownVisitOrder.Num() == 0) return -1;
	
	int32 CurrentIndex = CaravanData->TradeRoute.CurrentVisitIndex;
	int32 NextIndex = (CurrentIndex + 1) % CaravanData->TradeRoute.TownVisitOrder.Num();
	
	return CaravanData->TradeRoute.TownVisitOrder[NextIndex];
}

void UTradeRouteManager01::RegisterAutoTradeCaravan(int32 CaravanID)
{
	AutoTradeCaravanIDs.AddUnique(CaravanID);
}

void UTradeRouteManager01::UnregisterAutoTradeCaravan(int32 CaravanID)
{
	AutoTradeCaravanIDs.Remove(CaravanID);
}

void UTradeRouteManager01::RecalculateRoute(int32 CaravanID)
{
	// 1. 获取商队数据
	FCaravanData01* CaravanData = PlayerState ? PlayerState->GetCaravanDataPtr(CaravanID) : nullptr;
	if (!CaravanData || CaravanData->TradeRoute.TownVisitOrder.Num() < 2)
	{
		// 清空路径
		if (CaravanData)
		{
			CaravanData->TradeRoute.FullPathCoords.Empty();
			CaravanData->TradeRoute.TotalPathLength = 0;
			CaravanData->TradeRoute.RouteSegmentValidity.Empty();
			CaravanData->TradeRoute.bIsRouteValid = false;
		}
		return;
	}

	// 2. 检查PathfindingService
	if (!PathfindingService)
	{
		UE_LOG(LogTemp, Error, TEXT("[TradeRouteManager] 无法获取PathfindingService"));
		return;
	}

	// 3. 获取所有城镇坐标
	TArray<FIntVector2> TownCoords;
	for (int32 TownID : CaravanData->TradeRoute.TownVisitOrder)
	{
		FTownData01 TownData = PlayerState->GetTownData(TownID);
		TownCoords.Add(TownData.GridCoord);
	}

	// 4. 计算所有路径段（强制循环路线）
	TArray<FIntVector2> CompletePath;
	TArray<bool> SegmentValidity;
	bool bAllSegmentsValid = true;
	
	// 强制循环：段数 = 城镇数（最后一段回到起点）
	int32 SegmentCount = TownCoords.Num();

	for (int32 i = 0; i < SegmentCount; i++)
	{
		int32 NextIndex = (i + 1) % TownCoords.Num();  // 循环支持
		
		// 检查是否为同一点（零长度路径）
		if (TownCoords[i] == TownCoords[NextIndex])
		{
			UE_LOG(LogTemp, Warning, TEXT("[TradeRouteManager] 零长度路径段: %d→%d (同一坐标)"),
				CaravanData->TradeRoute.TownVisitOrder[i],
				CaravanData->TradeRoute.TownVisitOrder[NextIndex]);
			
			// 标记为无效
			SegmentValidity.Add(false);
			bAllSegmentsValid = false;
			
			// 不调用 FindPath，跳过此段
			continue;
		}
		
		// 计算单段路径
		TArray<FIntVector2> SegmentPath = PathfindingService->FindPath(
			TownCoords[i], 
			TownCoords[NextIndex]
		);
		
		// 检查是否可达
		bool bSegmentValid = (SegmentPath.Num() > 0);
		SegmentValidity.Add(bSegmentValid);
		
		if (!bSegmentValid)
		{
			bAllSegmentsValid = false;
			UE_LOG(LogTemp, Warning, TEXT("[TradeRouteManager] 路径段无效: %d→%d"),
				CaravanData->TradeRoute.TownVisitOrder[i],
				CaravanData->TradeRoute.TownVisitOrder[NextIndex]);
		}
		
		// 添加路径段（连接处去重）
		for (int32 j = 0; j < SegmentPath.Num(); j++)
		{
			if (i == 0 || j > 0)  // 第一段保留所有点，后续段跳过起点
			{
				CompletePath.Add(SegmentPath[j]);
			}
		}
	}

	// 5. 更新路线数据
	CaravanData->TradeRoute.FullPathCoords = CompletePath;
	CaravanData->TradeRoute.TotalPathLength = CompletePath.Num();
	CaravanData->TradeRoute.RouteSegmentValidity = SegmentValidity;
	CaravanData->TradeRoute.bIsRouteValid = bAllSegmentsValid;

	// 6. 记录日志
	int32 ValidCount = 0;
	for (bool bValid : SegmentValidity)
	{
		if (bValid) ValidCount++;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[TradeRouteManager] 路线重算完成：%d个城镇，路径长度%d，有效段%d/%d"),
		TownCoords.Num(),
		CompletePath.Num(),
		ValidCount,
		SegmentValidity.Num());
	
	// Debug: 打印完整路径坐标
	FString PathCoordsStr = TEXT("路径坐标: ");
	for (int32 i = 0; i < CompletePath.Num(); i++)
	{
		PathCoordsStr += FString::Printf(TEXT("(%d,%d) "), CompletePath[i].X, CompletePath[i].Y);
		if (i > 0 && i % 10 == 0)  // 每10个点换行，避免日志过长
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *PathCoordsStr);
			PathCoordsStr = TEXT("        ");
		}
	}
	if (!PathCoordsStr.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *PathCoordsStr);
	}
}
