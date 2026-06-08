#include "ui_renderer_adapter.h"

#include "lcd.h"

#include "canvas/egui_canvas.h"
#include "resource/egui_resource.h"

#include <string.h>

#define UI_RENDERER_COMMAND_CAPACITY 48U
#define UI_RENDERER_TEXT_LENGTH      48U
#define UI_RENDERER_STRIPE_HEIGHT    16U

typedef enum
{
  UI_RENDER_CMD_TEXT = 0,
  UI_RENDER_CMD_LINE,
  UI_RENDER_CMD_FILL_RECT,
  UI_RENDER_CMD_RECT
} ui_render_cmd_type_t;

typedef struct
{
  ui_render_cmd_type_t type;
  int16_t x1;
  int16_t y1;
  int16_t x2;
  int16_t y2;
  int16_t width;
  int16_t height;
  uint16_t color;
  char text[UI_RENDERER_TEXT_LENGTH];
} ui_render_cmd_t;

static ui_render_cmd_t g_render_commands[UI_RENDERER_COMMAND_CAPACITY];
static uint16_t g_render_command_count;
static egui_color_int_t g_render_stripe[LCD_W * UI_RENDERER_STRIPE_HEIGHT];

static uint16_t UI_RendererAdapter_BackgroundColor(void)
{
  return UI_RendererAdapter_RGB565(8U, 18U, 30U);
}

static egui_color_t UI_RendererAdapter_ToEguiColor(uint16_t color)
{
  egui_color_t egui_color;

  egui_color.full = color;
  return egui_color;
}

static int16_t UI_RendererAdapter_MaxInt16(int16_t a, int16_t b)
{
  return (a > b) ? a : b;
}

static int16_t UI_RendererAdapter_MinInt16(int16_t a, int16_t b)
{
  return (a < b) ? a : b;
}

static int UI_RendererAdapter_ClipRect(const ui_viewport_t *viewport,
                                       int16_t *x,
                                       int16_t *y,
                                       int16_t *width,
                                       int16_t *height)
{
  int16_t x1;
  int16_t y1;
  int16_t x2;
  int16_t y2;
  int16_t clip_x1;
  int16_t clip_y1;
  int16_t clip_x2;
  int16_t clip_y2;

  if ((viewport == NULL) || (x == NULL) || (y == NULL) || (width == NULL) || (height == NULL))
  {
    return 0;
  }

  if ((*width <= 0) || (*height <= 0) || (viewport->width <= 0) || (viewport->height <= 0))
  {
    return 0;
  }

  x1 = *x;
  y1 = *y;
  x2 = x1 + *width;
  y2 = y1 + *height;

  clip_x1 = UI_RendererAdapter_MaxInt16(viewport->x, 0);
  clip_y1 = UI_RendererAdapter_MaxInt16(viewport->y, 0);
  clip_x2 = UI_RendererAdapter_MinInt16((int16_t)(viewport->x + viewport->width), LCD_W);
  clip_y2 = UI_RendererAdapter_MinInt16((int16_t)(viewport->y + viewport->height), LCD_H);

  x1 = UI_RendererAdapter_MaxInt16(x1, clip_x1);
  y1 = UI_RendererAdapter_MaxInt16(y1, clip_y1);
  x2 = UI_RendererAdapter_MinInt16(x2, clip_x2);
  y2 = UI_RendererAdapter_MinInt16(y2, clip_y2);

  if ((x2 <= x1) || (y2 <= y1))
  {
    return 0;
  }

  *x = x1;
  *y = y1;
  *width = x2 - x1;
  *height = y2 - y1;
  return 1;
}

static uint8_t UI_RendererAdapter_LineOutCode(const ui_viewport_t *viewport, int16_t x, int16_t y)
{
  uint8_t code = 0U;
  int16_t left = viewport->x;
  int16_t top = viewport->y;
  int16_t right = viewport->x + viewport->width - 1;
  int16_t bottom = viewport->y + viewport->height - 1;

  if (x < left)
  {
    code |= 1U;
  }
  else if (x > right)
  {
    code |= 2U;
  }

  if (y < top)
  {
    code |= 4U;
  }
  else if (y > bottom)
  {
    code |= 8U;
  }

  return code;
}

