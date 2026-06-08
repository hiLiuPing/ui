#include "ui_task.h"

#include "task.h"

#include "ui_animation.h"
#include "ui_app.h"
#include "ui_config.h"
#include "ui_dirty_region.h"
#include "ui_message_bus.h"
#include "ui_page_manager.h"
#include "ui_property_store.h"
#include "ui_renderer_adapter.h"

static StaticTask_t g_ui_task_tcb;
static StackType_t g_ui_task_stack[UI_TASK_STACK_WORDS];
static TaskHandle_t g_ui_task_handle = NULL;

static void UI_Task_Run(void *argument)
{
  TickType_t last_wake_time = xTaskGetTickCount();
  ui_msg_t msg;

  (void)argument;

  for (;;)
  {
    while (UI_MessageBus_Receive(&msg, 0U) == pdPASS)
    {
      UI_PageManager_HandleMessage(&msg);
    }

    UI_PropertyStore_SyncFront();
    UI_PageManager_ProcessActivePage();
    UI_AnimationScheduler_Update(UI_FRAME_TICK_MS);

    UI_RendererAdapter_BeginFrame();
    UI_PageManager_DrawActivePage();
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

  UI_PropertyStore_Init();
  UI_DirtyRegion_Init();
  UI_RendererAdapter_Init();
  UI_AnimationScheduler_Init();

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
