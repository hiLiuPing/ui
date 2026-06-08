#include "core/egui_api.h"

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void egui_api_log(const char *format, ...)
{
  (void)format;
}

void egui_api_assert(const char *file, int line)
{
  (void)file;
  (void)line;
  Error_Handler();
}

void egui_api_free(egui_core_t *core, void *ptr)
{
  (void)core;
  (void)ptr;
}

void *egui_api_malloc(egui_core_t *core, int size)
{
  (void)core;
  (void)size;
  return NULL;
}

int egui_api_get_mem_monitor(egui_core_t *core, egui_mem_monitor_t *monitor)
{
  (void)core;
  (void)monitor;
  return 0;
}

void egui_api_sprintf(char *str, const char *format, ...)
{
  va_list args;

  va_start(args, format);
  (void)vsprintf(str, format, args);
  va_end(args);
}

void egui_api_draw_data(egui_core_t *core, int16_t x, int16_t y, int16_t width, int16_t height, const egui_color_int_t *data)
{
  (void)core;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)data;
}

void egui_api_refresh_display(egui_core_t *core)
{
  (void)core;
}

void egui_api_timer_start(egui_core_t *core, uint32_t ms)
{
  (void)core;
  (void)ms;
}

void egui_api_timer_stop(egui_core_t *core)
{
  (void)core;
}

uint32_t egui_api_timer_get_current_core(egui_core_t *core)
{
  (void)core;
  return (uint32_t)xTaskGetTickCount();
}

uint32_t egui_api_timer_get_current(void)
{
  return (uint32_t)xTaskGetTickCount();
}

void egui_api_delay_core(egui_core_t *core, uint32_t ms)
{
  (void)core;
  vTaskDelay(pdMS_TO_TICKS(ms));
}

void egui_api_delay(uint32_t ms)
{
  vTaskDelay(pdMS_TO_TICKS(ms));
}

void egui_api_pfb_clear(void *s, int n)
{
  memset(s, 0, (size_t)n);
}

void egui_api_memset(void *s, int c, int n)
{
  memset(s, c, (size_t)n);
}

void egui_api_memcpy(void *dst, const void *src, int n)
{
  memcpy(dst, src, (size_t)n);
}

void egui_api_load_external_resource(const egui_canvas_t *canvas, void *dest, egui_uintptr_t res_id, uint32_t start_offset, uint32_t size)
{
  const uint8_t *src = (const uint8_t *)res_id;

  (void)canvas;

  if ((dest == NULL) || (src == NULL))
  {
    return;
  }

  memcpy(dest, &src[start_offset], size);
}

egui_base_t egui_hw_interrupt_disable(void)
{
  taskENTER_CRITICAL();
  return 0;
}

void egui_hw_interrupt_enable(egui_base_t level)
{
  (void)level;
  taskEXIT_CRITICAL();
}
