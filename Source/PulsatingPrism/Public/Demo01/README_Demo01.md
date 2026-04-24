# Demo01 - 六边形网格商队移动系统

## 项目概述

Demo01 是一个基于六边形网格的商队移动演示系统，展示了战略游戏中的核心交互机制：选中单位、寻路移动、时间控制。这是一个独立的演示项目，与主框架分离，用于验证和测试核心游戏机制。

### 主题：商队贸易模拟

玩家可以控制商队在六边形网格地图上移动，访问不同的城镇进行贸易。系统包含基础的选中、移动、寻路和时间控制功能。

---

## 已实现功能

### 1. 核心系统

#### 网格系统
- **六边形网格**：使用平顶六边形（Flat-Top Hexagon）布局
- **坐标转换**：网格坐标 ↔ 世界坐标双向转换
- **GridService 集成**：复用主框架的 GridService 进行几何计算
- **配置参数**：
  - 网格尺寸：默认 20x20
  - 六边形半径：默认 100.0 单位
  - 可在蓝图中调整

#### Actor 系统
- **SelectableEntity01**（可选中实体基类）
  - 网格坐标管理
  - 选中状态管理
  - 点击检测绑定
  - 高亮组件支持
  
- **TownActor01**（城镇）
  - 可被点击查看信息
  - 存储城镇数据（ID、名称、人口等）
  - 作为商队移动的目标点
  
- **CaravanActor01**（商队）
  - 路径跟随移动
  - 平滑插值移动（每秒 1 格）
  - 暂停/恢复功能
  - 状态管理：空闲、移动中、已到达

#### 寻路系统
- **A* 算法**：完整的 A* 寻路实现
- **六边形网格适配**：使用 GridService 获取六边形邻居
- **启发式函数**：六边形网格距离计算
- **地形成本支持**：当前所有格子成本统一为 1.0（未来可扩展）

#### 时间管理
- **全局暂停/播放**：空格键切换
- **游戏时间**：秒数和天数计数
- **商队同步**：暂停时自动通知所有商队停止移动

#### 输入系统
- **Enhanced Input System**：使用 UE5 的增强输入系统
- **相机控制**：复用 PP_PlayerController 的相机控制
  - WASD：平移
  - QE：旋转
  - 鼠标滚轮：缩放
  - 鼠标中键：拖拽平移
  - 鼠标右键：旋转视角
  - PageUp/PageDown 或 R/F：升降高度
  - Shift：加速移动
  - Alt：减速移动
- **Demo01 特有输入**：
  - 左键：选中实体
  - 右键：发送移动命令（可点击城镇或空地）
  - 空格：切换暂停/播放

#### 选中管理
- 集成在 PlayerController 中
- 支持选中高亮显示
- 自动管理选中状态
- 区分城镇和商队

---

## 技术架构

### 文件组织

```
Source/PulsatingPrism/Public/Demo01/
├── Core01/                          # 核心逻辑
│   ├── Demo01_GM.h/cpp              # GameMode（网格、服务管理）
│   ├── Demo01_PC.h/cpp              # PlayerController（输入、选中）
│   ├── Demo01DataTypes.h            # 所有数据结构
│   ├── PathfindingService01.h/cpp   # A* 寻路服务
│   ├── TimeManager01.h/cpp          # 时间管理器
│   └── MarketManager01.h/cpp        # 市场管理器（保留，未启用）
├── Actors/                          # 游戏对象
│   ├── SelectableEntity01.h/cpp     # 可选中实体基类
│   ├── TownActor01.h/cpp            # 城镇 Actor
│   └── CaravanActor01.h/cpp         # 商队 Actor
└── UIs/                             # UI 组件（未实现）
    └── MarketWidget.h/cpp           # 市场 UI（保留）
```

### 设计原则

