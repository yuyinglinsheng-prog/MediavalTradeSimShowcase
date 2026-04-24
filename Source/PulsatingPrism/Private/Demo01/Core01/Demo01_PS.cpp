// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Core01/Demo01_PS.h"
#include "Demo01/Actors/TownActor01.h"
#include "Demo01/Actors/MoveableEntity01.h"
#include "Demo01/Actors/SelectableEntity01.h"

ADemo01_PS::ADemo01_PS()
{
	// 初始化默认值
	Gold = 1000.0f;
	NextEntityID = 1;
	TotalCaravansCreated = 0;
	TotalTownsCreated = 0;
	TotalColonistsCreated = 0;
}

void ADemo01_PS::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] PlayerState 初始化完成，初始金钱: %.2f"), Gold);
}

// ===== 金钱管理 =====

bool ADemo01_PS::UpdateGold(float Amount)
{
	// 如果是减少金钱，检查余额是否足够
	if (Amount < 0.0f && Gold < FMath::Abs(Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 金钱不足：当前 %.2f，需要 %.2f"), 
			Gold, FMath::Abs(Amount));
		return false;
	}
	
	Gold += Amount;
	OnGoldChanged.Broadcast(Gold);
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 金钱更新：%+.2f，当前余额: %.2f"), Amount, Gold);
	
	return true;
}

void ADemo01_PS::AddGold(float Amount)
{
	Gold += Amount;
	OnGoldChanged.Broadcast(Gold);
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 金钱变化：%+.2f，当前余额: %.2f"), Amount, Gold);
}

// ===== ID 生成管理 =====

int32 ADemo01_PS::GetNextEntityID()
{
	int32 ID = NextEntityID;
	NextEntityID++;
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 生成全局实体ID: %d"), ID);
	
	return ID;
}

// ===== 名称生成 =====

FString ADemo01_PS::GenerateCaravanName()
{
	// 使用当前的商队总数+1作为名称编号
	FString Name = FString::Printf(TEXT("商队%d"), TotalCaravansCreated + 1);
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 生成商队名称: %s"), *Name);
	
	return Name;
}

FString ADemo01_PS::GenerateTownName()
{
	// 使用当前的城镇总数+1作为名称编号
	FString Name = FString::Printf(TEXT("城镇%d"), TotalTownsCreated + 1);
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 生成城镇名称: %s"), *Name);
	
	return Name;
}

FString ADemo01_PS::GenerateColonistName()
{
	// 使用当前的殖民者总数+1作为名称编号
	FString Name = FString::Printf(TEXT("殖民者%d"), TotalColonistsCreated + 1);
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 生成殖民者名称: %s"), *Name);
	
	return Name;
}

// ===== 实体管理（统一接口） =====

ASelectableEntity01* ADemo01_PS::GetEntityInstance(FEntityReference01 Ref)
{
	if (!Ref.IsValid())
	{
		return nullptr;
	}
	
	switch (Ref.Type)
	{
	case EEntityType01::Caravan:
		if (FCaravanData01* Data = AllCaravans.Find(Ref.EntityID))
		{
			return Data->Instance;  // 可能为nullptr（在城镇中）
		}
		break;
		
	case EEntityType01::Town:
		if (FTownData01* Data = AllTowns.Find(Ref.EntityID))
		{
			return Data->Instance;  // 城镇始终有实例
		}
		break;
		
	case EEntityType01::Colonist:
		if (FColonistData01* Data = AllColonists.Find(Ref.EntityID))
		{
			return Data->Instance;  // 可能为nullptr（在城镇中）
		}
		break;
		
	default:
		break;
	}
	
	return nullptr;
}

FString ADemo01_PS::GetEntityName(FEntityReference01 Ref) const
{
	switch (Ref.Type)
	{
	case EEntityType01::Caravan:
		if (const FCaravanData01* Data = AllCaravans.Find(Ref.EntityID))
		{
			return Data->EntityName;
		}
		break;
		
	case EEntityType01::Town:
		if (const FTownData01* Data = AllTowns.Find(Ref.EntityID))
		{
			return Data->EntityName;
		}
		break;
		
	case EEntityType01::Colonist:
		if (const FColonistData01* Data = AllColonists.Find(Ref.EntityID))
		{
			return Data->EntityName;
		}
		break;
		
	default:
		break;
	}
	
	return TEXT("Unknown");
}

FIntVector2 ADemo01_PS::GetEntityCoord(FEntityReference01 Ref) const
{
	switch (Ref.Type)
	{
	case EEntityType01::Caravan:
		if (const FCaravanData01* Data = AllCaravans.Find(Ref.EntityID))
		{
			return Data->GridCoord;
		}
		break;
		
	case EEntityType01::Town:
		if (const FTownData01* Data = AllTowns.Find(Ref.EntityID))
		{
			return Data->GridCoord;
		}
		break;
		
	case EEntityType01::Colonist:
		if (const FColonistData01* Data = AllColonists.Find(Ref.EntityID))
		{
			return Data->GridCoord;
		}
		break;
		
	default:
		break;
	}
	
	return FIntVector2::ZeroValue;
}

