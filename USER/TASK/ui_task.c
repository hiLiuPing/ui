#include "ui_task.h"

#include "task.h"

#include "ui_animation.h"
#include "ui_app.h"
#include "ui_config.h"
#include "ui_dirty_region.h"
#include "ui_fx_emitter.h"
#include "ui_input_event_queue.h"
#include "ui_layer_manager.h"
#include "ui_message_bus.h"
#include "ui_modal_manager.h"
#include "ui_page_manager.h"
#include "ui_property_binding.h"
#include "ui_property_store.h"
#include "ui_renderer_adapter.h"

static StaticTask_t g_ui_task_tcb;
static StackType_t g_ui_task_stack[UI_TASK_STACK_WORDS];
static TaskHandle_t g_ui_task_handle = NULL;
static ui_viewport_t g_ui_task_viewport;

static void UI_Task_DrawBaseLayer(const ui_viewport_t *viewport, void *user_data)
{
  (void)viewport;
  (void)user_data;
  UI_PageManager_DrawActivePage();
}

static void UI_Task_DrawFxLayer(const ui_viewport_t *viewport, void *user_data)
{
  (void)user_data;
  UI_FXEmitter_Draw(viewport);
}

static void UI_Task_DrawTopLayer(const ui_viewport_t *viewport, void *user_data)
{
  (void)user_data;
  UI_ModalManager_Draw(viewport);
}

static void UI_Task_Run(void *argument)
{
  TickType_t last_wake_time = xTaskGetTickCount();
  ui_msg_t msg;
  ui_event_t event;

  (void)argument;

  for (;;)
  {
    while (UI_InputEventQueue_Receive(&event, 0U) == pdPASS)
    {
      UI_PageManager_DispatchEvent(&event);
    }

    while (UI_MessageBus_Receive(&msg, 0U) == pdPASS)
    {
      UI_PageManager_HandleMessage(&msg);
    }

    UI_PropertyStore_SyncFront();
    UI_PropertyBinding_UpdateDirtyRegions();
    UI_PageManager_ProcessActivePage();
    UI_AnimationScheduler_Update(UI_FRAME_TICK_MS);
    UI_FXEmitter_Update();
    UI_ModalManager_Process();

    UI_RendererAdapter_BeginFrame();
    UI_LayerManager_Draw(&g_ui_task_viewport);
    UI_RendererAdapter_EndFrame();

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(UI_FRAME_TICK_MS));
  }
}

BaseType_t UI_Task_Create(void)
{
  if (g_ui_task_handle != NULL)
  {
    return pdPASS;
  }

  if (UI_MessageBus_Init() != pdPASS)
  {
    return pdFAIL;
  }

  if (UI_InputEventQueue_Init() != pdPASS)
  {
    return pdFAIL;
  }

  UI_PropertyStore_Init();
  UI_PropertyBinding_Init();
  UI_DirtyRegion_Init();
  UI_RendererAdapter_Init();
  UI_AnimationScheduler_Init();
  UI_FXEmitter_Init();
  UI_LayerManager_Init();

  g_ui_task_viewport.x = 0;
  g_ui_task_viewport.y = 0;
  g_ui_task_viewport.width = UI_RendererAdapter_GetWidth();
  g_ui_task_viewport.height = UI_RendererAdapter_GetHeight();
  g_ui_task_viewport.scroll_x = 0;
  g_ui_task_viewport.scroll_y = 0;

  UI_LayerManager_SetDrawCallback(UI_LAYER_BASE, UI_Task_DrawBaseLayer, NULL);
  UI_LayerManager_SetDrawCallback(UI_LAYER_FX_PARTICLE, UI_Task_DrawFxLayer, NULL);
  UI_LayerManager_SetDrawCallback(UI_LAYER_TOP_WINDOW, UI_Task_DrawTopLayer, NULL);

  if (UI_App_Init() != pdPASS)
  {
    return pdFAIL;
  }

  g_ui_task_handle = xTaskCreateStatic(UI_Task_Run,
                                       "uiTask",
                                       UI_TASK_STACK_WORDS,
                                       NULL,
                                       UI_TASK_PRIORITY,
                                       g_ui_task_stack,
                                       &g_ui_task_tcb);

  return (g_ui_task_handle != NULL) ? pdPASS : pdFAIL;
}