1. **独立性**：与主框架分离，使用独立的 GameMode 和 PlayerController
2. **简单优先**：先实现核心交互，后续再扩展
3. **服务化**：所有 Manager/Service 作为 UObject 在 GameMode 中使用
4. **可扩展**：预留扩展接口，便于后续添加功能

### 关键技术点

#### 坐标系统
- 使用 GridService 的轴向坐标系统（Axial Coordinates）
- `FIntVector2(X, Y)` 对应 `FHexCoord(Q, R)`
- 六边形邻居偏移由 GridService 自动处理

#### 移动系统
- 插值移动：`MoveProgress` 从 0 到 1
- 移动速度：默认 1.0 格子/秒
- 支持暂停和恢复

#### 碰撞设置
- 所有 Actor 的 `MeshComponent` 设置为 `NoCollision`
- 关闭物理模拟
- 设置为对 `Visibility` 通道响应（用于射线检测）

---

## 数据结构

### FDemo01GridConfig
```cpp
struct FDemo01GridConfig
{
    int32 GridWidth = 20;        // 网格宽度（列数）
    int32 GridHeight = 20;       // 网格高度（行数）
    float HexRadius = 100.0f;    // 六边形半径
    bool bIsFlatTop = true;      // 是否平顶六边形
};
```

### FTownData01
```cpp
struct FTownData01
{
    int32 TownID;                // 城镇 ID
    FString TownName;            // 城镇名称
    FIntVector2 GridCoord;       // 网格坐标
    int32 Population;            // 人口（占位符）
};
```

### FCaravanData01
```cpp
struct FCaravanData01
{
    int32 CaravanID;             // 商队 ID
    FIntVector2 GridCoord;       // 当前网格坐标
    ECaravanState01 State;       // 状态（空闲/移动中/已到达）
    float MoveSpeed;             // 移动速度（格子/秒）
    int32 Gold;                  // 金钱（占位符）
};
```

---

## 使用指南

### 在编辑器中设置

1. **创建蓝图类**
   - `BP_Demo01_GM`（基于 `ADemo01_GM`）
   - `BP_Demo01_PC`（基于 `ADemo01_PC`）
   - `BP_TownActor01`（基于 `ATownActor01`）
   - `BP_CaravanActor01`（基于 `ACaravanActor01`）

2. **配置 GameMode**
   - 在 `BP_Demo01_GM` 中设置 `TestTownClass` 为 `BP_TownActor01`
   - 这样 BeginPlay 时会自动在网格中心生成一个测试城镇

3. **配置输入**
   - 在 `PP_InputConfig` 数据资产中配置三个 InputAction：
     - `IA_LeftClick`：鼠标左键
     - `IA_RightClick`：鼠标右键
     - `IA_TogglePause`：空格键
   - 在 `BP_Demo01_PC` 中设置 `InputConfig` 变量

4. **配置 Actor 蓝图**
   - 在 `BP_TownActor01` 和 `BP_CaravanActor01` 中：
     - 设置 `MeshComponent` 的静态网格体
     - 设置 `HighlightComponent` 的静态网格体（可选）
     - 配置材质和颜色
     - 设置初始网格坐标（城镇会在 BeginPlay 时自动生成，无需手动放置）

5. **创建测试关卡**
   - 创建新关卡 `Demo01_Level`
   - 设置 GameMode 为 `BP_Demo01_GM`
   - 不需要手动放置城镇，系统会自动在网格中心生成测试城镇

### 测试功能

- 运行游戏后，会在最接近 (0,0,0) 的网格中心自动生成一个测试城镇
- 左键点击城镇，会在日志中看到城镇信息 UI 打开的消息
- 左键点击选中商队，查看高亮效果
- 右键点击城镇，商队应该寻路并移动
- 右键点击空地，商队应该移动到该位置
- 空格键切换暂停/播放
- 查看日志输出，确认所有功能正常

### UI 系统说明

当前 UI 系统使用 UE_LOG 输出，点击城镇时会在日志中显示城镇信息。UI 指针存储在 GameMode 中：