// ===== 商队管理 =====

int32 ADemo01_PS::CreateCaravanData(int32 TownID)
{
	// 生成新ID
	int32 NewID = GetNextEntityID();
	TotalCaravansCreated++;
	
	// 创建商队数据
	FCaravanData01 NewCaravan;
	NewCaravan.EntityID = NewID;
	NewCaravan.EntityName = FString::Printf(TEXT("商队%d"), TotalCaravansCreated);
	NewCaravan.State = ECaravanState01::InTown;
	NewCaravan.CurrentTownID = TownID;
	NewCaravan.MoveSpeed = 1.0f;
	NewCaravan.Instance = nullptr;  // 在城镇中，无实例
	
	// 获取城镇坐标
	if (FTownData01* TownData = AllTowns.Find(TownID))
	{
		NewCaravan.GridCoord = TownData->GridCoord;
		
		// 添加到城镇的商队列表
		TownData->CaravansInTown.Add(NewID);
	}
	
	// 添加到商队数据表
	AllCaravans.Add(NewID, NewCaravan);
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 创建商队数据: %s (ID: %d)，位于城镇 ID: %d"), 
		*NewCaravan.EntityName, NewID, TownID);
	
	return NewID;
}

FCaravanData01 ADemo01_PS::GetCaravanData(int32 CaravanID) const
{
	if (const FCaravanData01* Data = AllCaravans.Find(CaravanID))
	{
		return *Data;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 未找到商队数据 ID: %d"), CaravanID);
	return FCaravanData01();
}

FCaravanData01* ADemo01_PS::GetCaravanDataPtr(int32 CaravanID)
{
	return AllCaravans.Find(CaravanID);
}

bool ADemo01_PS::UpdateCaravanData(int32 CaravanID, const FCaravanData01& NewData)
{
	if (FCaravanData01* Data = AllCaravans.Find(CaravanID))
	{
		*Data = NewData;
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 更新商队数据 ID: %d"), CaravanID);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法更新商队数据，未找到 ID: %d"), CaravanID);
	return false;
}

TArray<int32> ADemo01_PS::GetAllCaravanIDs() const
{
	TArray<int32> IDs;
	AllCaravans.GetKeys(IDs);
	return IDs;
}

AMoveableEntity01* ADemo01_PS::FindCaravanInstance(int32 CaravanID) const
{
	if (const FCaravanData01* Data = AllCaravans.Find(CaravanID))
	{
		return Data->Instance;
	}
	
	return nullptr;
}

AMoveableEntity01* ADemo01_PS::FindMoveableEntityInstance(int32 EntityID) const
{
	if (AMoveableEntity01* Inst = FindCaravanInstance(EntityID))
		return Inst;
	return FindColonistInstance(EntityID);
}

void ADemo01_PS::SetCaravanInstance(int32 CaravanID, AMoveableEntity01* Instance)
{
	if (FCaravanData01* Data = AllCaravans.Find(CaravanID))
	{
		Data->Instance = Instance;
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 设置商队实例 ID: %d, Instance: %s"), 
			CaravanID, Instance ? *Instance->GetName() : TEXT("nullptr"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法设置商队实例，未找到数据 ID: %d"), CaravanID);
	}
}

bool ADemo01_PS::RemoveCaravan(int32 CaravanID)
{
	if (AllCaravans.Remove(CaravanID) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 移除商队数据 ID: %d"), CaravanID);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法移除商队，未找到 ID: %d"), CaravanID);
	return false;
}

// ===== 城镇管理 =====

void ADemo01_PS::RegisterTown(ATownActor01* Town)
{
	if (!Town)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 尝试注册空城镇"));
		return;
	}
	
	// 生成新ID
	int32 NewID = GetNextEntityID();
	TotalTownsCreated++;
	
	// 创建城镇数据
	FTownData01 NewTown;
	NewTown.EntityID = NewID;
	NewTown.EntityName = FString::Printf(TEXT("城镇%d"), TotalTownsCreated);
	NewTown.GridCoord = Town->GetGridCoord();
	NewTown.Population = 1000;
	NewTown.Instance = Town;
	
	// 添加到城镇数据表
	AllTowns.Add(NewID, NewTown);
	
	// 更新Town Actor的ID和名称
	Town->EntityID = NewID;
	Town->EntityName = NewTown.EntityName;
	Town->TownData.EntityID = NewID;
	Town->TownData.EntityName = NewTown.EntityName;
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 注册城镇: %s (ID: %d)，当前总数: %d"), 
		*NewTown.EntityName, NewID, AllTowns.Num());
}

