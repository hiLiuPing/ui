#ifndef UI_PAGE_MANAGER_H
#define UI_PAGE_MANAGER_H

#include "FreeRTOS.h"

#include "ui_event.h"
#include "ui_message_bus.h"
#include "ui_page.h"

typedef ui_page_context_t *(*ui_page_factory_fn)(uint16_t page_id);

void UI_PageManager_Init(ui_page_factory_fn factory);
BaseType_t UI_PageManager_OpenRoot(uint16_t page_id);
BaseType_t UI_PageManager_Push(uint16_t page_id);
BaseType_t UI_PageManager_Pop(void);
void UI_PageManager_HandleMessage(const ui_msg_t *msg);
void UI_PageManager_DispatchEvent(const ui_event_t *event);
void UI_PageManager_ProcessActivePage(void);
void UI_PageManager_DrawActivePage(void);
ui_page_context_t *UI_PageManager_GetActivePage(void);

#endif /* UI_PAGE_MANAGER_H */
