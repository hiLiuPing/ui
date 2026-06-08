#ifndef UI_PAGE_H
#define UI_PAGE_H

#include <stdint.h>

#include "ui_event.h"
#include "ui_message_bus.h"
#include "ui_widget.h"

typedef struct ui_page_context ui_page_context_t;

typedef void (*ui_page_lifecycle_fn)(ui_page_context_t *page);
typedef void (*ui_page_message_fn)(ui_page_context_t *page, const ui_msg_t *msg);
typedef void (*ui_page_event_fn)(ui_page_context_t *page, const ui_event_t *event);

struct ui_page_context
{
  uint16_t page_id;
  uint8_t is_created;
  ui_widget_t *root_widget;
  void *user_data;
  ui_page_lifecycle_fn create;
  ui_page_lifecycle_fn enter;
  ui_page_lifecycle_fn process;
  ui_page_lifecycle_fn draw;
  ui_page_lifecycle_fn exit;
  ui_page_lifecycle_fn destroy;
  ui_page_message_fn handle_message;
  ui_page_event_fn handle_event;
};

#endif /* UI_PAGE_H */