FTownData01 ADemo01_PS::GetTownData(int32 TownID) const
{
	if (const FTownData01* Data = AllTowns.Find(TownID))
	{
		return *Data;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 未找到城镇数据 ID: %d"), TownID);
	return FTownData01();
}

FTownData01* ADemo01_PS::GetTownDataPtr(int32 TownID)
{
	return AllTowns.Find(TownID);
}

bool ADemo01_PS::UpdateTownData(int32 TownID, const FTownData01& NewData)
{
	if (FTownData01* Data = AllTowns.Find(TownID))
	{
		*Data = NewData;
		// UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 更新城镇数据 ID: %d"), TownID);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法更新城镇数据，未找到 ID: %d"), TownID);
	return false;
}

TArray<int32> ADemo01_PS::GetAllTownIDs() const
{
	TArray<int32> IDs;
	AllTowns.GetKeys(IDs);
	return IDs;
}

ATownActor01* ADemo01_PS::FindTownInstance(int32 TownID) const
{
	if (const FTownData01* Data = AllTowns.Find(TownID))
	{
		return Data->Instance;
	}
	
	return nullptr;
}

bool ADemo01_PS::RemoveTown(int32 TownID)
{
	if (AllTowns.Remove(TownID) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 移除城镇数据 ID: %d"), TownID);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法移除城镇，未找到 ID: %d"), TownID);
	return false;
}

// ===== 殖民者管理 =====

int32 ADemo01_PS::CreateColonistData(int32 TownID, const FString& ColonistName)
{
	// 生成新ID
	int32 NewID = GetNextEntityID();
	TotalColonistsCreated++;
	
	// 创建殖民者数据
	FColonistData01 NewColonist;
	NewColonist.EntityID = NewID;
	NewColonist.EntityName = ColonistName.IsEmpty() ? 
		FString::Printf(TEXT("殖民者%d"), TotalColonistsCreated) : ColonistName;
	NewColonist.State = EColonistState01::InTown;
	NewColonist.CurrentTownID = TownID;
	NewColonist.MoveSpeed = 1.0f;
	NewColonist.Instance = nullptr;  // 在城镇中，无实例
	
	// 获取城镇坐标
	if (FTownData01* TownData = AllTowns.Find(TownID))
	{
		NewColonist.GridCoord = TownData->GridCoord;
		
		// 添加到城镇的殖民者列表
		TownData->ColonistsInTown.Add(NewID);
	}
	
	// 添加到殖民者数据表
	AllColonists.Add(NewID, NewColonist);
	
	UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 创建殖民者数据: %s (ID: %d)，位于城镇 ID: %d"), 
		*NewColonist.EntityName, NewID, TownID);
	
	return NewID;
}

FColonistData01* ADemo01_PS::GetColonistData(int32 ColonistID)
{
	return AllColonists.Find(ColonistID);
}

FColonistData01 ADemo01_PS::GetColonistDataCopy(int32 ColonistID) const
{
	if (const FColonistData01* Data = AllColonists.Find(ColonistID))
	{
		return *Data;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 未找到殖民者数据 ID: %d"), ColonistID);
	return FColonistData01();
}

bool ADemo01_PS::UpdateColonistData(int32 ColonistID, const FColonistData01& NewData)
{
	if (FColonistData01* Data = AllColonists.Find(ColonistID))
	{
		*Data = NewData;
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 更新殖民者数据 ID: %d"), ColonistID);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法更新殖民者数据，未找到 ID: %d"), ColonistID);
	return false;
}

TArray<int32> ADemo01_PS::GetAllColonistIDs() const
{
	TArray<int32> IDs;
	AllColonists.GetKeys(IDs);
	return IDs;
}

AMoveableEntity01* ADemo01_PS::FindColonistInstance(int32 ColonistID) const
{
	if (const FColonistData01* Data = AllColonists.Find(ColonistID))
	{
		return Data->Instance;
	}
	
	return nullptr;
}

void ADemo01_PS::SetColonistInstance(int32 ColonistID, AMoveableEntity01* Instance)
{
	if (FColonistData01* Data = AllColonists.Find(ColonistID))
	{
		Data->Instance = Instance;
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 设置殖民者实例 ID: %d, Instance: %s"), 
			ColonistID, Instance ? *Instance->GetName() : TEXT("nullptr"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法设置殖民者实例，未找到数据 ID: %d"), ColonistID);
	}
}

bool ADemo01_PS::RemoveColonistData(int32 ColonistID)
{
	if (AllColonists.Remove(ColonistID) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Demo01_PS] 移除殖民者数据 ID: %d"), ColonistID);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Demo01_PS] 无法移除殖民者，未找到 ID: %d"), ColonistID);
	return false;
}
