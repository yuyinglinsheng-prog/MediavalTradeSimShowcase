<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">结合你已实现的六边形网格商队移动系统核心功能，对标《Grand Ages: Medieval》的经营 + 战略核心体验，本次 UI 设计遵循</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">功能适配、层级清晰、操作高效、风格统一</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">原则，既匹配现有技术框架（如选中管理、寻路、时间控制），又满足贸易经营的核心需求，同时预留未来扩展（多商队、地形、随机事件）的 UI 接口。</font>

<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">设计整体分为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心视窗层（游戏主界面）、功能面板层（各类操作窗口）、快捷交互层（悬浮 / 按钮 / 提示）、全局状态层（顶部 / 底部信息栏）</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，四层布局互不遮挡且可快速切换，适配六边形网格的视觉操作逻辑。</font>

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">一、全局基础 UI（常驻界面，无遮挡游戏视野）</font>
<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">作为游戏的基础信息和通用操作入口，常驻屏幕边缘，占用视野面积≤20%，参考《Grand Ages: Medieval》的极简全局栏设计，聚焦</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心状态、通用控制、快速导航</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">。</font>

### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">1. 顶部全局信息栏</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">左侧：游戏时间与资源总览</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">显示游戏内</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">天数 / 小时</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（对接现有 TimeManager01 的时间计数）、玩家总金币（对接 FCaravanData01 的 Gold 字段，后续扩展为全局资源），字体醒目且半透明背景，不遮挡网格视野；</font><font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">示例：</font>`<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">游戏时间：第12天 08时 | 总金币：1250</font>`
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">右侧：全局控制按钮组</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">包含</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">暂停 / 播放、倍速调节（1x/2x/4x）</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（对接现有空格键暂停，扩展多倍速）、</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">视角重置</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（快速回到地图中心）、</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">设置入口</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，按钮为图标 + 小文字提示，hover 时放大高亮，与现有相机控制系统联动。</font>

### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">2. 底部快捷操作栏</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">左侧：选中实体快捷操作</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">当选中商队 / 城镇时，显示</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心操作按钮</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（如商队的 “停止移动 / 查看详情”、城镇的 “贸易 / 查看详情”），无选中时隐藏，按钮样式与选中高亮组件视觉统一；</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">中间：地图与图层切换</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">小型迷你地图（六边形网格缩略版），显示当前视野范围、商队位置（红点）、城镇位置（蓝点）、已探索区域，支持鼠标点击快速跳转到地图位置；</font><font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">搭配</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">图层切换按钮</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（地形层 / 资源层 / 贸易层），后续扩展地形 / 商品资源时启用；</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">右侧：功能面板快速入口</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">固定</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">商队管理、城镇列表、贸易市场、日志</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">四个核心入口，以图标形式呈现，点击弹出对应面板，与未来多商队 / 贸易系统适配。</font>

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">二、核心交互 UI（游戏主视野内，随操作触发）</font>
<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">贴合现有</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">选中管理、寻路移动、Actor 交互</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心功能，为六边形网格操作设计专属交互 UI，参考《Grand Ages: Medieval》的 “选中 - 指令 - 反馈” 连贯操作逻辑，做到</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">操作有提示、状态有反馈、指令有确认</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">。</font>

### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">1. 实体选中高亮与信息悬浮框</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">选中高亮</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：在现有 HighlightComponent 基础上，增加</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">UI 层面的悬浮信息框</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，鼠标选中 / 点击商队 / 城镇时，在实体上方弹出半透明悬浮框，显示核心基础信息，无操作 3 秒后自动收起，hover 时持续显示；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">商队悬浮框：商队 ID / 名称、当前状态（空闲 / 移动中 / 已到达）、移动速度、携带金币 / 货物（后续扩展）、当前网格坐标；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">城镇悬浮框：城镇 ID / 名称、人口、网格坐标、核心贸易商品（后续扩展）、是否可交易；</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">选中标识</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：商队选中后显示</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">彩色光环（如蓝色）</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，城镇选中后显示</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">彩色轮廓（如黄色）</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，多商队选中时显示数字编号，与现有 SelectableEntity01 的选中状态管理联动。</font>

### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">2. 寻路与移动指令 UI</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">路径预览</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：右键点击目标位置（城镇 / 空地）后，立即在六边形网格上用</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">彩色线条 + 格子高亮</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">显示 A * 寻路的路径（对接现有 PathfindingService01），路径格子显示序号，清晰展示移动步骤；</font><font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">若目标不可到达（后续扩展障碍物），弹出红色提示 “该位置不可通行”，无路径生成；</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">移动状态反馈</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：商队移动中，在实体上方显示</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">移动进度条</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（对接现有 MoveProgress），显示 “剩余 X 格 / X 秒”，同时路径预览的已走格子变为灰色，未走格子保持彩色，直观展示移动过程；</font>
+ **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">移动取消</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：商队移动中，点击悬浮框的 “停止移动” 按钮，路径预览消失，进度条清零，商队回到空闲状态。</font>

### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">3. 鼠标交互提示</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">鼠标移至可交互实体（商队 / 城镇）时，光标变为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">手型 + 对应图标</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（商队 = 马车、城镇 = 城堡）；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">鼠标移至可移动的网格格子时，光标变为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">箭头</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，不可移动则为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">禁止符号</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">右键点击非目标位置时，弹出白色小字提示 “请选择商队后再发送移动指令”，避免无效操作。</font>

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">三、功能面板 UI（弹窗式，可拖拽 / 收起 / 关闭）</font>
<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">作为经营核心的操作载体，参考《Grand Ages: Medieval》的</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">模块化面板设计</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，所有面板均为弹窗式，支持鼠标拖拽移动、点击收起为侧边栏、关闭，面板风格统一（半透明深色背景 + 浅色文字 + 彩色功能按钮），分为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">基础功能面板（适配现有系统）</font>****<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">和</font>****<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">扩展功能面板（对接未来贸易 / 多商队系统）</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，面板数据与现有数据结构（FTownData01/FCaravanData01）实时联动。</font>

### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">1. 基础功能面板（立即适配现有系统）</font>
#### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（1）商队详情面板</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">入口：选中商队后，点击悬浮框 “查看详情”/ 底部快捷栏 “商队管理”；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心模块：</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">商队基础信息：ID、名称、当前状态、移动速度、当前坐标、所属玩家；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">移动记录：最近 3 次移动的起点 / 终点 / 耗时，对接日志输出；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">操作按钮：</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">停止移动、返回上一位置、快速前往最近城镇</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（后续扩展）、</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">解散商队</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（后续扩展）；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">状态日志：实时显示商队操作记录（如 “XX 时开始移动至 XX 城镇”“XX 时到达目标位置”），对接现有 UE_LOG 的中文输出，转为 UI 可视化日志。</font>

#### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（2）城镇详情面板</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">入口：选中城镇后，点击悬浮框 “查看详情”/ 底部快捷栏 “城镇列表”；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心模块：</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">城镇基础信息：ID、名称、人口、当前坐标、已解锁贸易功能（后续扩展）；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">周边信息：显示城镇周边 X 格内的其他城镇 / 地形（后续扩展）；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">操作按钮：</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">发送商队至该城镇</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（选中商队后此按钮激活，一键发送移动指令）、</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">标记为常用贸易点</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（后续扩展）。</font>

#### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（3）全局日志面板</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">入口：底部快捷栏 “日志”；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心模块：</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">日志分类：按</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">系统（GM/PC）、寻路、时间、商队、城镇</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">分类筛选（对接现有日志输出的前缀）；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">日志列表：按时间倒序显示所有中文日志，支持关键词搜索、清空日志；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">日志设置：是否开启弹窗提示、日志保存时长，替代现有纯 UE_LOG 输出。</font>

### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">2. 扩展功能面板（对接未来贸易 / 多商队 / 地形系统）</font>
#### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（1）商队管理面板</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心功能：</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">多商队统一管理</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，显示所有商队的列表（含状态、位置、货物），支持</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">批量选中、批量发送移动指令、批量调整速度</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">扩展模块：商队升级（提升移动速度 / 载货量）、商队招募（消耗金币创建新商队），对接未来多商队管理功能。</font>

#### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（2）贸易市场面板</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心功能：</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">城镇间商品贸易</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，是游戏经营核心，参考《Grand Ages: Medieval》的商品供需体系；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心模块：</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">商品列表：显示当前城镇的可交易商品（如粮食、木材、铁器），标注</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">买入价 / 卖出价 / 库存</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，不同城镇价格不同，形成贸易差价；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">商队交易：选择商队，设置</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">购买 / 出售的商品数量</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，扣除 / 增加金币，更新商队载货量；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">贸易路线：添加</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">固定贸易路线</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（如 A 城镇→B 城镇，自动买卖指定商品），对接未来商队自动贸易 AI。</font>

#### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（3）地形与地图面板</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心功能：</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">网格地图可视化</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，显示所有六边形网格的地形类型（平原 / 山地 / 河流）、移动成本、障碍物；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心模块：地形筛选、移动成本显示、地图缩放 / 标注，对接未来地形系统，鼠标移至格子可查看地形详情和移动成本。</font>

#### <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（4）全局资源与经营面板</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心功能：</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">玩家经营总览</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，参考《Grand Ages: Medieval》的全局经济面板；</font>
+ <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心模块：总金币 / 总货物统计、所有城镇的贸易收益汇总、商队运营成本、经济趋势图（如近 7 天金币变化），让玩家直观掌握经营状况。</font>

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">四、UI 交互逻辑与操作体验</font>
<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">遵循</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">“少操作、多反馈、强联动”</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">原则，贴合现有游戏核心机制，让操作连贯高效，避免冗余步骤，参考《Grand Ages: Medieval》的战略经营类游戏操作习惯：</font>

1. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">选中 - 指令</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：所有实体操作均以 “选中实体” 为前提，未选中时所有操作按钮禁用，避免无效指令；</font>
2. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">一键联动</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：选中商队后，点击城镇详情面板的 “发送商队至该城镇”，可直接生成寻路路径并启动移动，无需二次右键点击；</font>
3. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">面板记忆</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：所有弹窗面板关闭时记忆最后位置，再次打开时在同一位置显示，支持收起为侧边栏，不遮挡核心游戏视野；</font>
4. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">实时联动</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：所有面板数据与游戏内数据结构实时同步（如商队移动后坐标立即更新、金币交易后数值立即变化）；</font>
5. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">快捷键适配</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">：在现有输入系统（WASD / 鼠标 / 空格）基础上，为所有核心 UI 操作添加快捷键（如商队详情面板 = Q、城镇列表 = E、贸易市场 = R），与 Enhanced Input System 集成，支持自定义快捷键。</font>

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">五、UI 风格与视觉规范</font>
<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">结合 “中世纪商队贸易” 的游戏主题，参考《Grand Ages: Medieval》的</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">中世纪写实简约风格</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，同时适配六边形网格的几何视觉特征，制定统一视觉规范，保证 UI 与游戏场景融合：</font>

1. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">色彩体系</font>**
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">主色调：深棕色（中世纪木质质感）、藏蓝色（战略沉稳）、金色（金币 / 贸易），低饱和度，避免刺眼；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">功能色：蓝色（商队 / 选中）、黄色（城镇 / 贸易）、绿色（收益 / 可操作）、红色（警告 / 不可通行）、灰色（已完成 / 不可操作）；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">背景：所有面板 / 悬浮框均为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">半透明深色背景（透明度 70%）</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，既保证文字清晰，又不遮挡网格地图。</font>
2. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">字体规范</font>**
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">标题字体：粗体中世纪衬线字体（如 Trajan Pro），用于面板标题、全局信息栏核心数据；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">正文字体：无衬线字体（如微软雅黑），用于面板内容、悬浮框信息，保证可读性；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">字体大小：全局信息（18-24 号）、面板标题（16-18 号）、面板内容（12-14 号）、提示文字（10-12 号）。</font>
3. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">图标规范</font>**
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">所有功能图标均为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">扁平化 + 轻微中世纪风格</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">（如马车 = 商队、城堡 = 城镇、天平 = 贸易、时钟 = 时间）；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">图标尺寸统一（24×24/32×32），hover 时放大 1.2 倍并显示文字提示，激活时高亮，禁用时置灰。</font>
4. **<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">几何适配</font>**
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">部分面板边角可设计为</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">六边形切角</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，与游戏六边形网格视觉呼应，强化游戏主题；</font>
    - <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">路径预览、格子高亮均贴合六边形网格的形状，避免直角线条与六边形网格冲突。</font>

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">六、UI 开发与技术适配建议</font>
<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">结合你现有技术框架，UI 开发优先保证</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">与现有系统的无缝对接</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，预留扩展接口，降低开发成本：</font>

1. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">基于 UE5 的 UMG 系统开发所有 UI，与现有 Demo01 的 UIs 目录（MarketWidget.h/cpp）对接，统一管理 UI 组件；</font>
2. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">所有 UI 数据均通过</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">GameMode/PlayerController</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">获取（如 Demo01_GM/Demo01_PC），与现有服务化设计原则保持一致，不直接操作 Actor；</font>
3. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">寻路路径预览、格子高亮通过</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">UMG + 世界空间 UI</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">实现，对接 GridService 的坐标转换，保证 UI 与六边形网格精准对齐；</font>
4. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">日志面板直接对接现有 UE_LOG 的中文输出，通过蓝图将日志信息实时传递到 UI 列表，无需修改现有日志逻辑；</font>
5. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">所有扩展 UI 面板（贸易市场 / 商队管理）均预留</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">接口函数</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，与未来的 MarketManager01、地形系统、多商队系统联动，保证扩展性。</font>

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">七、与《Grand Ages: Medieval》的差异化设计</font>
<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">在参考其核心体验的基础上，结合六边形网格的特色，做轻量化差异化设计，避免同质化，突出本游戏的</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">网格寻路 + 精准贸易</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">核心：</font>

1. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">强化</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">六边形网格的视觉与操作体验</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，如路径预览、格子高亮均贴合六边形，区别于传统方形网格的战略游戏；</font>
2. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">简化初期 UI 复杂度，优先实现</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">商队移动 + 基础贸易</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">的核心 UI，后期再逐步扩展《Grand Ages: Medieval》的复杂城邦建设、战争系统；</font>
3. <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">突出</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">商队的个性化管理</font>**<font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">，如商队升级、贸易路线定制，让商队成为游戏的核心操作载体，而非单纯的移动工具。</font>

