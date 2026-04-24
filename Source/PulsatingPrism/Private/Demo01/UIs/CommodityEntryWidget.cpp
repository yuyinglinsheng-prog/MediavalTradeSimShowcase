// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo01/UIs/CommodityEntryWidget.h"

void UCommodityEntryWidget::InitializeCommodity(const FCommodityRuntime& Commodity, UMarketManager01* InMarketManager)
{
    CommodityID = Commodity.ID;
    OriginalCommodity = Commodity;
    MarketManager = InMarketManager;
    TransactionQuantity = 0;
    SimulatedPrice = Commodity.CurrentPrice;
    TransactionValue = 0.0f;

    // 立即设置QuantityInput的初始值（如果QuantityInput已经存在）
    if (QuantityInput)
    {
        QuantityInput->SetText(FText::FromString("0"));
    }
}

void UCommodityEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (bIsDebugOn)
    {
        UE_LOG(LogTemp, Warning, TEXT("CommodityEntryWidget NativeConstruct: %s"), *GetName());
        UE_LOG(LogTemp, Warning, TEXT("CommodityNameText valid: %s"), CommodityNameText ? TEXT("Yes") : TEXT("No"));
    }
    // 绑定事件
    if (QuantityInput)
    {
        QuantityInput->OnTextChanged.AddDynamic(this, &UCommodityEntryWidget::OnQuantityTextChanged);
        // 确保初始值为0
        QuantityInput->SetText(FText::FromString("0"));
    }

    if (IncreaseButton)
    {
        IncreaseButton->OnClicked.AddDynamic(this, &UCommodityEntryWidget::OnIncreaseClicked);
    }

    if (DecreaseButton)
    {
        DecreaseButton->OnClicked.AddDynamic(this, &UCommodityEntryWidget::OnDecreaseClicked);
    }
    // 初始显示
    UpdateDisplay();
}

void UCommodityEntryWidget::ResetTransaction()
{
    TransactionQuantity = 0;
    SimulatedPrice = OriginalCommodity.CurrentPrice;
    TransactionValue = 0.0f;

    if (QuantityInput)
    {
        QuantityInput->SetText(FText::FromString("0"));
    }

    UpdateDisplay();
}

void UCommodityEntryWidget::ExecuteTransaction()
{
    // [LEGACY] CommodityEntryWidget 已废弃，MarketManager 已重构为无状态服务，此处不再执行
    // float TotalValue;
    // bool bSuccess = MarketManager->ExecuteTransaction(CommodityID, TransactionQuantity, TotalValue);
}

void UCommodityEntryWidget::OnQuantityTextChanged(const FText& Text)
{
    FString TextStr = Text.ToString();
    if (TextStr.IsNumeric())
    {
        TransactionQuantity = FCString::Atoi(*TextStr);

        // 应用买入限制
        if (TransactionQuantity > OriginalCommodity.CurrentNumber)
        {
            TransactionQuantity = OriginalCommodity.CurrentNumber;
            if (QuantityInput)
            {
                QuantityInput->SetText(FText::FromString(FString::FromInt(TransactionQuantity)));
            }
        }

        CalculateTransactionEffects();
        UpdateDisplay();

        // 触发委托
        OnTransactionQuantityChanged.Broadcast(CommodityID, TransactionQuantity);
    }
}

void UCommodityEntryWidget::OnIncreaseClicked()
{
    TransactionQuantity++;

    // 应用买入限制
    if (TransactionQuantity > OriginalCommodity.CurrentNumber)
    {
        TransactionQuantity = OriginalCommodity.CurrentNumber;
    }

    if (QuantityInput)
    {
        QuantityInput->SetText(FText::FromString(FString::FromInt(TransactionQuantity)));
    }
    // 改变widget的文本显示
    CalculateTransactionEffects();
    UpdateDisplay();

    // 触发委托
    OnTransactionQuantityChanged.Broadcast(CommodityID, TransactionQuantity);
}

void UCommodityEntryWidget::OnDecreaseClicked()
{
    TransactionQuantity--;
    if (TransactionQuantity < -9999) // 简单的卖出限制
    {
        TransactionQuantity = -9999;
    }

    if (QuantityInput)
    {
        QuantityInput->SetText(FText::FromString(FString::FromInt(TransactionQuantity)));
    }
    // 改变widget的文本显示
    CalculateTransactionEffects();
    UpdateDisplay();

    // 触发委托
    OnTransactionQuantityChanged.Broadcast(CommodityID, TransactionQuantity);
}

void UCommodityEntryWidget::UpdateDisplay()
{
    if (!CommodityNameText || !CurrentQuantityText || !CurrentPriceText ||
        !SimulatedPriceText || !TransactionValueText) return;
    // 更新基础信息
    CommodityNameText->SetText(FText::FromString(OriginalCommodity.Name));
    CurrentQuantityText->SetText(FText::FromString(FString::FromInt(OriginalCommodity.CurrentNumber)));
    CurrentPriceText->SetText(FText::FromString(FString::SanitizeFloat(OriginalCommodity.CurrentPrice)));
    // 更新模拟信息
    SimulatedPriceText->SetText(FText::FromString(FString::SanitizeFloat(SimulatedPrice)));

    FString ValueString = FString::SanitizeFloat(FMath::Abs(TransactionValue));
    if (TransactionQuantity > 0)
    {
        TransactionValueText->SetText(FText::FromString(FString::Printf(TEXT("- %s"), *ValueString)));
    }
    else if (TransactionQuantity < 0)
    {
        TransactionValueText->SetText(FText::FromString(FString::Printf(TEXT("+ %s"), *ValueString)));
    }
    else
    {
        TransactionValueText->SetText(FText::FromString("0"));
    }
}

void UCommodityEntryWidget::CalculateTransactionEffects()
{
    if (!MarketManager) return;
    // 模拟交易后的商品数量
    int SimulatedQuantity = OriginalCommodity.CurrentNumber - TransactionQuantity;

    // 计算模拟价格（使用MarketManager的逻辑）
    float DemandFactor = FMath::Clamp(
        static_cast<float>(OriginalCommodity.BasicNumber) / FMath::Max(1, SimulatedQuantity),
        0.1f, 10.0f
    );
    SimulatedPrice = OriginalCommodity.BasicPrice * FMath::Pow(DemandFactor, OriginalCommodity.Elasticity);

    // 计算交易价值（买进为负支出，卖出为正收入）
    TransactionValue = -TransactionQuantity * SimulatedPrice;
}

