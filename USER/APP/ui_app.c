#include "ui_app.h"

#include "ui_config.h"
#include "ui_dirty_region.h"
#include "ui_page_manager.h"
#include "ui_property_store.h"
#include "ui_renderer_adapter.h"
#include "ui_types.h"

#include <stdio.h>

#define UI_APP_PAGE_HOME_ID      1U
#define UI_APP_PAGE_CONTROLS_ID  2U
#define UI_APP_PAGE_LIST_ID      3U
#define UI_APP_PAGE_DATA_ID      4U
#define UI_APP_PAGE_SETTINGS_ID  5U

#define UI_APP_PROP_TITLE_ID     1U
#define UI_APP_PROP_HEARTBEAT_ID 2U
#define UI_APP_PROP_STATUS_ID    3U
#define UI_APP_PROP_SERVICE_ID   4U
#define UI_APP_PROP_PROGRESS_ID  5U
#define UI_APP_PROP_TEMPERATURE_ID 6U
#define UI_APP_PROP_MODE_ID      7U

typedef struct
{
  uint16_t page_id;
  const char *title;
  const char *subtitle;
} ui_app_page_model_t;

static const ui_app_page_model_t g_app_page_models[] =
{
  { UI_APP_PAGE_HOME_ID, "Home", "whitepaper landing" },
  { UI_APP_PAGE_CONTROLS_ID, "Controls", "button toggle slider" },
  { UI_APP_PAGE_LIST_ID, "List", "menu and virtual rows" },
  { UI_APP_PAGE_DATA_ID, "Data", "property driven widgets" },
  { UI_APP_PAGE_SETTINGS_ID, "Settings", "porting checklist" }
};

static ui_viewport_t g_app_viewport = { 0, 0, 428, 142, 0, 0 };
static uint32_t g_app_frame_count;
static int32_t g_app_heartbeat;
static int32_t g_app_progress;
static int32_t g_app_temperature;
static uint8_t g_app_toggle_on = 1U;
static uint8_t g_app_checkbox_on = 1U;
static uint8_t g_app_selected_row;
static uint8_t g_app_active_page_index;

static void UI_App_PageCreate(ui_page_context_t *page);
static void UI_App_PageEnter(ui_page_context_t *page);
static void UI_App_PageProcess(ui_page_context_t *page);
static void UI_App_PageDraw(ui_page_context_t *page);
static void UI_App_PageExit(ui_page_context_t *page);
static void UI_App_PageDestroy(ui_page_context_t *page);
static void UI_App_PageHandleMessage(ui_page_context_t *page, const ui_msg_t *msg);
static void UI_App_PageHandleEvent(ui_page_context_t *page, const ui_event_t *event);

static ui_page_context_t g_home_page =
{
  .page_id = UI_APP_PAGE_HOME_ID,
  .is_created = 0U,
  .root_widget = NULL,
  .user_data = NULL,
  .create = UI_App_PageCreate,
  .enter = UI_App_PageEnter,
  .process = UI_App_PageProcess,
  .draw = UI_App_PageDraw,
  .exit = UI_App_PageExit,
  .destroy = UI_App_PageDestroy,
  .handle_message = UI_App_PageHandleMessage,
  .handle_event = UI_App_PageHandleEvent
};

static ui_page_context_t g_controls_page =
{
  .page_id = UI_APP_PAGE_CONTROLS_ID,
  .is_created = 0U,
  .root_widget = NULL,
  .user_data = NULL,
  .create = UI_App_PageCreate,
  .enter = UI_App_PageEnter,
  .process = UI_App_PageProcess,
  .draw = UI_App_PageDraw,
  .exit = UI_App_PageExit,
  .destroy = UI_App_PageDestroy,
  .handle_message = UI_App_PageHandleMessage,
  .handle_event = UI_App_PageHandleEvent
};