static int UI_RendererAdapter_ClipLine(const ui_viewport_t *viewport,
                                       int16_t *x1,
                                       int16_t *y1,
                                       int16_t *x2,
                                       int16_t *y2)
{
  uint8_t out1;
  uint8_t out2;
  int32_t x;
  int32_t y;
  ui_viewport_t clipped;

  if ((viewport == NULL) || (x1 == NULL) || (y1 == NULL) || (x2 == NULL) || (y2 == NULL))
  {
    return 0;
  }

  if ((viewport->width <= 0) || (viewport->height <= 0))
  {
    return 0;
  }

  clipped.x = UI_RendererAdapter_MaxInt16(viewport->x, 0);
  clipped.y = UI_RendererAdapter_MaxInt16(viewport->y, 0);
  clipped.width = UI_RendererAdapter_MinInt16((int16_t)(viewport->x + viewport->width - clipped.x), (int16_t)(LCD_W - clipped.x));
  clipped.height = UI_RendererAdapter_MinInt16((int16_t)(viewport->y + viewport->height - clipped.y), (int16_t)(LCD_H - clipped.y));
  clipped.scroll_x = 0;
  clipped.scroll_y = 0;

  if ((clipped.width <= 0) || (clipped.height <= 0))
  {
    return 0;
  }

  for (;;)
  {
    int16_t left = clipped.x;
    int16_t top = clipped.y;
    int16_t right = clipped.x + clipped.width - 1;
    int16_t bottom = clipped.y + clipped.height - 1;

    out1 = UI_RendererAdapter_LineOutCode(&clipped, *x1, *y1);
    out2 = UI_RendererAdapter_LineOutCode(&clipped, *x2, *y2);

    if ((out1 | out2) == 0U)
    {
      return 1;
    }

    if ((out1 & out2) != 0U)
    {
      return 0;
    }

    if ((out1 != 0U) && ((out1 & 8U) != 0U))
    {
      x = *x1 + ((int32_t)(*x2 - *x1) * (bottom - *y1)) / (*y2 - *y1);
      y = bottom;
      *x1 = (int16_t)x;
      *y1 = (int16_t)y;
    }
    else if ((out1 != 0U) && ((out1 & 4U) != 0U))
    {
      x = *x1 + ((int32_t)(*x2 - *x1) * (top - *y1)) / (*y2 - *y1);
      y = top;
      *x1 = (int16_t)x;
      *y1 = (int16_t)y;
    }
    else if ((out1 != 0U) && ((out1 & 2U) != 0U))
    {
      y = *y1 + ((int32_t)(*y2 - *y1) * (right - *x1)) / (*x2 - *x1);
      x = right;
      *x1 = (int16_t)x;
      *y1 = (int16_t)y;
    }
    else if (out1 != 0U)
    {
      y = *y1 + ((int32_t)(*y2 - *y1) * (left - *x1)) / (*x2 - *x1);
      x = left;
      *x1 = (int16_t)x;
      *y1 = (int16_t)y;
    }
    else if ((out2 & 8U) != 0U)
    {
      x = *x1 + ((int32_t)(*x2 - *x1) * (bottom - *y1)) / (*y2 - *y1);
      y = bottom;
      *x2 = (int16_t)x;
      *y2 = (int16_t)y;
    }
    else if ((out2 & 4U) != 0U)
    {
      x = *x1 + ((int32_t)(*x2 - *x1) * (top - *y1)) / (*y2 - *y1);
      y = top;
      *x2 = (int16_t)x;
      *y2 = (int16_t)y;
    }
    else if ((out2 & 2U) != 0U)
    {
      y = *y1 + ((int32_t)(*y2 - *y1) * (right - *x1)) / (*x2 - *x1);
      x = right;
      *x2 = (int16_t)x;
      *y2 = (int16_t)y;
    }
    else
    {
      y = *y1 + ((int32_t)(*y2 - *y1) * (left - *x1)) / (*x2 - *x1);
      x = left;
      *x2 = (int16_t)x;
      *y2 = (int16_t)y;
    }
  }
}

static ui_render_cmd_t *UI_RendererAdapter_AllocCommand(ui_render_cmd_type_t type)
{
  ui_render_cmd_t *cmd;

  if (g_render_command_count >= UI_RENDERER_COMMAND_CAPACITY)
  {
    return NULL;
  }

  cmd = &g_render_commands[g_render_command_count++];
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  return cmd;
}

