// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Demo01DataTypes.generated.h"

// 前向声明 UI 类
class UTownInfoPanel;
class UUnitEntryWidget01;
class UTownInfoPage;
class UTownBuildingPage;
class UTownMarketPage;

// 前向声明 Actor 类
class ASelectableEntity01;
class ATownActor01;
class AMoveableEntity01;

// ========== 实体类型枚举 ==========

/**
 * 实体类型枚举
 * 用于统一管理所有可选中的实体类型
 */
UENUM(BlueprintType)
enum class EEntityType01 : uint8
{
	None        UMETA(DisplayName = "无"),
	Town        UMETA(DisplayName = "城镇"),
	Caravan     UMETA(DisplayName = "商队"),
	Colonist    UMETA(DisplayName = "殖民者")
	// 未来可扩展：Merchant, Army, etc.
};

/**
 * 实体引用结构
 * 用于选中系统，通过类型+ID引用任何实体
 */
USTRUCT(BlueprintType)
struct FEntityReference01
{
	GENERATED_BODY()
	
	// 实体类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	EEntityType01 Type = EEntityType01::None;
	
	// 实体ID（全局唯一）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	int32 EntityID = -1;
	
	// 检查引用是否有效
	bool IsValid() const { return Type != EEntityType01::None && EntityID >= 0; }
	
	// 比较运算符
	bool operator==(const FEntityReference01& Other) const
	{
		return Type == Other.Type && EntityID == Other.EntityID;
	}
	
	bool operator!=(const FEntityReference01& Other) const
	{
		return !(*this == Other);
	}
	
	// 构造函数
	FEntityReference01() = default;
	FEntityReference01(EEntityType01 InType, int32 InID) : Type(InType), EntityID(InID) {}
};

// ========== 网格系统数据结构 ==========

/**
 * 简化的六边形网格配置
 * Demo01 使用独立的网格系统，但会使用 GridService 进行坐标转换
 */
USTRUCT(BlueprintType)
struct FDemo01GridConfig
{
	GENERATED_BODY()
	
	// 网格宽度（列数）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridWidth = 20;
	
	// 网格高度（行数）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridHeight = 20;
	
	// 六边形半径（世界单位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float HexRadius = 100.0f;
	
	// 是否使用平顶六边形（true = Flat-Top, false = Pointy-Top）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bIsFlatTop = true;
};

/**
 * 网格单元数据（可选，用于存储地形等信息）
 */
USTRUCT(BlueprintType)
struct FDemo01GridCell
{
	GENERATED_BODY()
	
	// 网格坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FIntVector2 Coord = FIntVector2::ZeroValue;
	
	// 是否可通行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bIsWalkable = true;
	
	// 移动成本（用于 A* 寻路）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float MovementCost = 1.0f;
	
	// 新增：格子资源类型（空字符串表示无资源）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Resources")
	FString ResourceType;  // 如"iron_ore", "wood", "stone"等
};

// ========== 实体数据结构 ==========

/**
 * 城镇建筑数据
 */
USTRUCT(BlueprintType)
struct FTownBuildingData01
{
	GENERATED_BODY()

	// 建筑ID（城镇内唯一）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 BuildingID = -1;

	// 建筑名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FText BuildingName;

	// 使用的配方ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FString RecipeID;

	// 是否激活生产
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bIsActive = false;

	// 生产等级（影响效率）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 ProductionLevel = 1;

	// 默认构造函数
	FTownBuildingData01()
	{
		BuildingName = FText::GetEmpty();
		RecipeID = TEXT("");
	}
};

/**
 * 城镇库存数据
 */
USTRUCT(BlueprintType)
struct FTownInventoryData01
{
	GENERATED_BODY()

	// 资源库存 (ResourceID -> Quantity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FString, int32> Resources;

	// 获取资源数量
	int32 GetResourceAmount(const FString& ResourceID) const
	{
		const int32* Found = Resources.Find(ResourceID);
		return Found ? *Found : 0;
	}

