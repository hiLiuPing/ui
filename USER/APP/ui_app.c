#include "ui_app.h"

#include "ui_config.h"
#include "ui_dirty_region.h"
#include "ui_page_manager.h"
#include "ui_property_store.h"
#include "ui_renderer_adapter.h"
#include "ui_types.h"

#include <stdio.h>

#define UI_APP_PAGE_HOME_ID           1U
#define UI_APP_PROP_TITLE_ID          1U
#define UI_APP_PROP_HEARTBEAT_ID      2U
#define UI_APP_PROP_STATUS_ID         3U
#define UI_APP_PROP_SERVICE_ID        4U

static ui_viewport_t g_home_viewport = { 0, 0, 428, 142, 0, 0 };
static int32_t g_home_heartbeat;
static uint32_t g_home_frame_count;

static void UI_App_HomeCreate(ui_page_context_t *page);
static void UI_App_HomeEnter(ui_page_context_t *page);
static void UI_App_HomeProcess(ui_page_context_t *page);
static void UI_App_HomeDraw(ui_page_context_t *page);
static void UI_App_HomeExit(ui_page_context_t *page);
static void UI_App_HomeDestroy(ui_page_context_t *page);
static void UI_App_HomeHandleMessage(ui_page_context_t *page, const ui_msg_t *msg);
static void UI_App_HomeHandleEvent(ui_page_context_t *page, const ui_event_t *event);

static ui_page_context_t g_home_page =
{
  .page_id = UI_APP_PAGE_HOME_ID,
  .is_created = 0U,
  .root_widget = NULL,
  .user_data = NULL,
  .create = UI_App_HomeCreate,
  .enter = UI_App_HomeEnter,
  .process = UI_App_HomeProcess,
  .draw = UI_App_HomeDraw,
  .exit = UI_App_HomeExit,
  .destroy = UI_App_HomeDestroy,
  .handle_message = UI_App_HomeHandleMessage,
  .handle_event = UI_App_HomeHandleEvent
};

static ui_page_context_t *UI_App_PageFactory(uint16_t page_id)
{
  if (page_id == UI_APP_PAGE_HOME_ID)
  {
    return &g_home_page;
  }

  return NULL;
}

static uint16_t UI_App_Color(uint8_t red, uint8_t green, uint8_t blue)
{
  return UI_RendererAdapter_RGB565(red, green, blue);
}

static void UI_App_HomeInvalidateFull(void)
{
  ui_rect_t full_rect = { 0, 0, g_home_viewport.width, g_home_viewport.height };

  UI_DirtyRegion_Invalidate(&full_rect);
}

static void UI_App_DrawPanelFrame(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t accent)
{
  UI_RendererAdapter_DrawFillRect(&g_home_viewport, x, y, width, height, UI_App_Color(13U, 30U, 48U));
  UI_RendererAdapter_DrawRect(&g_home_viewport, x, y, width, height, UI_App_Color(39U, 62U, 82U));
  UI_RendererAdapter_DrawFillRect(&g_home_viewport, x, y, 3, height, accent);
}

static void UI_App_HomeCreate(ui_page_context_t *page)
{
  (void)page;

  g_home_viewport.width = UI_RendererAdapter_GetWidth();
  g_home_viewport.height = UI_RendererAdapter_GetHeight();
  g_home_heartbeat = 0;
  g_home_frame_count = 0U;

  UI_PropertyStore_SetString(UI_APP_PROP_TITLE_ID, "EmbeddedGUI Home");
  UI_PropertyStore_SetString(UI_APP_PROP_STATUS_ID, "UI task locked to 10ms");
  UI_PropertyStore_SetString(UI_APP_PROP_SERVICE_ID, "Services isolated");
  UI_PropertyStore_SetInt(UI_APP_PROP_HEARTBEAT_ID, g_home_heartbeat);
}

static void UI_App_HomeEnter(ui_page_context_t *page)
{
  (void)page;
  UI_App_HomeInvalidateFull();
}

static void UI_App_HomeProcess(ui_page_context_t *page)
{
  (void)page;

  g_home_frame_count++;
  if ((g_home_frame_count % 25U) == 0U)
  {
    g_home_heartbeat++;
    UI_PropertyStore_SetInt(UI_APP_PROP_HEARTBEAT_ID, g_home_heartbeat);
    UI_App_HomeInvalidateFull();
  }
}