static void UI_RendererAdapter_FillStripe(uint16_t color, uint16_t count)
{
  uint16_t i;

  for (i = 0U; i < count; ++i)
  {
    g_render_stripe[i] = color;
  }
}

static void UI_RendererAdapter_DrawCommand(egui_canvas_t *canvas, const ui_render_cmd_t *cmd)
{
  egui_color_t color;
  const egui_font_t *font = (const egui_font_t *)&egui_res_font_montserrat_12_4;

  if ((canvas == NULL) || (cmd == NULL))
  {
    return;
  }

  color = UI_RendererAdapter_ToEguiColor(cmd->color);

  switch (cmd->type)
  {
  case UI_RENDER_CMD_TEXT:
    egui_canvas_draw_text(canvas, font, cmd->text, cmd->x1, cmd->y1, color, EGUI_ALPHA_100);
    break;

  case UI_RENDER_CMD_LINE:
    egui_canvas_draw_line(canvas, cmd->x1, cmd->y1, cmd->x2, cmd->y2, 1, color, EGUI_ALPHA_100);
    break;

  case UI_RENDER_CMD_FILL_RECT:
    egui_canvas_draw_rectangle_fill(canvas, cmd->x1, cmd->y1, cmd->width, cmd->height, color, EGUI_ALPHA_100);
    break;

  case UI_RENDER_CMD_RECT:
    egui_canvas_draw_line(canvas, cmd->x1, cmd->y1, cmd->x2, cmd->y1, 1, color, EGUI_ALPHA_100);
    egui_canvas_draw_line(canvas, cmd->x1, cmd->y2, cmd->x2, cmd->y2, 1, color, EGUI_ALPHA_100);
    egui_canvas_draw_line(canvas, cmd->x1, cmd->y1, cmd->x1, cmd->y2, 1, color, EGUI_ALPHA_100);
    egui_canvas_draw_line(canvas, cmd->x2, cmd->y1, cmd->x2, cmd->y2, 1, color, EGUI_ALPHA_100);
    break;

  default:
    break;
  }
}

void UI_RendererAdapter_Init(void)
{
  LCD_Init();
}

void UI_RendererAdapter_BeginFrame(void)
{
  g_render_command_count = 0U;
}

void UI_RendererAdapter_EndFrame(void)
{
  uint16_t y;
  uint16_t command_index;
  egui_canvas_t canvas;
  egui_region_t screen_region;
  egui_region_t stripe_region;

  screen_region.location.x = 0;
  screen_region.location.y = 0;
  screen_region.size.width = LCD_W;
  screen_region.size.height = LCD_H;

  for (y = 0U; y < LCD_H; y = (uint16_t)(y + UI_RENDERER_STRIPE_HEIGHT))
  {
    uint16_t stripe_height = ((y + UI_RENDERER_STRIPE_HEIGHT) <= LCD_H) ? UI_RENDERER_STRIPE_HEIGHT : (uint16_t)(LCD_H - y);

    UI_RendererAdapter_FillStripe(UI_RendererAdapter_BackgroundColor(), (uint16_t)(LCD_W * stripe_height));

    stripe_region.location.x = 0;
    stripe_region.location.y = y;
    stripe_region.size.width = LCD_W;
    stripe_region.size.height = stripe_height;

    egui_canvas_init(&canvas, NULL, g_render_stripe, &stripe_region);
    egui_canvas_calc_work_region(&canvas, &screen_region);

    for (command_index = 0U; command_index < g_render_command_count; ++command_index)
    {
      UI_RendererAdapter_DrawCommand(&canvas, &g_render_commands[command_index]);
    }

    LCD_DrawRGB565Buffer(0U, y, LCD_W, stripe_height, g_render_stripe);
  }
}

int16_t UI_RendererAdapter_GetWidth(void)
{
  return LCD_W;
}

int16_t UI_RendererAdapter_GetHeight(void)
{
  return LCD_H;
}

uint16_t UI_RendererAdapter_RGB565(uint8_t red, uint8_t green, uint8_t blue)
{
  return (uint16_t)((((uint16_t)red & 0xF8U) << 8) |
                    (((uint16_t)green & 0xFCU) << 3) |
                    ((uint16_t)blue >> 3));
}