	// 设置资源数量
	void SetResourceAmount(const FString& ResourceID, int32 Amount)
	{
		if (Amount <= 0)
		{
			Resources.Remove(ResourceID);
		}
		else
		{
			Resources.Add(ResourceID, Amount);
		}
	}

	// 添加资源
	void AddResource(const FString& ResourceID, int32 Amount)
	{
		if (Amount > 0)
		{
			int32 CurrentAmount = GetResourceAmount(ResourceID);
			SetResourceAmount(ResourceID, CurrentAmount + Amount);
		}
	}

	// 消耗资源（返回是否成功）
	bool ConsumeResource(const FString& ResourceID, int32 Amount)
	{
		int32 CurrentAmount = GetResourceAmount(ResourceID);
		if (CurrentAmount >= Amount)
		{
			SetResourceAmount(ResourceID, CurrentAmount - Amount);
			return true;
		}
		return false;
	}
};

/**
 * 城镇数据
 * 城镇始终有实例（不会被销毁）
 */
USTRUCT(BlueprintType)
struct FTownData01
{
	GENERATED_BODY()
	
	// 实体 ID（全局唯一）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town")
	int32 EntityID = 0;
	
	// 城镇名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town")
	FString EntityName = TEXT("Town");
	
	// 网格坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town")
	FIntVector2 GridCoord = FIntVector2::ZeroValue;
	
	// 人口
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town")
	int32 Population = 1000;
	
	// 停留在城镇的商队ID列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town")
	TArray<int32> CaravansInTown;
	
	// 停留在城镇的殖民者ID列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town")
	TArray<int32> ColonistsInTown;
	
	// ===== 生产系统相关 =====
	
	// 城镇建筑列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TArray<FTownBuildingData01> Buildings;
	
	// 城镇库存
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	FTownInventoryData01 Inventory;
	
	// 天然原料配方白名单（ProductLevel==0，城镇创建时随机分配）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TArray<FString> NativeRecipeIDs;

	// 可建造的配方ID列表（未建造的建筑）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TArray<FString> AvailableRecipeIDs;
	
	// ===== 资源系统相关 =====
	
	// 城镇所在格子的资源类型（空表示无资源）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Town|Resources")
	FString LocalResourceType;
	
	// 解锁的建筑列表（基于资源类型）
	UPROPERTY(BlueprintReadOnly, Category = "Town|Resources")
	TArray<FString> UnlockedBuildingIDs;
	
	// 城镇实例指针（城镇始终有实例）
	UPROPERTY()
	TObjectPtr<ATownActor01> Instance = nullptr;
	
	// 兼容性：保留旧字段名的访问器
	int32 GetTownID() const { return EntityID; }
	FString GetTownName() const { return EntityName; }
};

/**
 * 商队状态枚举
 */
UENUM(BlueprintType)
enum class ECaravanState01 : uint8
{
	Idle        UMETA(DisplayName = "空闲"),
	Moving      UMETA(DisplayName = "移动中"),
	Arrived     UMETA(DisplayName = "已到达"),
	InTown      UMETA(DisplayName = "在城镇中")
};

// ========== 贸易路线相关数据结构 ==========

// 城镇贸易操作（未来扩展，现在留空）
USTRUCT(BlueprintType)
struct FTownTradeAction
{
	GENERATED_BODY()

	// 暂时留空，后续添加买卖配置
};

// 贸易路线配置
USTRUCT(BlueprintType)
struct FTradeRouteConfig
{
	GENERATED_BODY()

	// 访问城镇的顺序（必须访问的城镇ID）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradeRoute")
	TArray<int32> TownVisitOrder;

	// 完整路径（每个格子的坐标，用于显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradeRoute")
	TArray<FIntVector2> FullPathCoords;

	// 当前在路线中的位置（指向TownVisitOrder的索引）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradeRoute")
	int32 CurrentVisitIndex = 0;

	// 是否激活（玩家可以临时暂停）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradeRoute")
	bool bIsActive = true;

	// 路线总长度（缓存，用于显示）
	UPROPERTY(BlueprintReadOnly, Category = "TradeRoute")
	int32 TotalPathLength = 0;

