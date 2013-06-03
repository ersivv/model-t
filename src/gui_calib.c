
#include "gui_calib.h"
#include "gui.h"
#include "touch_calib.h"
#include "touch.h"
#include "terminal.h"
#include "lcd.h"
#include "image.h"


#define NUM_SAMPLES_PER_POINT 128


typedef struct {
  int ref_pt_idx;
  uint8_t calib_complete;
  point_t sampled_pts[NUM_CALIB_POINTS][NUM_SAMPLES_PER_POINT];
  int sample_idx;
  widget_t* widget;
  touch_handler_t touch_handler;
  point_t last_touch_pos;
} calib_screen_t;


static void calib_widget_touch(touch_event_t* event);
static void calib_widget_paint(paint_event_t* event);

static void calib_raw_touch(bool touch_down, point_t raw, point_t calib, void* user_data);
static void calib_touch_down(calib_screen_t* s, point_t p);
static void calib_touch_up(calib_screen_t* s, point_t p);


static const point_t ref_pts[NUM_CALIB_POINTS] = {
    {  50,  50 },
    { 270, 120 },
    { 170, 190 },
};

static const widget_class_t calib_widget_class = {
    .on_paint = calib_widget_paint,
    .on_touch = calib_widget_touch,
};

widget_t*
calib_screen_create()
{
  calib_screen_t* calib_screen = calloc(1, sizeof(calib_screen_t));

  calib_screen->widget = widget_create(&calib_widget_class, calib_screen, display_rect);

  touch_handler_register(calib_raw_touch, calib_screen);

  return calib_screen->widget;
}

static void
calib_widget_paint(paint_event_t* event)
{
  calib_screen_t* s = widget_get_instance_data(event->widget);
  const point_t* marker_pos;

  tile_bitmap(img_squares, 0, 0, DISP_WIDTH, DISP_HEIGHT);

  if (!s->calib_complete) {
    marker_pos = &ref_pts[s->ref_pt_idx];

    if (s->sample_idx == 0)
      setColor(YELLOW);
    else if (s->sample_idx < NUM_SAMPLES_PER_POINT)
      setColor(RED);
    else
      setColor(GREEN);
  }
  else {
    marker_pos = &s->last_touch_pos;
    setColor(BLUE);
  }
  fillCircle(marker_pos->x, marker_pos->y, 25);
}

static void
calib_widget_touch(touch_event_t* event)
{
  calib_screen_t* s = widget_get_instance_data(event->widget);

  if (s->calib_complete) {
    s->last_touch_pos = event->pos;
    widget_invalidate(event->widget);
  }
}

static void
calib_raw_touch(bool touch_down, point_t raw, point_t calib, void* user_data)
{
  calib_screen_t* s = user_data;

  (void)calib;

  if (!s->calib_complete) {
    if (touch_down)
      calib_touch_down(s, raw);
    else
      calib_touch_up(s, raw);
  }
  else {
    s->last_touch_pos = calib;
    widget_invalidate(s->widget);
  }
}

static void
calib_touch_down(calib_screen_t* s, point_t p)
{
  s->sampled_pts[s->ref_pt_idx][s->sample_idx % NUM_SAMPLES_PER_POINT] = p;
  s->sample_idx++;

  if ((s->sample_idx == 1) ||
      (s->sample_idx == NUM_SAMPLES_PER_POINT))
    widget_invalidate(s->widget);
}

static void
calib_touch_up(calib_screen_t* s, point_t p)
{
  (void)p;

  if (++s->ref_pt_idx < NUM_CALIB_POINTS) {
    s->sample_idx = 0;
    widget_invalidate(s->widget);
  }
  else {
    int i,j;
    s->calib_complete = 1;
    point_t avg_sample[3];
    for (i = 0; i < 3; ++i) {
      avg_sample[i].x = 0;
      avg_sample[i].y = 0;
      for (j = 0; j < NUM_SAMPLES_PER_POINT; ++j) {
        avg_sample[i].x += s->sampled_pts[i][j].x;
        avg_sample[i].y += s->sampled_pts[i][j].y;
      }
      avg_sample[i].x /= NUM_SAMPLES_PER_POINT;
      avg_sample[i].y /= NUM_SAMPLES_PER_POINT;
    }
    touch_calibrate(ref_pts, avg_sample);
  }
}