- `CurrentTownUI`：当前打开的城镇 UI 对应的城镇
- `OpenTownInfoUI(Town)`：打开城镇信息 UI
- `CloseTownInfoUI()`：关闭城镇信息 UI

未来实现真实 UI 时，只需要在这些函数中创建和管理 UMG Widget 即可。

---

## 日志输出

所有关键操作都有中文日志输出：

- `[Demo01_GM]`：GameMode 相关
- `[Demo01_PC]`：PlayerController 相关
- `[寻路服务]`：寻路相关
- `[时间管理器]`：时间控制相关
- `[商队]`：商队移动相关
- `[城镇]`：城镇相关
- `[UI]`：UI 相关（暂时用 UE_LOG）

---

## 当前限制

1. **UI 系统**：暂时用 UE_LOG 输出，未实现真实 UI
2. **地形成本**：所有格子移动成本相同（1.0）
3. **碰撞检测**：商队到达目标后可能重叠
4. **市场系统**：已实现但默认禁用（`bEnableMarketSystem = false`）

---

## 未来扩展方向

### 短期扩展（Demo01 内部）
1. **真实 UI 界面**
   - 城镇信息面板
   - 商队状态面板
   - 时间控制 UI（暂停/播放按钮）
   - 路径预览显示

2. **多商队管理**
   - 支持多个商队同时移动
   - 商队列表 UI
   - 批量选中和控制

3. **城镇贸易系统**
   - 城镇之间的商品价格差异
   - 商队货物系统
   - 买卖交易逻辑
   - 利润计算

4. **地形系统**
   - 不同地形类型（平原、山地、河流等）
   - 地形移动成本
   - 障碍物和不可通行区域

5. **游戏性增强**
   - 多倍速控制（2x、4x）
   - 自动寻路到最近城镇
   - 商队自动贸易 AI
   - 随机事件（强盗、天气等）

### 长期扩展（迁移到主框架）
1. 集成到 EventBus 系统
2. 使用 SystemScheduler 管理时间
3. 数据持久化和存档
4. 更复杂的 AI 和经济模拟
5. 多人联机支持

---

## 命名规范

所有 Demo01 相关的类都添加 `01` 后缀：
- `TownActor01`
- `CaravanActor01`
- `PathfindingService01`
- `TimeManager01`
- `Demo01_GM`
- `Demo01_PC`
- `FTownData01`
- `FCaravanData01`

---

## 开发状态

### ✅ 已完成
- 网格系统和坐标转换
- 可选中的城镇和商队 Actor
- A* 寻路算法
- 商队移动和路径跟随
- 时间管理和暂停控制
- 输入处理和选中管理
- 相机控制系统
- 网格可视化和 Debug 绘制
- 自动生成测试城镇
- 城镇点击打开 UI（日志输出）
- UI 状态管理（GameMode 单例模式）

### 🚧 进行中
- 真实 UI 界面实现（当前使用 UE_LOG）

### 📋 计划中
- 城镇信息面板 UI
- 商队状态面板 UI
- 贸易系统实现
- 地形系统
- 多商队管理

---

## 技术参考

### 相关文档
- `Demo01实现方案_v2.md`：详细的技术设计文档
- `Demo01实现完成总结.md`：实现总结和测试指南

### 相关类
- `UGridService`：网格几何计算服务（主框架）
- `APP_PlayerPawn`：相机 Pawn（主框架）
- `UPP_InputConfig`：输入配置数据资产（主框架）

---

## 更新日志

### 2026-03-06
- 初始版本创建
- 完成所有核心系统实现
- 输入系统集成到 PP_InputConfig
- 网格系统创建和绘制完成
- 实现自动生成测试城镇功能
- 实现城镇点击打开 UI（日志输出）
- UI 状态管理框架搭建（GameMode 单例模式）

---

## 联系和反馈

如果你在使用 Demo01 时遇到问题，或有改进建议，请查看日志输出并参考相关文档。

