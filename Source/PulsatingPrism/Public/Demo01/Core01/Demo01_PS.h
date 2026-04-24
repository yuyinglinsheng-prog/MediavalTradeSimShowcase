// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Demo01DataTypes.h"
#include "Demo01_PS.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, float, NewGold);

class ATownActor01;
class AMoveableEntity01;
class ASelectableEntity01;

/**
 * Demo01 PlayerState
 * 管理玩家的全局数据和资源（金钱、商队/城镇ID生成等）
 */
UCLASS()
class PULSATINGPRISM_API ADemo01_PS : public APlayerState
{
	GENERATED_BODY()
	
public:
	ADemo01_PS();
	
	// ===== 金钱管理 =====

	// 金钱数值变化时广播（新余额）
	UPROPERTY(BlueprintAssignable, Category = "Resources")
	FOnGoldChanged OnGoldChanged;
	
	// 获取当前金钱
	UFUNCTION(BlueprintPure, Category = "Resources")
	float GetGold() const { return Gold; }
	
	// 更新金钱（正数增加，负数减少）
	// 返回是否成功（如果减少金钱但余额不足则返回false）
	UFUNCTION(BlueprintCallable, Category = "Resources")
	bool UpdateGold(float Amount);
	
	// 添加金钱（简化接口，总是成功）
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void AddGold(float Amount);
	
	// 检查是否有足够的金钱
	UFUNCTION(BlueprintPure, Category = "Resources")
	bool HasEnoughGold(float Amount) const { return Gold >= Amount; }
	
	// ===== ID 生成管理 =====
	
	// 获取下一个全局实体ID（统一ID空间）
	UFUNCTION(BlueprintCallable, Category = "ID Generation")
	int32 GetNextEntityID();
	
	// 获取已创建的商队总数
	UFUNCTION(BlueprintPure, Category = "Statistics")
	int32 GetTotalCaravansCreated() const { return TotalCaravansCreated; }
	
	// 获取已创建的城镇总数
	UFUNCTION(BlueprintPure, Category = "Statistics")
	int32 GetTotalTownsCreated() const { return TotalTownsCreated; }
	
	// 获取已创建的殖民者总数
	UFUNCTION(BlueprintPure, Category = "Statistics")
	int32 GetTotalColonistsCreated() const { return TotalColonistsCreated; }
	
	// ===== 名称生成 =====
	
	// 生成商队名称（格式："商队1"、"商队2"）
	UFUNCTION(BlueprintCallable, Category = "Name Generation")
	FString GenerateCaravanName();
	
	// 生成城镇名称（格式："城镇1"、"城镇2"）
	UFUNCTION(BlueprintCallable, Category = "Name Generation")
	FString GenerateTownName();
	
	// 生成殖民者名称（格式："殖民者1"、"殖民者2"）
	UFUNCTION(BlueprintCallable, Category = "Name Generation")
	FString GenerateColonistName();
	
	// ===== 实体管理（统一接口） =====
	
	// 通过实体引用获取实例（可能返回nullptr）
	UFUNCTION(BlueprintCallable, Category = "Entity Management")
	ASelectableEntity01* GetEntityInstance(FEntityReference01 Ref);
	
	// 通过实体引用获取名称
	UFUNCTION(BlueprintPure, Category = "Entity Management")
	FString GetEntityName(FEntityReference01 Ref) const;
	
	// 通过实体引用获取坐标
	UFUNCTION(BlueprintPure, Category = "Entity Management")
	FIntVector2 GetEntityCoord(FEntityReference01 Ref) const;
	
	// ===== 商队管理 =====
	
	// 创建商队数据（不创建实例）
	UFUNCTION(BlueprintCallable, Category = "Caravan Management")
	int32 CreateCaravanData(int32 TownID);
	
	// 查找商队数据（返回拷贝，供蓝图使用）
	UFUNCTION(BlueprintPure, Category = "Caravan Management")
	FCaravanData01 GetCaravanData(int32 CaravanID) const;
	
	// 查找商队数据（返回指针，供C++直接操作，零拷贝）
	FCaravanData01* GetCaravanDataPtr(int32 CaravanID);
	
	// 更新商队数据
	UFUNCTION(BlueprintCallable, Category = "Caravan Management")
	bool UpdateCaravanData(int32 CaravanID, const FCaravanData01& NewData);
	
	// 获取所有商队ID
	UFUNCTION(BlueprintPure, Category = "Caravan Management")
	TArray<int32> GetAllCaravanIDs() const;
	
