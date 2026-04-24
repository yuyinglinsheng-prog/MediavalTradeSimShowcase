// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Actors/SelectableEntity01.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Services/GridService.h"
#include "Components/StaticMeshComponent.h"

ASelectableEntity01::ASelectableEntity01()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	// 创建静态网格组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	
	// 设置碰撞：只用于射线检测（Visibility通道）
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);  // 只用于查询，不用于物理
	MeshComponent->SetCollisionObjectType(ECC_WorldStatic);  // 设置为静态物体
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);  // 默认忽略所有通道
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);  // 只响应Visibility通道
	MeshComponent->SetSimulatePhysics(false);  // 不模拟物理
	
	// 创建选中高亮组件
	HighlightComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighlightComponent"));
	HighlightComponent->SetupAttachment(MeshComponent);
	HighlightComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 高亮组件不需要碰撞
	HighlightComponent->SetVisibility(false);  // 默认隐藏
}

void ASelectableEntity01::BeginPlay()
{
	Super::BeginPlay();
	
	// 不自动调整位置
	// 对于静态实体（如城镇），位置应该在spawn时设置
	// 对于动态实体（如商队），位置会通过移动系统更新
	
	UE_LOG(LogTemp, Log, TEXT("[%s] 初始化完成，世界位置: %s"), 
		*EntityName, *GetActorLocation().ToString());
}

void ASelectableEntity01::SetGridCoord(FIntVector2 NewCoord)
{
	GridCoord = NewCoord;
	
	// 同步更新世界位置
	/*FVector WorldPos = GridCoordToWorld(NewCoord);
	// FVector AdjustedPos = AdjustZPosition(WorldPos);  // 注释：Z轴调整应在蓝图中处理
	SetActorLocation(WorldPos);
	
	UE_LOG(LogTemp, Log, TEXT("[%s] 设置网格坐标: (%d, %d), 世界位置: %s"), 
		*EntityName, NewCoord.X, NewCoord.Y, *AdjustedPos.ToString());*/
}

void ASelectableEntity01::InitializeGridCoord(FIntVector2 NewCoord)
{
	// 只设置网格坐标，不更新世界位置
	// 用于spawn时，世界位置已经由SpawnActor设置
	GridCoord = NewCoord;
	
	UE_LOG(LogTemp, Log, TEXT("[%s] 初始化网格坐标: (%d, %d)"), 
		*EntityName, NewCoord.X, NewCoord.Y);
}

void ASelectableEntity01::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	
	// 更新高亮显示
	if (HighlightComponent)
	{
		HighlightComponent->SetVisibility(bIsSelected);
		UE_LOG(LogTemp, Log, TEXT("[%s] 选中状态: %s，高亮组件: %s"), 
			*EntityName, 
			bIsSelected ? TEXT("选中") : TEXT("取消选中"),
			HighlightComponent->IsVisible() ? TEXT("显示") : TEXT("隐藏"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 选中状态: %s，但高亮组件为空"), 
			*EntityName, 
			bIsSelected ? TEXT("选中") : TEXT("取消选中"));
	}
}

void ASelectableEntity01::OnClicked()
{
	// 基类默认实现：通知 GameMode
	ADemo01_GM* GM = GetDemo01GameMode();
	if (GM)
	{
		// 子类可以重写此方法来实现自定义行为
		UE_LOG(LogTemp, Log, TEXT("[%s] 被点击"), *EntityName);
	}
}

FVector ASelectableEntity01::GridCoordToWorld(FIntVector2 Coord) const
{
	ADemo01_GM* GM = GetDemo01GameMode();
	if (GM)
	{
		return GM->GridCoordToWorld(Coord);
	}
	
	// 如果没有 GameMode，返回简单的转换
	UE_LOG(LogTemp, Warning, TEXT("[%s] 无法获取 GameMode，使用简单坐标转换"), *EntityName);
	return FVector(Coord.X * 100.0f, Coord.Y * 100.0f, 0.0f);
}

ADemo01_GM* ASelectableEntity01::GetDemo01GameMode() const
{
	return Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
}
// ===== Z轴位置调整 =====

FVector ASelectableEntity01::AdjustZPosition(const FVector& WorldPosition) const
{
	FVector AdjustedPosition = WorldPosition;
	
	// 注释：模型可能会更换，Z轴调整应在蓝图中处理，此处直接返回原始位置
	// FVector TraceStart = FVector(WorldPosition.X, WorldPosition.Y, WorldPosition.Z + 1000.0f);
	// FVector TraceEnd = FVector(WorldPosition.X, WorldPosition.Y, WorldPosition.Z - 1000.0f);
	// 
	// FHitResult HitResult;
	// FCollisionQueryParams QueryParams;
	// QueryParams.AddIgnoredActor(this);
	// QueryParams.bTraceComplex = false;
	// 
	// // 使用WorldStatic通道检测地面
	// bool bHit = GetWorld()->LineTraceSingleByChannel(
	// 	HitResult,
	// 	TraceStart,
	// 	TraceEnd,
	// 	ECC_WorldStatic,
	// 	QueryParams
	// );
	// 
	// if (bHit)
	// {
	// 	// 找到地面，调整Z坐标，并添加一个小的偏移避免穿透
	// 	AdjustedPosition.Z = HitResult.Location.Z + 50.0f;  // 50单位的高度偏移
	// }
	// else
	// {
	// 	// 没有找到地面，保持原始Z坐标
	// 	UE_LOG(LogTemp, Warning, TEXT("[可选中实体] %s: 在位置 (%f, %f) 未找到地面"), 
	// 		*EntityName, WorldPosition.X, WorldPosition.Y);
	// }
	// 
	// return AdjustedPosition;
	
	// 直接返回原始位置，Z轴调整应在蓝图中处理
	return WorldPosition;
}