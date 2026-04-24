// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo01/UIs/MarketWidget.h"
#include "Demo01/UIs/Core/IDemo01Styleable.h"
#include "Engine/Font.h"

void UCommodityHeaderWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 设置标题文本
    if (NameHeader) NameHeader->SetText(FText::FromString("商品名称 CommodityName"));
    if (QuantityHeader) QuantityHeader->SetText(FText::FromString("当前数量 CurrentNumber"));
    if (PriceHeader) PriceHeader->SetText(FText::FromString("当前价格 CurrentPrice"));
    if (TransactionHeader) TransactionHeader->SetText(FText::FromString("交易数量 TradeNumber"));
        if (SimulatedPriceHeader) SimulatedPriceHeader->SetText(FText::FromString("模拟价格 TradeUnitPrice"));
    if (ValueHeader) ValueHeader->SetText(FText::FromString("交易价值 TotalTradePrice"));
}

void UMarketWidget::SetMarketManager(UMarketManager01* InMarketManager)
{
	MarketManager = InMarketManager;
}

void UMarketWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (bIsDebugOn)
    {
        UE_LOG(LogTemp, Warning, TEXT("MarketWidget NativeConstruct called"));
        UE_LOG(LogTemp, Warning, TEXT("CommodityListContainer valid: %s"), CommodityListContainer ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogTemp, Warning, TEXT("CancelButton valid: %s"), CancelButton ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogTemp, Warning, TEXT("ExecuteButton valid: %s"), ExecuteButton ? TEXT("Yes") : TEXT("No"));

        // 临时测试：直接添加一个文本看看是否显示
        if (CommodityListContainer)
        {
            UTextBlock* TestText = NewObject<UTextBlock>(this);
            TestText->SetText(FText::FromString("测试文本 - 如果能看到这个，说明容器正常"));
            CommodityListContainer->AddChild(TestText);
            UE_LOG(LogTemp, Warning, TEXT("Added test text to container"));
        }
    }

    // 绑定按钮事件
    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UMarketWidget::OnCancelTransaction);
    }

    if (ExecuteButton)
    {
        ExecuteButton->OnClicked.AddDynamic(this, &UMarketWidget::OnExecuteTransaction);
    }

    // 创建标题行
    CreateHeaderRow();

    // 初始化商品列表
    InitializeCommodityList();

    // 初始化资金显示
    UpdatePlayerMoneyDisplay();
}

void UMarketWidget::NativeDestruct()
{
    // 清理资源
    if (CancelButton) CancelButton->OnClicked.RemoveAll(this);
    if (ExecuteButton) ExecuteButton->OnClicked.RemoveAll(this);

    Super::NativeDestruct();
}

void UMarketWidget::RefreshCommodityList()
{
    // 移除旧的条目（保留Header）
    for (UCommodityEntryWidget* Entry : CommodityEntries)
    {
        if (Entry && Entry->IsValidLowLevel())
        {
            CommodityListContainer->RemoveChild(Entry);
        }
    }
    CommodityEntries.Empty();
    // 重新创建条目
    InitializeCommodityList();
}

void UMarketWidget::OnCancelTransaction()
{
    // 重置所有条目的交易数量
    for (UCommodityEntryWidget* Entry : CommodityEntries)
    {
        Entry->ResetTransaction();
    }

    // 重置交易后也更新资金显示（虽然资金没变，但确保显示正确）
    UpdatePlayerMoneyDisplay();
}

void UMarketWidget::OnExecuteTransaction()
{
    if (!MarketManager) return;
    // 执行所有交易
    bool bAnyTransactionExecuted = false;
    for (UCommodityEntryWidget* Entry : CommodityEntries)
    {
        Entry->ExecuteTransaction();
        bAnyTransactionExecuted = true;
    }
    //// 重置UI
    //OnCancelTransaction();

    //// 刷新列表显示最新数据
    //RefreshCommodityList();

    //// 交易成功后更新资金显示
    //UpdatePlayerMoneyDisplay();

    //UE_LOG(LogTemp, Log, TEXT("Transaction executed, money updated"));
    if (bAnyTransactionExecuted)
    {
        // 只刷新列表，不要调用OnCancelTransaction！
        RefreshCommodityList();

        // 更新资金显示
        UpdatePlayerMoneyDisplay();

        UE_LOG(LogTemp, Log, TEXT("Transaction executed successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No transaction to execute"));
    }
}

float UMarketWidget::GetTotalTransactionValue() const
{
    float TotalValue = 0.0f;
    for (const UCommodityEntryWidget* Entry : CommodityEntries)
    {
        TotalValue += Entry->GetTransactionValue();
    }
    return TotalValue;
}

void UMarketWidget::OnTransactionQuantityChanged(int CommodityID, int Quantity)
{
    // 可以在这里更新总交易金额显示等
    UE_LOG(LogTemp, Log, TEXT("Commodity %d quantity changed to %d"), CommodityID, Quantity);
}

void UMarketWidget::UpdatePlayerMoneyDisplay()
{
    // [LEGACY] MarketWidget 已废弃，MarketManager 不再存储 PlayerMoney
    // 资金用第 Demo01_PS::GetGold() 获取
    if (PlayerMoneyText)
    {
        PlayerMoneyText->SetText(FText::FromString(TEXT("[LEGACY]请使用新市场页面")));
    }
}

void UMarketWidget::CreateHeaderRow()
{
    if (!CommodityListContainer) return;

    CommodityListContainer->ClearChildren();

    if (HeaderWidgetClass)
    {
        UCommodityHeaderWidget* HeaderWidget = CreateWidget<UCommodityHeaderWidget>(this, HeaderWidgetClass);
        if (HeaderWidget)
        {
            CommodityListContainer->AddChild(HeaderWidget);
            // 应用样式
            UE_LOG(LogTemp, Log, TEXT("[MarketWidget] 为 %s 应用样式"), *HeaderWidget->GetName());
            IDemo01Styleable::Execute_ApplyWidgetStyle(HeaderWidget);
        }
    }
    else
    {
        // 备用方案：动态创建简单标题
    }
}

void UMarketWidget::InitializeCommodityList()
{
    // [LEGACY] MarketWidget 已废弃，MarketManager 不再存储商品列表（GetAllCommodities 已移除）
    // 请使用 TownMarketPage + ResourceEntryWidget01 替代
    UE_LOG(LogTemp, Warning, TEXT("[MarketWidget] InitializeCommodityList: 该类已废弃，无操作"));
    CommodityEntries.Empty();
}