static ui_page_context_t g_list_page =
{
  .page_id = UI_APP_PAGE_LIST_ID,
  .is_created = 0U,
  .root_widget = NULL,
  .user_data = NULL,
  .create = UI_App_PageCreate,
  .enter = UI_App_PageEnter,
  .process = UI_App_PageProcess,
  .draw = UI_App_PageDraw,
  .exit = UI_App_PageExit,
  .destroy = UI_App_PageDestroy,
  .handle_message = UI_App_PageHandleMessage,
  .handle_event = UI_App_PageHandleEvent
};

static ui_page_context_t g_data_page =
{
  .page_id = UI_APP_PAGE_DATA_ID,
  .is_created = 0U,
  .root_widget = NULL,
  .user_data = NULL,
  .create = UI_App_PageCreate,
  .enter = UI_App_PageEnter,
  .process = UI_App_PageProcess,
  .draw = UI_App_PageDraw,
  .exit = UI_App_PageExit,
  .destroy = UI_App_PageDestroy,
  .handle_message = UI_App_PageHandleMessage,
  .handle_event = UI_App_PageHandleEvent
};

static ui_page_context_t g_settings_page =
{
  .page_id = UI_APP_PAGE_SETTINGS_ID,
  .is_created = 0U,
  .root_widget = NULL,
  .user_data = NULL,
  .create = UI_App_PageCreate,
  .enter = UI_App_PageEnter,
  .process = UI_App_PageProcess,
  .draw = UI_App_PageDraw,
  .exit = UI_App_PageExit,
  .destroy = UI_App_PageDestroy,
  .handle_message = UI_App_PageHandleMessage,
  .handle_event = UI_App_PageHandleEvent
};

static ui_page_context_t *UI_App_PageFactory(uint16_t page_id)
{
  switch (page_id)
  {
  case UI_APP_PAGE_HOME_ID:
    return &g_home_page;

  case UI_APP_PAGE_CONTROLS_ID:
    return &g_controls_page;

  case UI_APP_PAGE_LIST_ID:
    return &g_list_page;

  case UI_APP_PAGE_DATA_ID:
    return &g_data_page;

  case UI_APP_PAGE_SETTINGS_ID:
    return &g_settings_page;

  default:
    break;
  }

  return NULL;
}

static ui_color_t UI_App_Color(uint8_t red, uint8_t green, uint8_t blue)
{
  return UI_RendererAdapter_Color(red, green, blue);
}

static ui_render_style_t UI_App_Style(ui_color_t accent)
{
  ui_render_style_t style;

  style.text = UI_App_Color(232U, 240U, 248U);
  style.muted = UI_App_Color(136U, 154U, 174U);
  style.background = UI_App_Color(13U, 30U, 48U);
  style.border = UI_App_Color(43U, 66U, 88U);
  style.accent = accent;
  style.radius = 4U;
  style.border_width = 1U;
  return style;
}

static void UI_App_InvalidateFull(void)
{
  ui_rect_t full_rect = { 0, 0, g_app_viewport.width, g_app_viewport.height };

  UI_DirtyRegion_Invalidate(&full_rect);
}

static uint8_t UI_App_FindPageIndex(uint16_t page_id)
{
  uint8_t i;

  for (i = 0U; i < (uint8_t)(sizeof(g_app_page_models) / sizeof(g_app_page_models[0])); ++i)
  {
    if (g_app_page_models[i].page_id == page_id)
    {
      return i;
    }
  }

  return 0U;
}

static uint16_t UI_App_PageIdFromIndex(uint8_t index)
{
  uint8_t page_count = (uint8_t)(sizeof(g_app_page_models) / sizeof(g_app_page_models[0]));

  return g_app_page_models[index % page_count].page_id;
}

static void UI_App_OpenPageIndex(uint8_t index)
{
  uint8_t page_count = (uint8_t)(sizeof(g_app_page_models) / sizeof(g_app_page_models[0]));

  g_app_active_page_index = index % page_count;
  (void)UI_PageManager_Replace(UI_App_PageIdFromIndex(g_app_active_page_index));
}

