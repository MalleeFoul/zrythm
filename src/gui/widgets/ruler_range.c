/*
 * Copyright (C) 2019 Alexandros Theodotou
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

/** \file
 */

#include "gui/widgets/center_dock.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/ruler_range.h"
#include "gui/widgets/timeline_ruler.h"

#include <gtk/gtk.h>

G_DEFINE_TYPE (RulerRangeWidget,
               ruler_range_widget,
               GTK_TYPE_DRAWING_AREA)

static gboolean
draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  GtkStyleContext *context;

  context = gtk_widget_get_style_context (widget);

  guint width = gtk_widget_get_allocated_width (widget);
  guint height = gtk_widget_get_allocated_height (widget);

  gtk_render_background (context, cr, 0, 0, width, height);

  cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1.0);
  cairo_set_line_width (cr, 2);
  cairo_move_to (cr, 1, 0);
  cairo_line_to (cr, 1, height);
  cairo_move_to (cr, width - 1, 0);
  cairo_line_to (cr, width - 1, height);
  cairo_stroke (cr);

  cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 0.3);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_fill (cr);

  return 0;
}

/**
 * Sets the appropriate cursor.
 */
static void
on_motion (GtkWidget * widget, GdkEventMotion *event)
{
  RulerRangeWidget * self = Z_RULER_RANGE_WIDGET (widget);
  GtkAllocation allocation;
  gtk_widget_get_allocation (widget,
                             &allocation);

  TimelineRulerWidget * trw = MW_RULER;

  if (event->type == GDK_MOTION_NOTIFY)
    {
      if (event->x < RESIZE_CURSOR_SPACE)
        {
          self->cursor_state = UI_CURSOR_STATE_RESIZE_L;
          if (trw->action != UI_OVERLAY_ACTION_MOVING)
            ui_set_cursor (widget, "w-resize");
        }
      else if (event->x > allocation.width -
                 RESIZE_CURSOR_SPACE)
        {
          self->cursor_state = UI_CURSOR_STATE_RESIZE_R;
          if (trw->action != UI_OVERLAY_ACTION_MOVING)
            ui_set_cursor (widget, "e-resize");
        }
      else
        {
          self->cursor_state = UI_CURSOR_STATE_DEFAULT;
          if (trw->action !=
                UI_OVERLAY_ACTION_MOVING &&
              trw->action !=
                UI_OVERLAY_ACTION_STARTING_MOVING &&
              trw->action !=
                UI_OVERLAY_ACTION_RESIZING_L &&
              trw->action !=
                UI_OVERLAY_ACTION_RESIZING_R)
            {
              ui_set_cursor (widget, "default");
            }
        }
    }
  /* if leaving */
  else if (event->type == GDK_LEAVE_NOTIFY)
    {
      if (trw->action != UI_OVERLAY_ACTION_MOVING &&
          trw->action != UI_OVERLAY_ACTION_RESIZING_L &&
          trw->action != UI_OVERLAY_ACTION_RESIZING_R)
        {
          ui_set_cursor (widget, "default");
        }
    }
}

RulerRangeWidget *
ruler_range_widget_new ()
{
  RulerRangeWidget * self =
    g_object_new (RULER_RANGE_WIDGET_TYPE,
                  NULL);

  return self;
}

/**
 * GTK boilerplate.
 */
static void
ruler_range_widget_init (RulerRangeWidget * self)
{
  gtk_widget_add_events (GTK_WIDGET (self),
                         GDK_ALL_EVENTS_MASK);

  /* connect signals */
  g_signal_connect (GTK_WIDGET (self),
                    "draw",
                    G_CALLBACK (draw_cb), NULL);
  g_signal_connect (G_OBJECT (self), "enter-notify-event",
                    G_CALLBACK (on_motion),  self);
  g_signal_connect (G_OBJECT(self), "leave-notify-event",
                    G_CALLBACK (on_motion),  self);
  g_signal_connect (G_OBJECT(self), "motion-notify-event",
                    G_CALLBACK (on_motion),  self);
}

static void
ruler_range_widget_class_init (
  RulerRangeWidgetClass * _klass)
{
  GtkWidgetClass * klass = GTK_WIDGET_CLASS (_klass);
  gtk_widget_class_set_css_name (klass,
                                 "ruler-range");
}
