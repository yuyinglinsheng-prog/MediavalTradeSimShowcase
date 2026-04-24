// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo01/Actors/HexGridActorBase.h"
#include "Demo01/Core01/Demo01_GM.h"
#include "Demo01/Core01/ProductionManager01.h"
#include "Demo01/Core01/Demo01DataTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AHexGridActorBase::AHexGridActorBase()
{
	// 设置默认属性
	PrimaryActorTick.bCanEverTick = false;

	// 创建静态网格组件
	HexMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HexMesh"));
	RootComponent = HexMesh;

	// 初始化变量
	ResourceType = TEXT("");
	GridCoord = FIntVector2::ZeroValue;
	BaseMaterial = nullptr;
	DefaultIcon = nullptr;
}

void AHexGridActorBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 在游戏开始时更新一次视觉效果
	UpdateVisuals();
}

void AHexGridActorBase::UpdateVisuals()
{
	if (!HexMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HexGridActorBase] HexMesh is null, cannot update visuals"));
		return;
	}

	if (!BaseMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HexGridActorBase] BaseMaterial is not set, cannot create material instance"));
		return;
	}

	// 创建材质实例
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!MID)
	{
		UE_LOG(LogTemp, Error, TEXT("[HexGridActorBase] Failed to create material instance"));
		return;
	}

	// 根据资源类型设置贴图
	UTexture2D* IconToUse = nullptr;

	if (!ResourceType.IsEmpty())
	{
		// 尝试从生产管理器获取资源图标
		ADemo01_GM* GM = GetDemo01GameMode();
		if (GM)
		{
			UProductionManager01* ProdManager = GM->GetProductionManager();
			if (ProdManager && ProdManager->IsInitialized())
			{
				FProductData01 ProductData;
				if (GetProductData(ResourceType, ProductData) && ProductData.Icon)
				{
					IconToUse = ProductData.Icon;
					UE_LOG(LogTemp, Verbose, TEXT("[HexGridActorBase] Found icon for resource: %s"), *ResourceType);
				}
				else
				{
					UE_LOG(LogTemp, Verbose, TEXT("[HexGridActorBase] No icon found for resource: %s"), *ResourceType);
				}
			}
		}
	}

	// 如果没有找到资源图标，使用默认图标
	if (!IconToUse)
	{
		IconToUse = DefaultIcon;
		if (!ResourceType.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[HexGridActorBase] Resource '%s' has no icon, using default"), *ResourceType);
		}
	}

	// 设置材质参数
	if (IconToUse)
	{
		MID->SetTextureParameterValue(TEXT("ResourceIcon"), IconToUse);
		UE_LOG(LogTemp, Verbose, TEXT("[HexGridActorBase] Set icon texture on material instance"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HexGridActorBase] No icon texture available"));
	}

	// 应用材质到Mesh
	HexMesh->SetMaterial(0, MID);

	// 触发蓝图事件
	OnResourceTypeChanged();

	/*UE_LOG(LogTemp, Log, TEXT("[HexGridActorBase] Updated visuals for cell at (%d,%d) with resource: %s"),
		GridCoord.X, GridCoord.Y, *ResourceType);*/
}

bool AHexGridActorBase::GetProductData(const FString& InResourceType, FProductData01& OutProductData)
{
	if (InResourceType.IsEmpty())
		return false;

	ADemo01_GM* GM = GetDemo01GameMode();
	if (!GM)
		return false;

	UProductionManager01* ProdManager = GM->GetProductionManager();
	if (!ProdManager || !ProdManager->IsInitialized())
		return false;

	// 注意：GetProductData 返回的是 FProductData01 对象，不是指针
	OutProductData = ProdManager->GetProductData(InResourceType);
	
	// 检查 ProductID 是否为空来判断是否有效
	return !OutProductData.ProductID.IsEmpty();
}

ADemo01_GM* AHexGridActorBase::GetDemo01GameMode() const
{
	if (!GetWorld())
		return nullptr;

	return Cast<ADemo01_GM>(GetWorld()->GetAuthGameMode());
}