	// ===== 路径有效性（新增） =====

	// 每段路径是否有效（与TownVisitOrder对应，强制循环路线）
	UPROPERTY(BlueprintReadOnly, Category = "TradeRoute")
	TArray<bool> RouteSegmentValidity;

	// 整体路线是否有效（所有段都有效）
	UPROPERTY(BlueprintReadOnly, Category = "TradeRoute")
	bool bIsRouteValid = true;

	// ===== 金钱追踪相关 =====

	// 是否正在追踪金钱变化
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradeRoute|MoneyTracking")
	bool bIsTrackingMoneyChange = false;

	// 路线金钱变化（从TrackingStartTownID开始追踪的累计变化）
	UPROPERTY(BlueprintReadOnly, Category = "TradeRoute|MoneyTracking")
	float RouteMoneyChange = 0.0f;

	// 追踪起始城镇ID（记录从哪个城镇开始追踪）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradeRoute|MoneyTracking")
	int32 TrackingStartTownID = -1;

	// 是否已完成一次完整循环
	UPROPERTY(BlueprintReadOnly, Category = "TradeRoute|MoneyTracking")
	bool bHasCompletedLoop = false;
};

/**
 * 商队数据
 * 商队可能有实例（在世界中）或无实例（在城镇中）
 */
USTRUCT(BlueprintType)
struct FCaravanData01
{
	GENERATED_BODY()
	
	// 实体 ID（全局唯一）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	int32 EntityID = 0;
	
	// 商队名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	FString EntityName = TEXT("Caravan");
	
	// 当前网格坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	FIntVector2 GridCoord = FIntVector2::ZeroValue;
	
	// 当前状态
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	ECaravanState01 State = ECaravanState01::Idle;
	
	// 移动速度（格子/天）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	float MoveSpeed = 1.0f;
	
	// 当前所在城镇ID（-1表示不在城镇中）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	int32 CurrentTownID = -1;
	
	// ===== 数据层移动系统 =====
	
	// 移动路径（完整路径，格子坐标）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Movement")
	TArray<FIntVector2> MovementPath;
	
	// 当前路径索引（指向下一个目标格子）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Movement")
	int32 CurrentPathIndex = 0;
	
	// 累积时间（天，用于计算是否到达下一格）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Movement")
	float AccumulatedTime = 0.0f;
	
	// 是否正在移动中（数据层状态）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Movement")
	bool bIsMoving = false;
	
	// ===== 商队库存系统 =====
	
	// 商队库存
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	FTownInventoryData01 Inventory;
	
	// 载重容量（kg）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan")
	float CarryCapacity = 1000.0f;
	
	// 商队实例指针（在城镇中为nullptr）
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> Instance = nullptr;
	
	// 兼容性：保留旧字段名的访问器
	int32 GetCaravanID() const { return EntityID; }
	FString GetCaravanName() const { return EntityName; }
	
	// ===== 贸易路线系统 =====
	
	// 贸易路线配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Trade")
	FTradeRouteConfig TradeRoute;
	
	// 自动贸易开关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caravan|Trade")
	bool bIsAutoTrading = false;
};

/**
 * 殖民者状态枚举_和商队状态枚举相同，并且就是个实体状态枚举将来也会是相同的，反正运行良好，我不改了
 */
UENUM(BlueprintType)
enum class EColonistState01 : uint8
{
	Idle        UMETA(DisplayName = "空闲"),
	Moving      UMETA(DisplayName = "移动中"),
	Arrived     UMETA(DisplayName = "已到达"),
	InTown      UMETA(DisplayName = "在城镇中")
};

/**
 * 殖民者数据
 * 殖民者可能有实例（在世界中）或无实例（在城镇中）
 */
USTRUCT(BlueprintType)
struct FColonistData01
{
	GENERATED_BODY()
	
	// 实体 ID（全局唯一）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist")
	int32 EntityID = 0;
	
	// 殖民者名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist")
	FString EntityName = TEXT("Colonist");
	