static void UI_App_DrawChrome(uint16_t page_id)
{
  uint8_t i;
  char page_line[24];
  const ui_app_page_model_t *model = &g_app_page_models[UI_App_FindPageIndex(page_id)];
  ui_color_t text = UI_App_Color(232U, 240U, 248U);
  ui_color_t muted = UI_App_Color(136U, 154U, 174U);
  ui_color_t blue = UI_App_Color(61U, 174U, 235U);

  (void)snprintf(page_line, sizeof(page_line), "%u/5", (unsigned int)(UI_App_FindPageIndex(page_id) + 1U));

  UI_RendererAdapter_DrawFillRect(&g_app_viewport, 0, 0, g_app_viewport.width, g_app_viewport.height, UI_App_Color(8U, 18U, 30U));
  UI_RendererAdapter_DrawFillRect(&g_app_viewport, 0, 0, g_app_viewport.width, 28, UI_App_Color(11U, 34U, 54U));
  UI_RendererAdapter_DrawFillRect(&g_app_viewport, 0, 27, g_app_viewport.width, 2, blue);
  UI_RendererAdapter_DrawText(&g_app_viewport, 12, 7, model->title, text);
  UI_RendererAdapter_DrawText(&g_app_viewport, 82, 7, model->subtitle, muted);
  UI_RendererAdapter_DrawText(&g_app_viewport, 380, 7, page_line, blue);

  for (i = 0U; i < 5U; ++i)
  {
    int16_t x = (int16_t)(340 + (i * 8U));
    UI_RendererAdapter_DrawFillRect(&g_app_viewport, x, 20, 5, 5, (i == UI_App_FindPageIndex(page_id)) ? blue : UI_App_Color(40U, 61U, 82U));
  }
}

static void UI_App_DrawFooter(void)
{
  UI_RendererAdapter_DrawLine(&g_app_viewport, 12, 134, 416, 134, UI_App_Color(28U, 50U, 70U));
  UI_RendererAdapter_DrawText(&g_app_viewport, 14, 124, "LEFT/RIGHT page  ENTER action  BACK home", UI_App_Color(136U, 154U, 174U));
}

static void UI_App_DrawHome(void)
{
  char heartbeat_line[28];
  char frame_line[28];
  ui_render_style_t blue = UI_App_Style(UI_App_Color(61U, 174U, 235U));
  ui_render_style_t green = UI_App_Style(UI_App_Color(87U, 220U, 142U));
  ui_render_style_t amber = UI_App_Style(UI_App_Color(246U, 184U, 82U));
  ui_rect_t card0 = { 12, 40, 126, 78 };
  ui_rect_t card1 = { 151, 40, 126, 78 };
  ui_rect_t card2 = { 290, 40, 126, 78 };

  (void)snprintf(heartbeat_line, sizeof(heartbeat_line), "heartbeat %ld", (long)UI_PropertyStore_GetFrontInt(UI_APP_PROP_HEARTBEAT_ID, 0));
  (void)snprintf(frame_line, sizeof(frame_line), "frame %lu", (unsigned long)g_app_frame_count);

  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &card0, "UI", "Page lifecycle", &blue);
  UI_RendererAdapter_DrawText(&g_app_viewport, 60, 72, "one task", blue.muted);
  UI_RendererAdapter_DrawText(&g_app_viewport, 60, 91, "no ui mutex", green.accent);

  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &card1, "BUS", "MessageBus", &green);
  UI_RendererAdapter_DrawText(&g_app_viewport, 199, 72, "Property dualbuf", green.muted);
  UI_RendererAdapter_DrawText(&g_app_viewport, 199, 91, heartbeat_line, green.accent);

  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &card2, "EGUI", "Renderer Adapter", &amber);
  UI_RendererAdapter_DrawText(&g_app_viewport, 338, 72, "LCD BSP flush", amber.muted);
  UI_RendererAdapter_DrawText(&g_app_viewport, 338, 91, frame_line, amber.accent);
}

