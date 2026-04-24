// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"

#include "Demo01/Core01/MarketManager01.h"
#include "CommodityEntryWidget.h"

#include "MarketWidget.generated.h"

/**
 *专门的标题行_今后可能转移到别的文件
 */
UCLASS()
class PULSATINGPRISM_API UCommodityHeaderWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* NameHeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* QuantityHeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* PriceHeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* TransactionHeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* SimulatedPriceHeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* ValueHeader;
};
/**
 * 
 */
UCLASS()
class PULSATINGPRISM_API UMarketWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // 设置MarketManager引用_需要手动在创建这个widget的时候调用
    UFUNCTION(BlueprintCallable, Category = "MarketUI")
    void SetMarketManager(UMarketManager01* InMarketManager);
    
    // Widget生命周期
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    // UI操作
    UFUNCTION(BlueprintCallable, Category = "MarketUI")
    void RefreshCommodityList();
    UFUNCTION(BlueprintCallable, Category = "MarketUI")
    void OnCancelTransaction();
    UFUNCTION(BlueprintCallable, Category = "MarketUI")
    void OnExecuteTransaction();
    
    // 获取总交易金额
    UFUNCTION(BlueprintCallable, Category = "MarketUI")
    float GetTotalTransactionValue() const;
    
    // 处理单个商品交易数量变化
    UFUNCTION()
    void OnTransactionQuantityChanged(int CommodityID, int Quantity);
    
protected:
    // UI控件绑定
    UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (BindWidget))
    UVerticalBox* CommodityListContainer;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UButton* CancelButton;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UButton* ExecuteButton;
	// HeaderWidget类引用_用于创建标题行
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MarketUI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UCommodityHeaderWidget> HeaderWidgetClass;
	// EntryWidget类引用_用于创建条目
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MarketUI")
    TSubclassOf<UCommodityEntryWidget> CommodityEntryWidgetClass;

    // 显示玩家资金的TextBlock
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* PlayerMoneyText; 

    // 更新资金显示的函数
    UFUNCTION(BlueprintCallable, Category = "MarketUI")
    void UpdatePlayerMoneyDisplay();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MarketUI")
    bool bIsDebugOn = false;
    
private:
    UFUNCTION()
    void CreateHeaderRow();
	//初始化商品列表_注意不要清除contruct已经创建的header
    UFUNCTION()
    void InitializeCommodityList();
    
    // MarketManager引用
    UPROPERTY()
    UMarketManager01* MarketManager;
    
    // 商品条目Widget数组
    UPROPERTY()
    TArray<UCommodityEntryWidget*> CommodityEntries;
    
    // 记录交易前的状态用于撤销
    TMap<int, FCommodityRuntime> PreTransactionState;
};
