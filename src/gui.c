
#include "ch.h"
#include "gui.h"
#include "touch.h"


typedef enum {
  GUI_TOUCH_DOWN,
  GUI_TOUCH_UP,
  GUI_SET_SCREEN,
  GUI_PAINT,
} gui_event_id_t;

typedef struct {
  gui_event_id_t id;
} gui_event_t;

typedef struct {
  gui_event_id_t id;
  point_t pos;
} gui_touch_event_t;

typedef struct {
  gui_event_id_t id;
  screen_t* screen;
} gui_set_screen_event_t;


static msg_t gui_thread_func(void* arg);
static void handle_touch(bool touch_down, point_t raw, point_t calib);
static void gui_send_event(gui_event_t* event);

static void handle_touch_down(gui_touch_event_t* event);
static void handle_touch_up(gui_touch_event_t* event);
static void handle_set_screen(gui_set_screen_event_t* event);
static void gui_dispatch(gui_event_t* event);
static void gui_idle(void);


static uint8_t wa_gui_thread[1024];
static Thread* gui_thread;
static screen_t* screen;

static touch_handler_t touch_handler = {
    .on_touch = handle_touch
};

void
gui_init()
{
  gui_thread = chThdCreateStatic(wa_gui_thread, sizeof(wa_gui_thread), NORMALPRIO, gui_thread_func, NULL);

  touch_handler_register(&touch_handler);
}

static void
handle_touch(bool touch_down, point_t raw, point_t calib)
{
  (void)raw;

  gui_touch_event_t event = {
      .id = touch_down ? GUI_TOUCH_DOWN : GUI_TOUCH_UP,
      .pos = calib,
  };
  gui_send_event((gui_event_t*)&event);
}

void
gui_set_screen(screen_t* screen)
{
  gui_set_screen_event_t event = {
      .id = GUI_SET_SCREEN,
      .screen = screen,
  };
  gui_send_event((gui_event_t*)&event);
}

void
gui_paint()
{
  gui_event_t event = {
      .id = GUI_PAINT
  };
  gui_send_event(&event);
}

static void
gui_send_event(gui_event_t* event)
{
  chMsgSend(gui_thread, (msg_t)event);
}

static msg_t
gui_thread_func(void* arg)
{
  (void)arg;

  while (1) {
    if (chMsgIsPendingI(gui_thread)) {
      Thread* tp = chMsgWait();
      gui_event_t* event = (gui_event_t*)chMsgGet(tp);

      gui_dispatch(event);

      chMsgRelease(tp, 0);
    }
    else {
      gui_idle();
    }
  }
  return 0;
}

static void
gui_idle()
{
  widget_paint(screen);

  chThdYield();
}

static void
gui_dispatch(gui_event_t* event)
{
  switch(event->id) {
  case GUI_PAINT:
    widget_paint(screen);
    break;

  case GUI_TOUCH_DOWN:
    handle_touch_down((gui_touch_event_t*)event);
    break;

  case GUI_TOUCH_UP:
    handle_touch_up((gui_touch_event_t*)event);
    break;

  case GUI_SET_SCREEN:
    handle_set_screen((gui_set_screen_event_t*)event);
    break;

  default:
    break;
  }
}

static void
handle_touch_down(gui_touch_event_t* event)
{
//  touch_event_t te = {
//
//  };
//  widget_t* widget = widget_hit_test((widget_t*)screen, event->pos);
//  if (widget != NULL) {
//    widget_dispatch_event(widget, (event_t*)&te);
//  }
}

static void
handle_touch_up(gui_touch_event_t* event)
{

}

static void
handle_set_screen(gui_set_screen_event_t* event)
{
  terminal_write("setting screen\n");
  screen = event->screen;
  widget_paint((widget_t*)screen);
}