static void UI_App_DrawControls(void)
{
  char progress_text[24];
  ui_render_style_t blue = UI_App_Style(UI_App_Color(61U, 174U, 235U));
  ui_render_style_t green = UI_App_Style(UI_App_Color(87U, 220U, 142U));
  ui_render_style_t amber = UI_App_Style(UI_App_Color(246U, 184U, 82U));
  ui_rect_t btn0 = { 12, 40, 94, 28 };
  ui_rect_t btn1 = { 116, 40, 94, 28 };
  ui_rect_t btn2 = { 220, 40, 94, 28 };
  ui_rect_t toggle = { 334, 42, 56, 24 };
  ui_rect_t progress = { 12, 82, 168, 18 };
  ui_rect_t slider = { 200, 78, 186, 28 };
  ui_rect_t checkbox = { 12, 108, 180, 18 };

  (void)snprintf(progress_text, sizeof(progress_text), "progress %ld%%", (long)g_app_progress);

  UI_RendererAdapter_DrawButton(&g_app_viewport, &btn0, "Primary", &blue, 1U);
  UI_RendererAdapter_DrawButton(&g_app_viewport, &btn1, "Ghost", &green, 0U);
  UI_RendererAdapter_DrawButton(&g_app_viewport, &btn2, "Danger", &amber, 0U);
  UI_RendererAdapter_DrawToggle(&g_app_viewport, &toggle, g_app_toggle_on, &green);
  UI_RendererAdapter_DrawText(&g_app_viewport, 334, 72, g_app_toggle_on ? "toggle on" : "toggle off", green.muted);

  UI_RendererAdapter_DrawText(&g_app_viewport, 12, 70, progress_text, blue.muted);
  UI_RendererAdapter_DrawProgressBar(&g_app_viewport, &progress, (uint8_t)g_app_progress, &blue);
  UI_RendererAdapter_DrawSlider(&g_app_viewport, &slider, g_app_progress, 0, 100, &amber);
  UI_RendererAdapter_DrawCheckbox(&g_app_viewport, &checkbox, g_app_checkbox_on, "checked option", &green);
}

static void UI_App_DrawList(void)
{
  ui_render_style_t blue = UI_App_Style(UI_App_Color(61U, 174U, 235U));
  ui_render_style_t green = UI_App_Style(UI_App_Color(87U, 220U, 142U));
  ui_render_style_t amber = UI_App_Style(UI_App_Color(246U, 184U, 82U));
  ui_rect_t row0 = { 12, 36, 260, 22 };
  ui_rect_t row1 = { 12, 61, 260, 22 };
  ui_rect_t row2 = { 12, 86, 260, 22 };
  ui_rect_t row3 = { 12, 111, 260, 22 };
  ui_rect_t badge0 = { 294, 40, 84, 20 };
  ui_rect_t badge1 = { 294, 68, 84, 20 };
  ui_rect_t badge2 = { 294, 96, 84, 20 };

  UI_RendererAdapter_DrawListItem(&g_app_viewport, &row0, "Dashboard", "cards and status", g_app_selected_row == 0U, &blue);
  UI_RendererAdapter_DrawListItem(&g_app_viewport, &row1, "Controls", "button input demo", g_app_selected_row == 1U, &green);
  UI_RendererAdapter_DrawListItem(&g_app_viewport, &row2, "Telemetry", "chart from store", g_app_selected_row == 2U, &amber);
  UI_RendererAdapter_DrawListItem(&g_app_viewport, &row3, "Settings", "porting options", g_app_selected_row == 3U, &blue);
  UI_RendererAdapter_DrawBadge(&g_app_viewport, &badge0, "READY", &green);
  UI_RendererAdapter_DrawBadge(&g_app_viewport, &badge1, "STATIC", &blue);
  UI_RendererAdapter_DrawBadge(&g_app_viewport, &badge2, "LVGL OK", &amber);
}

