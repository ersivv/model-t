
#include "ch.h"
#include "widget.h"
#include "common.h"
#include "gfx.h"

#include <string.h>


#define CALL_WC(w, m)   if ((w)->widget_class != NULL && (w)->widget_class->m != NULL) (w)->widget_class->m


typedef struct widget_s {
  const widget_class_t* widget_class;
  void* instance_data;

  struct widget_s* parent;

  struct widget_s* first_child;
  struct widget_s* last_child;

  struct widget_s* next_sibling;
  struct widget_s* prev_sibling;

  rect_t rect;
  bool invalid;
  bool visible;

  bool bg_transparent;
  color_t bg_color;
} widget_t;


static void
widget_invalidate_predicate(widget_t* w, widget_traversal_event_t event);

static void
widget_paint_predicate(widget_t* w, widget_traversal_event_t event);

static void
widget_destroy_predicate(widget_t* w, widget_traversal_event_t event);

static void
dispatch_touch(widget_t* w, touch_event_t* event);


widget_t*
widget_create(widget_t* parent, const widget_class_t* widget_class, void* instance_data, rect_t rect)
{
  widget_t* w = calloc(1, sizeof(widget_t));

  w->widget_class = widget_class;
  w->instance_data = instance_data;

  w->rect = rect;
  w->invalid = true;
  w->visible = true;
  w->bg_color = BLACK;
  w->bg_transparent = true;

  if (parent != NULL)
    widget_add_child(parent, w);

  return w;
}

void
widget_destroy(widget_t* w)
{
  widget_for_each(w, widget_destroy_predicate);
}

static void
widget_destroy_predicate(widget_t* w, widget_traversal_event_t event)
{
  if (event == WIDGET_TRAVERSAL_AFTER_CHILDREN) {
    CALL_WC(w, on_destroy)(w);
    free(w);
  }
}

widget_t*
widget_get_parent(widget_t* w)
{
  return w->parent;
}

rect_t
widget_get_rect(widget_t* w)
{
  return w->rect;
}

void*
widget_get_instance_data(widget_t* w)
{
  return w->instance_data;
}

void
widget_add_child(widget_t* parent, widget_t* child)
{
  if (parent->last_child != NULL) {
    parent->last_child->next_sibling = child;
    child->prev_sibling = parent->last_child;
    parent->last_child = child;
  }
  else {
    chDbgAssert(parent->first_child == NULL, "widget_add_child(), #1", "");

    parent->first_child = parent->last_child = child;
  }

  child->parent = parent;
}

void
widget_unparent(widget_t* w)
{
  if (w->prev_sibling != NULL)
    w->prev_sibling->next_sibling = w->next_sibling;
  if (w->next_sibling != NULL)
    w->next_sibling->prev_sibling = w->prev_sibling;

  w->next_sibling = NULL;
  w->prev_sibling = NULL;
  w->parent = NULL;
}

void
widget_for_each(widget_t* w, widget_predicate_t pred)
{
  widget_t* child;
  widget_t* next_child = NULL;

  pred(w, WIDGET_TRAVERSAL_BEFORE_CHILDREN);

  for (child = w->first_child; child != NULL; child = next_child) {
    /*  Save the next child locally because this child might be destroyed
     *  inside the predicate...
     */
    next_child = child->next_sibling;
    widget_for_each(child, pred);
  }

  pred(w, WIDGET_TRAVERSAL_AFTER_CHILDREN);
}

widget_t*
widget_hit_test(widget_t* root, point_t p)
{
  widget_t* w;

  for (w = root->first_child; w != NULL; w = w->next_sibling) {
    widget_t* w_hit = widget_hit_test(w, p);
    if (w_hit != NULL)
      return w_hit;
  }

  if (widget_is_visible(root) && rect_inside(root->rect, p))
    return root;

  return NULL;
}

void
widget_dispatch_event(widget_t* w, event_t* event)
{
  switch (event->id) {
  case EVT_PAINT:
    break;

  case EVT_TOUCH_UP:
  case EVT_TOUCH_DOWN:
    dispatch_touch(w, (touch_event_t*)event);
    break;

  default:
    break;
  }
}

static void
dispatch_touch(widget_t* w, touch_event_t* event)
{
  // TODO bubble touch up to parent if this widget doesn't handle touches
  CALL_WC(w, on_touch)(event);
}

void
widget_paint(widget_t* w)
{
  widget_for_each(w, widget_paint_predicate);
}

static void
widget_paint_predicate(widget_t* w, widget_traversal_event_t event)
{
  if (event == WIDGET_TRAVERSAL_BEFORE_CHILDREN) {
    if (w->invalid && widget_is_visible(w)) {
      paint_event_t event = {
          .id = EVT_PAINT,
          .widget = w,
      };

      gfx_ctx_push();

      if (!w->bg_transparent) {
        gfx_set_bg_color(w->bg_color);
        gfx_clear_rect(w->rect);
      }

      CALL_WC(w, on_paint)(&event);

      w->invalid = false;
    }
  }
  else {
    gfx_ctx_pop();
  }
}

void
widget_invalidate(widget_t* w)
{
  if (w == NULL)
    return;

  widget_for_each(w, widget_invalidate_predicate);
}

static void
widget_invalidate_predicate(widget_t* w, widget_traversal_event_t event)
{
  if (event == WIDGET_TRAVERSAL_BEFORE_CHILDREN)
    w->invalid = true;
}

void
widget_hide(widget_t* w)
{
  if (w->visible) {
    w->visible = false;
    widget_invalidate(w->parent);
  }
}

void
widget_show(widget_t* w)
{
  if (!w->visible) {
    w->visible = true;
    widget_invalidate(w);
  }
}

bool
widget_is_visible(widget_t* w)
{
  return w->visible && (w->parent == NULL || widget_is_visible(w->parent));
}

void
widget_set_background(widget_t* w, color_t color, bool transparent)
{
  w->bg_color = color;
  w->bg_transparent = transparent;
  widget_invalidate(w);
}
