#ifndef UI_RENDERER_ADAPTER_H
#define UI_RENDERER_ADAPTER_H

#include <stdint.h>

#include "ui_types.h"

void UI_RendererAdapter_Init(void);
void UI_RendererAdapter_BeginFrame(void);
void UI_RendererAdapter_EndFrame(void);
int16_t UI_RendererAdapter_GetWidth(void);
int16_t UI_RendererAdapter_GetHeight(void);
uint16_t UI_RendererAdapter_RGB565(uint8_t red, uint8_t green, uint8_t blue);
void UI_RendererAdapter_DrawLabel(const ui_viewport_t *viewport, int16_t rx, int16_t ry, const char *text);
void UI_RendererAdapter_DrawText(const ui_viewport_t *viewport,
                                 int16_t rx,
                                 int16_t ry,
                                 const char *text,
                                 uint16_t color);
void UI_RendererAdapter_DrawFillRect(const ui_viewport_t *viewport,
                                     int16_t rx,
                                     int16_t ry,
                                     int16_t width,
                                     int16_t height,
                                     uint16_t color);
void UI_RendererAdapter_DrawRect(const ui_viewport_t *viewport,
                                 int16_t rx,
                                 int16_t ry,
                                 int16_t width,
                                 int16_t height,
                                 uint16_t color);
void UI_RendererAdapter_DrawLine(const ui_viewport_t *viewport,
                                 int16_t rx1,
                                 int16_t ry1,
                                 int16_t rx2,
                                 int16_t ry2,
                                 uint16_t color);

#endif /* UI_RENDERER_ADAPTER_H */
