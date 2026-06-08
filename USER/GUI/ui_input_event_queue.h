#ifndef UI_INPUT_EVENT_QUEUE_H
#define UI_INPUT_EVENT_QUEUE_H

#include "FreeRTOS.h"

#include "ui_event.h"

BaseType_t UI_InputEventQueue_Init(void);
BaseType_t UI_InputEventQueue_Publish(const ui_event_t *event);
BaseType_t UI_InputEventQueue_Receive(ui_event_t *event, TickType_t wait_ticks);

#endif /* UI_INPUT_EVENT_QUEUE_H */