	// 当前网格坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist")
	FIntVector2 GridCoord = FIntVector2::ZeroValue;
	
	// 当前状态
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist")
	EColonistState01 State = EColonistState01::Idle;
	
	// 移动速度（格子/天）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist")
	float MoveSpeed = 1.0f;
	
	// 当前所在城镇ID（-1表示不在城镇中）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist")
	int32 CurrentTownID = -1;
	
	// ===== 数据层移动系统 =====
	
	// 移动路径（完整路径，格子坐标）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist|Movement")
	TArray<FIntVector2> MovementPath;
	
	// 当前路径索引（指向下一个目标格子）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist|Movement")
	int32 CurrentPathIndex = 0;
	
	// 累积时间（天，用于计算是否到达下一格）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist|Movement")
	float AccumulatedTime = 0.0f;
	
	// 是否正在移动中（数据层状态）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist|Movement")
	bool bIsMoving = false;
	
	// 殖民者实例指针（在城镇中为nullptr）
	UPROPERTY()
	TObjectPtr<AMoveableEntity01> Instance = nullptr;
	
	// ===== 建城命令系统 =====
	
	// 是否有待执行的建城命令
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist|Command")
	bool bHasPendingFoundCityCommand = false;
	
	// 建城目标坐标（命令下达时的目的地）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colonist|Command")
	FIntVector2 FoundCityTargetCoord = FIntVector2::ZeroValue;
	
	// 兼容性：保留旧字段名的访问器
	int32 GetColonistID() const { return EntityID; }
	FString GetColonistName() const { return EntityName; }
};

// ========== 寻路数据结构 ==========

/**
 * A* 寻路节点
 */
USTRUCT()
struct FPathNode01
{
	GENERATED_BODY()
	
	FIntVector2 Coord;
	float GCost = 0.0f;  // 从起点到当前节点的实际成本
	float HCost = 0.0f;  // 从当前节点到终点的启发式成本
	float FCost = 0.0f;  // GCost + HCost
	FIntVector2 Parent = FIntVector2(-1, -1);
	
	FPathNode01() : Coord(FIntVector2::ZeroValue) {}
	FPathNode01(FIntVector2 InCoord) : Coord(InCoord) {}
	
	void CalculateFCost()
	{
		FCost = GCost + HCost;
	}
	
	// 用于排序和比较
	bool operator<(const FPathNode01& Other) const
	{
		return FCost < Other.FCost;
	}
};

// ========== 生产系统数据结构 ==========

/**
 * 产品类别枚举
 */
UENUM(BlueprintType)
enum class EProductCategory01 : uint8
{
	Raw         UMETA(DisplayName = "原材料"),
	Processed   UMETA(DisplayName = "加工材料"), 
	Product     UMETA(DisplayName = "最终产品")
};

/**
 * 产品数据表结构
 * 对应 DT_Products01 DataTable
 */
USTRUCT(BlueprintType)
struct PULSATINGPRISM_API FProductData01 : public FTableRowBase
{
	GENERATED_BODY()

	// 产品ID（字符串标识符）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Product")
	FString ProductID;

	// 产品名称（显示用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Product")
	FText ProductName;

	// 产品类别
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Product")
	EProductCategory01 Category = EProductCategory01::Raw;

	// 是否为原材料
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Product")
	bool bIsRaw = true;

	// 基础价值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Product")
	int32 BaseValue = 0;

	// 人口消费量（每人每周期消费量）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Product")
	float PopulationConsumption = 0.0f;

	// ===== 市场系统相关字段 =====
	
	// 价格弹性系数（供需敏感度，默认1.0）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	float Elasticity = 1.0f;
	
	// 商品重量（每单位重量，用于商队载货量计算）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market", meta = (ClampMin = 0.1))
	float WeightPerUnit = 1.0f;
	
	// ===== 视觉相关 =====
	
	// 资源/产品图标（用于UI和地块可视化）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Product|Visual")
	TObjectPtr<UTexture2D> Icon;

