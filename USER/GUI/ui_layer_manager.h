#ifndef UI_LAYER_MANAGER_H
#define UI_LAYER_MANAGER_H

#include <stdint.h>

#include "ui_types.h"

typedef enum
{
  UI_LAYER_BASE = 0,
  UI_LAYER_CLIMATE,
  UI_LAYER_FX_PARTICLE,
  UI_LAYER_TOP_WINDOW,
  UI_LAYER_COUNT
} ui_layer_id_t;

typedef void (*ui_layer_draw_fn)(const ui_viewport_t *viewport, void *user_data);

void UI_LayerManager_Init(void);
void UI_LayerManager_SetDrawCallback(ui_layer_id_t layer, ui_layer_draw_fn draw_cb, void *user_data);
void UI_LayerManager_Draw(const ui_viewport_t *viewport);

#endif /* UI_LAYER_MANAGER_H */