void UI_RendererAdapter_DrawLabel(const ui_viewport_t *viewport, int16_t rx, int16_t ry, const char *text)
{
  UI_RendererAdapter_DrawText(viewport, rx, ry, text, UI_RendererAdapter_RGB565(232U, 238U, 245U));
}

void UI_RendererAdapter_DrawText(const ui_viewport_t *viewport,
                                 int16_t rx,
                                 int16_t ry,
                                 const char *text,
                                 uint16_t color)
{
  ui_render_cmd_t *cmd;
  int16_t x;
  int16_t y;

  if ((viewport == NULL) || (text == NULL))
  {
    return;
  }

  x = viewport->x + rx - viewport->scroll_x;
  y = viewport->y + ry - viewport->scroll_y;

  if ((x < viewport->x) || (y < viewport->y) || (x >= (viewport->x + viewport->width)) || (y >= (viewport->y + viewport->height)))
  {
    return;
  }

  cmd = UI_RendererAdapter_AllocCommand(UI_RENDER_CMD_TEXT);
  if (cmd == NULL)
  {
    return;
  }

  cmd->x1 = x;
  cmd->y1 = y;
  cmd->color = color;
  strncpy(cmd->text, text, sizeof(cmd->text) - 1U);
  cmd->text[sizeof(cmd->text) - 1U] = '\0';
}

void UI_RendererAdapter_DrawFillRect(const ui_viewport_t *viewport,
                                     int16_t rx,
                                     int16_t ry,
                                     int16_t width,
                                     int16_t height,
                                     uint16_t color)
{
  ui_render_cmd_t *cmd;
  int16_t x;
  int16_t y;

  if (viewport == NULL)
  {
    return;
  }

  x = viewport->x + rx - viewport->scroll_x;
  y = viewport->y + ry - viewport->scroll_y;

  if (UI_RendererAdapter_ClipRect(viewport, &x, &y, &width, &height) == 0)
  {
    return;
  }

  cmd = UI_RendererAdapter_AllocCommand(UI_RENDER_CMD_FILL_RECT);
  if (cmd == NULL)
  {
    return;
  }

  cmd->x1 = x;
  cmd->y1 = y;
  cmd->width = width;
  cmd->height = height;
  cmd->color = color;
}

void UI_RendererAdapter_DrawRect(const ui_viewport_t *viewport,
                                 int16_t rx,
                                 int16_t ry,
                                 int16_t width,
                                 int16_t height,
                                 uint16_t color)
{
  ui_render_cmd_t *cmd;
  int16_t x;
  int16_t y;

  if (viewport == NULL)
  {
    return;
  }

  x = viewport->x + rx - viewport->scroll_x;
  y = viewport->y + ry - viewport->scroll_y;

  if (UI_RendererAdapter_ClipRect(viewport, &x, &y, &width, &height) == 0)
  {
    return;
  }

  cmd = UI_RendererAdapter_AllocCommand(UI_RENDER_CMD_RECT);
  if (cmd == NULL)
  {
    return;
  }

  cmd->x1 = x;
  cmd->y1 = y;
  cmd->x2 = x + width - 1;
  cmd->y2 = y + height - 1;
  cmd->color = color;
}

void UI_RendererAdapter_DrawLine(const ui_viewport_t *viewport,
                                 int16_t rx1,
                                 int16_t ry1,
                                 int16_t rx2,
                                 int16_t ry2,
                                 uint16_t color)
{
  ui_render_cmd_t *cmd;
  int16_t x1;
  int16_t y1;
  int16_t x2;
  int16_t y2;

  if (viewport == NULL)
  {
    return;
  }

  x1 = viewport->x + rx1 - viewport->scroll_x;
  y1 = viewport->y + ry1 - viewport->scroll_y;
  x2 = viewport->x + rx2 - viewport->scroll_x;
  y2 = viewport->y + ry2 - viewport->scroll_y;

  if (UI_RendererAdapter_ClipLine(viewport, &x1, &y1, &x2, &y2) == 0)
  {
    return;
  }

  cmd = UI_RendererAdapter_AllocCommand(UI_RENDER_CMD_LINE);
  if (cmd == NULL)
  {
    return;
  }

  cmd->x1 = x1;
  cmd->y1 = y1;
  cmd->x2 = x2;
  cmd->y2 = y2;
  cmd->color = color;
}
