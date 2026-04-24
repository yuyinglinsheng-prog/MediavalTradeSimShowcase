// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/Button.h"

#include "Demo01/Core01/MarketManager01.h"

#include "CommodityEntryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTransactionQuantityChanged, int, CommodityID, int, Quantity);

/**
 * 
 */
UCLASS()
class PULSATINGPRISM_API UCommodityEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // 初始化商品数据
    UFUNCTION(BlueprintCallable, Category = "CommodityEntry")
    void InitializeCommodity(const FCommodityRuntime& Commodity, UMarketManager01* InMarketManager);
    
    // Widget生命周期
    virtual void NativeConstruct() override;
    
    // 重置交易数量
    UFUNCTION(BlueprintCallable, Category = "CommodityEntry")
    void ResetTransaction();
    
    // 执行交易
    UFUNCTION(BlueprintCallable, Category = "CommodityEntry")
    void ExecuteTransaction();
    
    // 获取交易信息
    UFUNCTION(BlueprintCallable, Category = "CommodityEntry")
    int GetTransactionQuantity() const { return TransactionQuantity; }
    UFUNCTION(BlueprintCallable, Category = "CommodityEntry")
    float GetTransactionValue() const { return TransactionValue; }
    UFUNCTION(BlueprintCallable, Category = "CommodityEntry")
    float GetSimulatedPrice() const { return SimulatedPrice; }
    
    // 委托：交易数量变化时触发
    UPROPERTY(BlueprintAssignable, Category = "CommodityEntry")
    FOnTransactionQuantityChanged OnTransactionQuantityChanged;
    
protected:
    // UI控件绑定
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* CommodityNameText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* CurrentQuantityText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* CurrentPriceText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UEditableText* QuantityInput;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* SimulatedPriceText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* TransactionValueText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UButton* IncreaseButton;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    UButton* DecreaseButton;
    
    // 调试开关
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MarketUI")
    bool bIsDebugOn = false;
    
private:
    // 事件处理
    UFUNCTION()
    void OnQuantityTextChanged(const FText& Text);
    UFUNCTION()
    void OnIncreaseClicked();
    UFUNCTION()
    void OnDecreaseClicked();
    
    // 更新显示
    void UpdateDisplay();
    
    // 计算模拟价格和交易价值
    void CalculateTransactionEffects();
    
    // 成员变量
    int CommodityID;
    FCommodityRuntime OriginalCommodity;
    
    UPROPERTY()
    UMarketManager01* MarketManager;

    int TransactionQuantity;
    float SimulatedPrice;
    float TransactionValue;
};