	// 根据ID查找商队实例（可能返回nullptr）
	AMoveableEntity01* FindCaravanInstance(int32 CaravanID) const;

	// 根据ID查找任意可移动实体实例（先查商队，再查殖民者）
	AMoveableEntity01* FindMoveableEntityInstance(int32 EntityID) const;
	
	// 设置商队实例
	void SetCaravanInstance(int32 CaravanID, AMoveableEntity01* Instance);
	
	// 移除商队数据
	UFUNCTION(BlueprintCallable, Category = "Caravan Management")
	bool RemoveCaravan(int32 CaravanID);
	
	// ===== 城镇管理 =====
	
	// 注册城镇（创建数据并关联实例）
	UFUNCTION(BlueprintCallable, Category = "Town Management")
	void RegisterTown(ATownActor01* Town);
	
	// 查找城镇数据
	UFUNCTION(BlueprintPure, Category = "Town Management")
	FTownData01 GetTownData(int32 TownID) const;

	// 查找城镇数据（返回指针，供C++直接操作）
	FTownData01* GetTownDataPtr(int32 TownID);
	
	// 更新城镇数据
	UFUNCTION(BlueprintCallable, Category = "Town Management")
	bool UpdateTownData(int32 TownID, const FTownData01& NewData);
	
	// 获取所有城镇ID
	UFUNCTION(BlueprintPure, Category = "Town Management")
	TArray<int32> GetAllTownIDs() const;
	
	// 根据ID查找城镇实例
	UFUNCTION(BlueprintPure, Category = "Town Management")
	ATownActor01* FindTownInstance(int32 TownID) const;
	
	// 移除城镇
	UFUNCTION(BlueprintCallable, Category = "Town Management")
	bool RemoveTown(int32 TownID);
	
	// ===== 殖民者管理 =====
	
	// 创建殖民者数据（不创建实例）
	UFUNCTION(BlueprintCallable, Category = "Colonist Management")
	int32 CreateColonistData(int32 TownID, const FString& ColonistName = TEXT(""));
	
	// 查找殖民者数据（返回指针以便修改，不暴露给蓝图）
	FColonistData01* GetColonistData(int32 ColonistID);
	
	// 查找殖民者数据（只读版本）
	UFUNCTION(BlueprintPure, Category = "Colonist Management")
	FColonistData01 GetColonistDataCopy(int32 ColonistID) const;
	
	// 更新殖民者数据
	UFUNCTION(BlueprintCallable, Category = "Colonist Management")
	bool UpdateColonistData(int32 ColonistID, const FColonistData01& NewData);
	
	// 获取所有殖民者ID
	UFUNCTION(BlueprintPure, Category = "Colonist Management")
	TArray<int32> GetAllColonistIDs() const;
	
	// 根据ID查找殖民者实例
	AMoveableEntity01* FindColonistInstance(int32 ColonistID) const;
	
	// 设置殖民者实例
	void SetColonistInstance(int32 ColonistID, AMoveableEntity01* Instance);
	
	// 移除殖民者数据
	UFUNCTION(BlueprintCallable, Category = "Colonist Management")
	bool RemoveColonistData(int32 ColonistID);
	
protected:
	virtual void BeginPlay() override;
	
private:
	// ===== 资源数据 =====
	
	// 玩家金钱
	UPROPERTY(EditAnywhere, Category = "Resources")
	float Gold = 1000.0f;
	
	// ===== ID 计数器 =====
	
	// 下一个全局实体ID（统一ID空间）
	UPROPERTY()
	int32 NextEntityID = 1;
	
	// 已创建的商队总数
	UPROPERTY()
	int32 TotalCaravansCreated = 0;
	
	// 已创建的城镇总数
	UPROPERTY()
	int32 TotalTownsCreated = 0;
	
	// 已创建的殖民者总数
	UPROPERTY()
	int32 TotalColonistsCreated = 0;
	
	// ===== 实体数据存储（分类型存储） =====
	
	// 所有商队数据（包含可选实例）
	UPROPERTY()
	TMap<int32, FCaravanData01> AllCaravans;
	
	// 所有城镇数据（包含实例）
	UPROPERTY()
	TMap<int32, FTownData01> AllTowns;
	
	// 所有殖民者数据（包含可选实例）
	UPROPERTY()
	TMap<int32, FColonistData01> AllColonists;
};

