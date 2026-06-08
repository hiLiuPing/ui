# EmbeddedGUI 落地与白皮书补充说明

本文档对应当前 Keil/STM32 工程里的 UI 迁移结果，用来把白皮书中的分层架构落到可编译、可显示、可继续扩展的代码上。

## 1. 当前落地范围

- 已将 Renderer Adapter 直接接入 EmbeddedGUI 的 canvas/font 渲染路径，页面层不包含 `egui.h`、LCD BSP 或 LVGL 头文件。
- 已保留单 UI Task 模型：消息消费、Property Store 前台同步、Page `process/draw`、Renderer `BeginFrame/EndFrame` 都在同一个 UI 上下文中完成。
- 已采用 428x142 实屏尺寸和 16 行 stripe PFB 刷屏，避免常驻整屏 framebuffer，RAM 成本可控。
- 已扩展 5 个真实可显示页面：Home、Controls、List、Data、Settings，覆盖常见控件和数据展示。
- 已新增 `UI_PageManager_Replace()`，适合嵌入式仪表盘左右切页，不会反复压深导航栈。

## 2. 分层边界

Page 层只允许依赖：

- `ui_renderer_adapter.h`
- `ui_property_store.h`
- `ui_message_bus.h`
- `ui_page_manager.h`
- 通用 `ui_types.h` / `ui_event.h`

Page 层禁止依赖：

- EmbeddedGUI 内部头文件
- LCD/OLED/SPI BSP 头文件
- 业务驱动或硬件采样驱动
- LVGL 头文件

这样后续从 EmbeddedGUI 换到 LVGL 时，主要替换 Renderer Adapter 和 Display Driver，不需要重写页面生命周期、消息总线、属性同步和页面代码。

## 3. Renderer Adapter API 与 LVGL 兼容意图

| 当前接口 | 当前 EmbeddedGUI 实现 | 后续 LVGL 映射建议 |
| --- | --- | --- |
| `DrawTextInRect` | `egui_canvas_draw_text` 加坐标估算 | `lv_label` / `lv_draw_label` |
| `DrawPanel` | fill rect + border line | `lv_obj` background/border style |
| `DrawButton` | panel + centered text | `lv_btn` + `lv_label` |
| `DrawProgressBar` | rect + percent fill | `lv_bar` |
| `DrawSlider` | track + knob | `lv_slider` |
| `DrawToggle` | track + knob | `lv_switch` |
| `DrawCheckbox` | box + check mark + label | `lv_checkbox` |
| `DrawListItem` | selected row + title/subtitle | `lv_list` / custom row |
| `DrawBadge` | small filled label | `lv_label` with style |
| `DrawSparkline` | line strip chart | `lv_chart` |
| `DrawIconBox` | icon placeholder + label | `lv_obj` container |

当前 API 没有暴露 EmbeddedGUI 对象生命周期，故意保留 immediate-mode 风格。这样既适合当前小屏 MCU，也方便后续在 Adapter 内部切换到 LVGL 的 draw/task/display flush 模型。

## 4. 5 个示例页面覆盖点

- Home：白皮书核心分层入口，展示 UI Task、MessageBus、Property Store、Renderer Adapter 和 LCD BSP 的边界。
- Controls：按钮、开关、复选框、进度条、滑条，验证常用输入控件接口。
- List：列表行、选中态、Badge，验证菜单/虚拟列表页面的基础表现。
- Data：Property Store 驱动的数值卡片、进度条、sparkline，验证服务数据到 UI 的同步路径。
- Settings：迁移检查项、静态内存、PFB stripe、LVGL-ready API，作为后续项目落地 checklist。

## 5. 数据与任务模型

- 业务服务不直接调用页面函数，只写 Property Store 或发布 MessageBus。
- UI Task 每帧先消费消息，再同步 Property Store 前台缓冲，最后执行页面处理和绘制。
- 页面读取 front buffer，避免服务任务与 UI 绘制读写同一份状态。
- 当前 demo 用定时 heartbeat/progress/temperature 模拟真实服务数据，后续可替换为传感器、通信或设备状态服务。

## 6. 后续查漏补缺清单

- 输入设备：把真实按键、旋钮或触摸事件统一转换成 `ui_event_t`。
- Dirty Region：当前保留接口，后续可将 Renderer EndFrame 从全屏 stripe 演进为脏区 stripe。
- 字体：当前使用 EmbeddedGUI 的 Montserrat 12，后续可增加字号枚举到真实字体表。
- 图片：当前 `DrawIconBox` 是文本占位，后续可补 `DrawBitmap/DrawImage` 并映射到 LVGL image source。
- 主题：当前样式结构已抽象 text/muted/background/border/accent，可继续扩成主题表。
- 服务层：建议新增设备状态采集任务，只通过 MessageBus/PropertyStore 入 UI。
- LVGL 迁移：先保持页面 API 不变，再在 Adapter 内实现 LVGL display flush、draw buffer 和控件映射。