static void UI_App_HomeDraw(ui_page_context_t *page)
{
  char heartbeat_line[32];
  char frame_line[32];
  const char *title;
  const char *status;
  const char *service;
  int32_t heartbeat;
  uint16_t c_text = UI_App_Color(230U, 238U, 247U);
  uint16_t c_muted = UI_App_Color(142U, 160U, 180U);
  uint16_t c_blue = UI_App_Color(61U, 174U, 235U);
  uint16_t c_green = UI_App_Color(87U, 220U, 142U);
  uint16_t c_amber = UI_App_Color(246U, 184U, 82U);

  (void)page;

  title = UI_PropertyStore_GetFrontString(UI_APP_PROP_TITLE_ID, "EmbeddedGUI Home");
  status = UI_PropertyStore_GetFrontString(UI_APP_PROP_STATUS_ID, "UI task active");
  service = UI_PropertyStore_GetFrontString(UI_APP_PROP_SERVICE_ID, "Service layer ready");
  heartbeat = UI_PropertyStore_GetFrontInt(UI_APP_PROP_HEARTBEAT_ID, 0);

  (void)snprintf(heartbeat_line, sizeof(heartbeat_line), "Heartbeat %ld", (long)heartbeat);
  (void)snprintf(frame_line, sizeof(frame_line), "Frame %lu", (unsigned long)g_home_frame_count);

  UI_RendererAdapter_DrawFillRect(&g_home_viewport, 0, 0, g_home_viewport.width, g_home_viewport.height, UI_App_Color(8U, 18U, 30U));
  UI_RendererAdapter_DrawFillRect(&g_home_viewport, 0, 0, g_home_viewport.width, 32, UI_App_Color(11U, 34U, 54U));
  UI_RendererAdapter_DrawFillRect(&g_home_viewport, 0, 31, g_home_viewport.width, 2, c_blue);
  UI_RendererAdapter_DrawText(&g_home_viewport, 14, 8, title, c_text);
  UI_RendererAdapter_DrawText(&g_home_viewport, 304, 8, "Renderer: EGUI", c_blue);

  UI_App_DrawPanelFrame(12, 43, 128, 84, c_blue);
  UI_RendererAdapter_DrawText(&g_home_viewport, 23, 52, "UI Framework", c_text);
  UI_RendererAdapter_DrawText(&g_home_viewport, 23, 72, "One UI task", c_muted);
  UI_RendererAdapter_DrawText(&g_home_viewport, 23, 91, "Page lifecycle", c_muted);
  UI_RendererAdapter_DrawText(&g_home_viewport, 23, 110, "No UI mutex", c_green);

  UI_App_DrawPanelFrame(150, 43, 128, 84, c_green);
  UI_RendererAdapter_DrawText(&g_home_viewport, 161, 52, "State Channel", c_text);
  UI_RendererAdapter_DrawText(&g_home_viewport, 161, 72, "MessageBus async", c_muted);
  UI_RendererAdapter_DrawText(&g_home_viewport, 161, 91, "Property dualbuf", c_muted);
  UI_RendererAdapter_DrawText(&g_home_viewport, 161, 110, heartbeat_line, c_green);

  UI_App_DrawPanelFrame(288, 43, 128, 84, c_amber);
  UI_RendererAdapter_DrawText(&g_home_viewport, 299, 52, "Landing Status", c_text);
  UI_RendererAdapter_DrawText(&g_home_viewport, 299, 72, status, c_muted);
  UI_RendererAdapter_DrawText(&g_home_viewport, 299, 91, service, c_muted);
  UI_RendererAdapter_DrawText(&g_home_viewport, 299, 110, frame_line, c_amber);

  UI_RendererAdapter_DrawLine(&g_home_viewport, 12, 134, 416, 134, UI_App_Color(28U, 50U, 70U));
  UI_RendererAdapter_DrawText(&g_home_viewport, 15, 126, "ENTER: heartbeat  BACK: reserved  Driver split: Renderer Adapter -> LCD BSP", c_muted);
}

static void UI_App_HomeExit(ui_page_context_t *page)
{
  (void)page;
}

static void UI_App_HomeDestroy(ui_page_context_t *page)
{
  (void)page;
}

static void UI_App_HomeHandleMessage(ui_page_context_t *page, const ui_msg_t *msg)
{
  (void)page;

  if ((msg != NULL) && (msg->payload_len > 0U))
  {
    UI_PropertyStore_SetString(UI_APP_PROP_SERVICE_ID, "Message consumed");
    UI_App_HomeInvalidateFull();
  }
}

static void UI_App_HomeHandleEvent(ui_page_context_t *page, const ui_event_t *event)
{
  (void)page;

  if (event == NULL)
  {
    return;
  }

  if ((event->type == UI_KEY_ENTER) || (event->type == UI_KEY_CLICK))
  {
    g_home_heartbeat++;
    UI_PropertyStore_SetInt(UI_APP_PROP_HEARTBEAT_ID, g_home_heartbeat);
    UI_App_HomeInvalidateFull();
  }
}

BaseType_t UI_App_Init(void)
{
  UI_PageManager_Init(UI_App_PageFactory);
  return UI_PageManager_OpenRoot(UI_APP_PAGE_HOME_ID);
}