static void UI_App_DrawData(void)
{
  static const int16_t values[] = { 28, 32, 35, 31, 42, 48, 45, 55, 52, 61, 58, 66 };
  char temp_line[24];
  char mode_line[24];
  ui_render_style_t blue = UI_App_Style(UI_App_Color(61U, 174U, 235U));
  ui_render_style_t green = UI_App_Style(UI_App_Color(87U, 220U, 142U));
  ui_render_style_t amber = UI_App_Style(UI_App_Color(246U, 184U, 82U));
  ui_rect_t chart = { 12, 42, 186, 70 };
  ui_rect_t progress = { 225, 52, 170, 18 };
  ui_rect_t card0 = { 218, 82, 84, 30 };
  ui_rect_t card1 = { 312, 82, 84, 30 };

  (void)snprintf(temp_line, sizeof(temp_line), "temp %ld C", (long)UI_PropertyStore_GetFrontInt(UI_APP_PROP_TEMPERATURE_ID, 25));
  (void)snprintf(mode_line, sizeof(mode_line), "mode %ld", (long)UI_PropertyStore_GetFrontInt(UI_APP_PROP_MODE_ID, 1));

  UI_RendererAdapter_DrawText(&g_app_viewport, 12, 32, "sparkline: sensor trend", blue.muted);
  UI_RendererAdapter_DrawSparkline(&g_app_viewport, &chart, values, (uint8_t)(sizeof(values) / sizeof(values[0])), 20, 70, &blue);
  UI_RendererAdapter_DrawText(&g_app_viewport, 225, 34, "service property", green.muted);
  UI_RendererAdapter_DrawProgressBar(&g_app_viewport, &progress, (uint8_t)g_app_progress, &green);
  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &card0, "T", temp_line, &amber);
  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &card1, "M", mode_line, &green);
}

static void UI_App_DrawSettings(void)
{
  ui_render_style_t blue = UI_App_Style(UI_App_Color(61U, 174U, 235U));
  ui_render_style_t green = UI_App_Style(UI_App_Color(87U, 220U, 142U));
  ui_render_style_t amber = UI_App_Style(UI_App_Color(246U, 184U, 82U));
  ui_rect_t item0 = { 12, 38, 190, 28 };
  ui_rect_t item1 = { 218, 38, 190, 28 };
  ui_rect_t item2 = { 12, 74, 190, 28 };
  ui_rect_t item3 = { 218, 74, 190, 28 };
  ui_rect_t cb0 = { 12, 112, 120, 18 };
  ui_rect_t cb1 = { 146, 112, 130, 18 };
  ui_rect_t toggle = { 320, 111, 56, 20 };

  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &item0, "A", "Adapter only", &blue);
  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &item1, "P", "PFB stripes", &green);
  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &item2, "S", "Static memory", &amber);
  UI_RendererAdapter_DrawIconBox(&g_app_viewport, &item3, "L", "LVGL ready API", &blue);
  UI_RendererAdapter_DrawCheckbox(&g_app_viewport, &cb0, 1U, "no egui in page", &green);
  UI_RendererAdapter_DrawCheckbox(&g_app_viewport, &cb1, 1U, "service isolated", &green);
  UI_RendererAdapter_DrawToggle(&g_app_viewport, &toggle, 1U, &amber);
}

static void UI_App_PageCreate(ui_page_context_t *page)
{
  (void)page;

  g_app_viewport.width = UI_RendererAdapter_GetWidth();
  g_app_viewport.height = UI_RendererAdapter_GetHeight();

  UI_PropertyStore_SetString(UI_APP_PROP_TITLE_ID, "EmbeddedGUI Demo");
  UI_PropertyStore_SetString(UI_APP_PROP_STATUS_ID, "Renderer adapter ready");
  UI_PropertyStore_SetString(UI_APP_PROP_SERVICE_ID, "MessageBus + PropertyStore");
  UI_PropertyStore_SetInt(UI_APP_PROP_HEARTBEAT_ID, g_app_heartbeat);
  UI_PropertyStore_SetInt(UI_APP_PROP_PROGRESS_ID, g_app_progress);
  UI_PropertyStore_SetInt(UI_APP_PROP_TEMPERATURE_ID, g_app_temperature);
  UI_PropertyStore_SetInt(UI_APP_PROP_MODE_ID, 1);
}

static void UI_App_PageEnter(ui_page_context_t *page)
{
  g_app_active_page_index = UI_App_FindPageIndex(page->page_id);
  UI_App_InvalidateFull();
}

