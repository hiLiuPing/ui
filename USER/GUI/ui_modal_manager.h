#ifndef UI_MODAL_MANAGER_H
#define UI_MODAL_MANAGER_H

#include "FreeRTOS.h"

#include <stdint.h>

#include "ui_event.h"
#include "ui_types.h"

typedef void (*ui_modal_event_fn)(const ui_event_t *event, void *user_data);

void UI_ModalManager_Init(void);
BaseType_t UI_ModalManager_Show(const ui_rect_t *rect,
                                const char *title,
                                const char *message,
                                uint16_t timeout_frames,
                                ui_modal_event_fn event_cb,
                                void *user_data);
void UI_ModalManager_Dismiss(void);
uint8_t UI_ModalManager_IsActive(void);
BaseType_t UI_ModalManager_HandleEvent(const ui_event_t *event);
void UI_ModalManager_Process(void);
void UI_ModalManager_Draw(const ui_viewport_t *viewport);

#endif /* UI_MODAL_MANAGER_H */
