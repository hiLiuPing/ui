# UI Whitepaper Landing Notes

本次落地把白皮书中的核心分层收敛到当前 Keil/STM32 工程：

1. Page 层不包含 `egui.h` 或 LCD BSP 头文件，只调用 `ui_renderer_adapter.h` 的抽象绘制接口。
2. Renderer Adapter 内部使用 EmbeddedGUI canvas/font 作为渲染引擎，再通过 OLED/LCD BSP 的 RGB565 flush 接口落屏。
3. UI Task 仍是唯一 UI 上下文，负责消息消费、Property Store 前台同步、页面 Process/Draw、Renderer Begin/End。
4. 首页仅从 Property Store 读状态，服务状态通过固定 property/message 入口进入 UI，避免页面直接访问硬件或业务驱动。
5. 渲染采用 428x142 实屏尺寸和 16 行 stripe PFB，避免整屏 framebuffer 常驻 RAM，同时保留后续 dirty-region 局部刷新的演进空间。

下一步建议按同一边界继续扩展：

1. 在 Service 层增加真实设备状态采集任务，只写 Property Store 或发布 MessageBus。
2. 在 Page 层新增页面时继续只依赖 `ui_renderer_adapter.h`，不要直接依赖 EmbeddedGUI/LCD。
3. 若后续迁移 LVGL/TouchGFX，只替换 Renderer Adapter 和 Display Driver，Page/Task/Property/MessageBus 保持不动。
