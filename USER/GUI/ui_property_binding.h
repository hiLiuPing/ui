#ifndef UI_PROPERTY_BINDING_H
#define UI_PROPERTY_BINDING_H

#include "FreeRTOS.h"

#include <stdint.h>

#include "ui_types.h"

void UI_PropertyBinding_Init(void);
BaseType_t UI_PropertyBinding_BindRect(uint16_t property_id, const ui_rect_t *rect);
void UI_PropertyBinding_UpdateDirtyRegions(void);

#endif /* UI_PROPERTY_BINDING_H */
