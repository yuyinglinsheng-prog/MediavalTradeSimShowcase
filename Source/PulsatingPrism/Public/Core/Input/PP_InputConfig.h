#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PP_InputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class PULSATINGPRISM_API UPP_InputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // 默认输入映射上下文
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;
    // ===== 镜头平移控制 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
    TObjectPtr<UInputAction> CameraMoveAction;          // WASD平移（已废弃，使用Forward/Right替代）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
    TObjectPtr<UInputAction> CameraMoveForwardAction;   // WS前后移动
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
    TObjectPtr<UInputAction> CameraMoveRightAction;     // AD左右移动
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
    TObjectPtr<UInputAction> CameraPanAction;           // 鼠标中键拖拽平移
    // ===== 镜头旋转控制 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Rotation")
    TObjectPtr<UInputAction> CameraRotateAction;        // QE旋转
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Rotation")
    TObjectPtr<UInputAction> CameraLookAction;          // 鼠标右键旋转
    // ===== 镜头缩放与高度 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Zoom")
    TObjectPtr<UInputAction> CameraZoomAction;          // 滚轮缩放
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Zoom")
    TObjectPtr<UInputAction> CameraAscendAction;        // 升高高度（PageUp/R）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Zoom")
    TObjectPtr<UInputAction> CameraDescendAction;       // 降低高度（PageDown/F）
    // ===== 功能控制 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Functions")
    TObjectPtr<UInputAction> CameraFocusAction;         // 聚焦选中对象
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Functions")
    TObjectPtr<UInputAction> CameraResetAction;         // 重置视角
    // ===== 速度控制 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Speed")
    TObjectPtr<UInputAction> SpeedBoostAction;          // 加速（Shift）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Speed")
    TObjectPtr<UInputAction> SpeedReduceAction;         // 减速（Alt）
    
    // ===== Demo01 交互控制 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Demo01")
    TObjectPtr<UInputAction> LeftClickAction;           // 左键点击选中
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Demo01")
    TObjectPtr<UInputAction> RightClickAction;          // 右键点击移动
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Demo01")
    TObjectPtr<UInputAction> TogglePauseAction;         // 切换暂停（空格）
    
    // 构造函数
    UPP_InputConfig();
};
