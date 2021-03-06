
#include "gui/controller_settings.h"
#include "gfx.h"
#include "button.h"
#include "label.h"
#include "icon.h"
#include "gui.h"
#include "temp_control.h"
#include "app_cfg.h"
#include "gui/quantity_select.h"
#include "gui/button_list.h"
#include "gui/output_settings.h"
#include "gui/session_action.h"


typedef struct {
  widget_t* screen;

  temp_controller_id_t controller;
  controller_settings_t settings;
} session_action_screen_t;


static void create_session_button_clicked(button_event_t* event);
static void edit_session_button_clicked(button_event_t* event);
static void back_button_clicked(button_event_t* event);
static void cancel_button_clicked(button_event_t* event);
static void session_action_screen_destroy(widget_t* w);
static void display_create_session_button(session_action_screen_t* s);
static void display_edit_session_button(session_action_screen_t* s);
static void display_cancel_button(session_action_screen_t* s);
static void display_back_button(session_action_screen_t* s);
static void display_title(session_action_screen_t* s);

static const widget_class_t session_action_widget_class = {
    .on_destroy = session_action_screen_destroy,
};

widget_t*
session_action_screen_create(temp_controller_id_t controller, controller_settings_t* settings)
{
  session_action_screen_t* s = calloc(1, sizeof(session_action_screen_t));

  s->screen = widget_create(NULL, &session_action_widget_class, s, display_rect);
  widget_set_background(s->screen, BLACK);

  display_title(s);
  display_back_button(s);
  display_create_session_button(s);
  display_edit_session_button(s);
  display_cancel_button(s);

  s->controller = controller;
  s->settings = *settings;

  return s->screen;
}

static void
session_action_screen_destroy(widget_t* w)
{
  session_action_screen_t* s = widget_get_instance_data(w);
  free(s);
}

static void display_title(session_action_screen_t* s)
{
  rect_t rect;

  rect.x = 80;
  rect.y = 15;
  rect.width = 220;
  rect.height = 30;
  label_create(s->screen, rect, "Would you like to update the current session or create a new one?", font_opensans_regular_18, WHITE, 3);
}

static void display_back_button(session_action_screen_t* s)
{
  rect_t rect;

  rect.x = 20;
  rect.y = 20;
  rect.width = 30;
  rect.height = 30;
  widget_t* back_btn = button_create(s->screen, rect, img_left, WHITE, BLACK, back_button_clicked);

  button_set_up_bg_color(back_btn, BLACK);
  button_set_up_icon_color(back_btn, WHITE);
  button_set_down_bg_color(back_btn, BLACK);
  button_set_down_icon_color(back_btn, LIGHT_GRAY);
  button_set_disabled_bg_color(back_btn, BLACK);
  button_set_disabled_icon_color(back_btn, DARK_GRAY);
}

static void display_create_session_button(session_action_screen_t* s)
{
  rect_t rect;

  rect.x = 5;
  rect.y = 80;
  rect.width = 310;
  rect.height = 40;
  widget_t* create_button = button_create(s->screen, rect, NULL, WHITE, BLACK, create_session_button_clicked);

  rect.x = 15;
  rect.y = 5;
  rect.width = 30;
  rect.height = 30;
  icon_create(create_button, rect, img_hysteresis, EMERALD, TRANSPARENT);

  rect.x = 65;
  rect.y = 12;
  rect.width = 235;
  label_create(create_button, rect, "Start New Session", font_opensans_regular_18, WHITE, 1);
}

static void display_edit_session_button(session_action_screen_t* s)
{
  rect_t rect;

  rect.x = 5;
  rect.y = 135;
  rect.width = 310;
  rect.height = 40;
  widget_t* edit_button = button_create(s->screen, rect, NULL, WHITE, BLACK, edit_session_button_clicked);

  rect.x = 15;
  rect.y = 5;
  rect.width = 30;
  rect.height = 30;
  icon_create(edit_button, rect, img_edit, BROWN, TRANSPARENT);

  rect.x = 65;
  rect.y = 12;
  rect.width = 235;
  label_create(edit_button, rect, "Update Current Session", font_opensans_regular_18, WHITE, 1);
}

static void display_cancel_button(session_action_screen_t* s)
{
  rect_t rect;

  rect.x = 5;
  rect.y = 190;
  rect.width = 310;
  rect.height = 40;
  widget_t* cancel_button = button_create(s->screen, rect, NULL, WHITE, BLACK, cancel_button_clicked);

  rect.x = 15;
  rect.y = 5;
  rect.width = 30;
  rect.height = 30;
  icon_create(cancel_button, rect, img_cancel_small, RED, TRANSPARENT);

  rect.x = 65;
  rect.y = 12;
  rect.width = 235;
  label_create(cancel_button, rect, "Cancel Changes", font_opensans_regular_18, WHITE, 1);
}

static void
back_button_clicked(button_event_t* event)
{
  if (event->id == EVT_BUTTON_CLICK)
    gui_pop_screen();
}

static void
edit_session_button_clicked(button_event_t* event)
{
  if (event->id == EVT_BUTTON_CLICK) {
    widget_t* parent = widget_get_parent(event->widget);
    session_action_screen_t* s = widget_get_instance_data(parent);

    s->settings.session_action = EDIT_SESSION;
    app_cfg_set_controller_settings(s->controller, SS_DEVICE, &s->settings);

    gui_pop_screen();
    gui_pop_screen();
  }
}

static void
create_session_button_clicked(button_event_t* event)
{
  if (event->id == EVT_BUTTON_CLICK) {
    widget_t* parent = widget_get_parent(event->widget);
    session_action_screen_t* s = widget_get_instance_data(parent);

    s->settings.session_action = CREATE_SESSION;
    app_cfg_set_controller_settings(s->controller, SS_DEVICE, &s->settings);

    gui_pop_screen();
    gui_pop_screen();
  }
}

static void
cancel_button_clicked(button_event_t* event)
{
  if (event->id == EVT_BUTTON_CLICK) {
    gui_pop_screen();
    gui_pop_screen();
  }
}