static void UI_App_PageProcess(ui_page_context_t *page)
{
  (void)page;

  g_app_frame_count++;
  if ((g_app_frame_count % 20U) == 0U)
  {
    g_app_heartbeat++;
    g_app_progress = (g_app_progress + 5) % 101;
    g_app_temperature = 25 + (int32_t)((g_app_frame_count / 20U) % 16U);
    UI_PropertyStore_SetInt(UI_APP_PROP_HEARTBEAT_ID, g_app_heartbeat);
    UI_PropertyStore_SetInt(UI_APP_PROP_PROGRESS_ID, g_app_progress);
    UI_PropertyStore_SetInt(UI_APP_PROP_TEMPERATURE_ID, g_app_temperature);
    UI_App_InvalidateFull();
  }
}

static void UI_App_PageDraw(ui_page_context_t *page)
{
  UI_App_DrawChrome(page->page_id);

  switch (page->page_id)
  {
  case UI_APP_PAGE_CONTROLS_ID:
    UI_App_DrawControls();
    break;

  case UI_APP_PAGE_LIST_ID:
    UI_App_DrawList();
    break;

  case UI_APP_PAGE_DATA_ID:
    UI_App_DrawData();
    break;

  case UI_APP_PAGE_SETTINGS_ID:
    UI_App_DrawSettings();
    break;

  case UI_APP_PAGE_HOME_ID:
  default:
    UI_App_DrawHome();
    break;
  }

  UI_App_DrawFooter();
}

static void UI_App_PageExit(ui_page_context_t *page)
{
  (void)page;
}

static void UI_App_PageDestroy(ui_page_context_t *page)
{
  (void)page;
}

static void UI_App_PageHandleMessage(ui_page_context_t *page, const ui_msg_t *msg)
{
  (void)page;

  if ((msg != NULL) && (msg->payload_len > 0U))
  {
    UI_PropertyStore_SetString(UI_APP_PROP_SERVICE_ID, "message consumed");
    UI_App_InvalidateFull();
  }
}

static void UI_App_PageHandleEvent(ui_page_context_t *page, const ui_event_t *event)
{
  uint8_t page_count = (uint8_t)(sizeof(g_app_page_models) / sizeof(g_app_page_models[0]));

  (void)page;

  if (event == NULL)
  {
    return;
  }

  if ((event->type == UI_KEY_RIGHT) || (event->type == UI_KEY_DOWN))
  {
    UI_App_OpenPageIndex((uint8_t)((g_app_active_page_index + 1U) % page_count));
  }
  else if ((event->type == UI_KEY_LEFT) || (event->type == UI_KEY_UP))
  {
    UI_App_OpenPageIndex((g_app_active_page_index == 0U) ? (uint8_t)(page_count - 1U) : (uint8_t)(g_app_active_page_index - 1U));
  }
  else if (event->type == UI_KEY_BACK)
  {
    UI_App_OpenPageIndex(0U);
  }
  else if ((event->type == UI_KEY_ENTER) || (event->type == UI_KEY_CLICK))
  {
    g_app_toggle_on = (g_app_toggle_on == 0U) ? 1U : 0U;
    g_app_checkbox_on = (g_app_checkbox_on == 0U) ? 1U : 0U;
    g_app_selected_row = (uint8_t)((g_app_selected_row + 1U) % 4U);
    g_app_progress = (g_app_progress + 10) % 101;
    UI_PropertyStore_SetInt(UI_APP_PROP_PROGRESS_ID, g_app_progress);
    UI_App_InvalidateFull();
  }
}

BaseType_t UI_App_Init(void)
{
  g_app_frame_count = 0U;
  g_app_heartbeat = 0;
  g_app_progress = 35;
  g_app_temperature = 25;
  g_app_toggle_on = 1U;
  g_app_checkbox_on = 1U;
  g_app_selected_row = 0U;
  g_app_active_page_index = 0U;

  UI_PageManager_Init(UI_App_PageFactory);
  return UI_PageManager_OpenRoot(UI_APP_PAGE_HOME_ID);
}
