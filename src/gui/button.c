
#include "button.h"
#include "gui.h"
#include "lcd.h"

#include <stdlib.h>


#define BORDER_COLOR   COLOR(0x88, 0x88, 0x88)
#define BTN_DOWN_COLOR COLOR(0xCC, 0xCC, 0xCC)


typedef struct {
  bool is_down;
  const char* text;
  const Image_t* icon;

  click_handler_t on_click;
} button_t;


static void button_touch(touch_event_t* event);
static void button_paint(paint_event_t* event);
static void button_destroy(widget_t* w);


static const widget_class_t button_widget_class = {
    .on_touch = button_touch,
    .on_paint = button_paint,
    .on_destroy = button_destroy,
};

widget_t*
button_create(rect_t rect, const char* text, const Image_t* icon, click_handler_t click_handler)
{
  button_t* b = calloc(1, sizeof(button_t));

  b->text = text;
  b->icon = icon;
  b->on_click = click_handler;

  return widget_create(&button_widget_class, b, rect);
}

static void
button_destroy(widget_t* w)
{
  button_t* b = widget_get_instance_data(w);
  free(b);
}

static void
button_touch(touch_event_t* event)
{
  button_t* b = widget_get_instance_data(event->widget);

  if (event->id == EVT_TOUCH_DOWN) {
    if (!b->is_down) {
      b->is_down = true;
      gui_acquire_touch_capture(event->widget);
      widget_invalidate(event->widget);
    }
  }
  else {
    if (b->is_down) {
      b->is_down = false;
      gui_release_touch_capture();
      if (rect_inside(widget_get_rect(event->widget), event->pos)) {
        if (b->on_click) {
          click_event_t ce = {
              .id = EVT_CLICK,
              .widget = event->widget,
              .pos = event->pos,
          };
          b->on_click(&ce);
        }
      }
      widget_invalidate(event->widget);
    }
  }
}

static void
button_paint(paint_event_t* event)
{
  button_t* b = widget_get_instance_data(event->widget);

  rect_t rect = widget_get_rect(event->widget);
  point_t center = rect_center(rect);

  /* draw border */
  setColor(BORDER_COLOR);
  drawRect(rect);

  /* draw background */
  rect.x += 1;
  rect.y += 1;
  rect.width -= 2;
  rect.height -= 2;
  if (b->is_down) {
    setColor(BTN_DOWN_COLOR);
    set_bg_color(BTN_DOWN_COLOR);
    fillRect(rect);
  }
  else {
    point_t bg_anchor = { .x = rect.x, .y = rect.y };
    set_bg_img(img_button_bg, bg_anchor);
    tile_bitmap(img_button_bg, rect);
  }

  /* draw text */
  if (b->text != NULL) {
    setColor(BLACK);
    setFont(font_terminal);
    Extents_t text_extents = font_text_extents(font_terminal, b->text);
    print(b->text,
        center.x - (text_extents.width / 2),
        center.y - (text_extents.height / 2));
  }

  /* draw icon */
  if (b->icon != NULL) {
    drawBitmap(
        rect.x + 10,
        center.y - (b->icon->height / 2),
        b->icon);
  }
}