	// 默认构造函数
	FProductData01()
	{
		ProductID = TEXT("");
		ProductName = FText::GetEmpty();
	}
};

/**
 * 资源需求/产出结构
 */
USTRUCT(BlueprintType)
struct FResourceAmount01
{
	GENERATED_BODY()

	// 资源ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FString ResourceID;

	// 数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 Quantity = 0;

	// 默认构造函数
	FResourceAmount01()
	{
		ResourceID = TEXT("");
		Quantity = 0;
	}

	// 带参数构造函数
	FResourceAmount01(const FString& InResourceID, int32 InQuantity)
		: ResourceID(InResourceID), Quantity(InQuantity)
	{
	}
};

/**
 * 生产配方数据表结构
 * 对应 DT_Recipes01 DataTable
 */
USTRUCT(BlueprintType)
struct PULSATINGPRISM_API FRecipeData01 : public FTableRowBase
{
	GENERATED_BODY()

	// 配方ID（字符串标识符）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FString RecipeID;

	// 主要产品ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FString ProductID;

	// 配方别名（如"使用工具"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText AlternativeName;

	// CSV导入桥接字段（格式: "resource_id:qty,resource_id2:qty2"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Import")
	FString InputsStr;

	// CSV导入桥接字段（格式: "resource_id:qty"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Import")
	FString OutputsStr;

	// 输入资源列表（由EditorUtilityObject从InputsStr解析填入）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FResourceAmount01> Inputs;

	// 输出资源列表（由EditorUtilityObject从OutputsStr解析填入）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FResourceAmount01> Outputs;

	// 生产设施名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText ProductionFacility;

	// 产品在生产链中的等级（原料等级是0）（和产品的分类使用枚举不同，这里的分类还希望得出配方在整个生产链的位置，就是不知道有没有必要）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int ProductLevel = 0;

	// 默认构造函数
	FRecipeData01()
	{
		RecipeID = TEXT("");
		ProductID = TEXT("");
		AlternativeName = FText::GetEmpty();
		ProductionFacility = FText::GetEmpty();
	}
};

// ========== 贸易系统数据结构 ==========

// 交易方案（用于MarketManager返回计算结果）
USTRUCT(BlueprintType)
struct FTradePlan
{
	GENERATED_BODY()
	
	// 购买清单（商品ID → 数量）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradePlan")
	TMap<FString, int32> BuyPlan;
	
	// 卖出清单（商品ID → 数量）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradePlan")
	TMap<FString, int32> SellPlan;
	
	// 本次交易的预计金钱变化（+/-金币）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradePlan")
	float MoneyChange = 0.0f;
	
	// 是否可执行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradePlan")
	bool bIsExecutable = false;
	
	// 不可执行的原因
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TradePlan")
	FString FailureReason;
};

// ========== UI 配置数据结构 ==========

// 前向声明商队UI类
class UCaravanInfoPanel01;
class UCaravanInventoryPage01;
class UDemo01MainUI;

/**
 * Demo01 UI 配置结构体
 * 集中管理所有 Widget 类引用
 */
USTRUCT(BlueprintType)
struct FDemo01UIConfig
{
	GENERATED_BODY()
	
	// 主UI容器类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main UI")
	TSubclassOf<UDemo01MainUI> MainUIWidgetClass;
	
	// 城镇信息面板类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panels")
	TSubclassOf<UTownInfoPanel> TownInfoPanelClass;
	
	// 商队信息面板类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panels")
	TSubclassOf<UCaravanInfoPanel01> CaravanInfoPanelClass;
	
	// 城镇页面类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pages")
	TSubclassOf<UTownInfoPage> TownInfoPageClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pages")
	TSubclassOf<UTownBuildingPage> TownBuildingPageClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pages")
	TSubclassOf<UTownMarketPage> TownMarketPageClass;
	
	// 商队页面类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pages")
	TSubclassOf<UCaravanInventoryPage01> CaravanInventoryPageClass;
	
	// 通用单位列表项Widget类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUnitEntryWidget01> UnitEntryWidgetClass;
};

