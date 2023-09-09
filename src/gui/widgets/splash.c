// SPDX-FileCopyrightText: © 2018-2023 Alexandros Theodotou <alex@zrythm.org>
// SPDX-License-Identifier: LicenseRef-ZrythmLicense

#include "zrythm-config.h"

#include "dsp/engine.h"
#include "gui/backend/event.h"
#include "gui/backend/event_manager.h"
#include "gui/widgets/custom_image.h"
#include "gui/widgets/splash.h"
#include "project.h"
#include "utils/gtk.h"
#include "utils/resources.h"
#include "zrythm.h"
#include "zrythm_app.h"

#include <glib/gi18n.h>

G_DEFINE_TYPE (SplashWindowWidget, splash_window_widget, GTK_TYPE_WINDOW)

static gboolean
splash_tick_cb (
  GtkWidget *          widget,
  GdkFrameClock *      frame_clock,
  SplashWindowWidget * self)
{
  zix_sem_wait (&zrythm_app->progress_status_lock);
  gtk_label_set_text (self->status_label, zrythm_app->status);
  gtk_progress_bar_set_fraction (self->progress_bar, ZRYTHM->progress);
  zix_sem_post (&zrythm_app->progress_status_lock);

  return G_SOURCE_CONTINUE;
}

void
splash_window_widget_close (SplashWindowWidget * self)
{
  g_debug ("closing splash window");

  gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->tick_cb_id);
  gtk_window_close (GTK_WINDOW (self));

  EVENTS_PUSH (ET_SPLASH_CLOSED, NULL);
}

static void
finalize (SplashWindowWidget * self)
{
  g_debug ("finalizing splash screen");

  if (zrythm_app->init_thread)
    {
      g_thread_join (zrythm_app->init_thread);
      zrythm_app->init_thread = NULL;
    }

  G_OBJECT_CLASS (splash_window_widget_parent_class)->finalize (G_OBJECT (self));
}

SplashWindowWidget *
splash_window_widget_new (ZrythmApp * app)
{
  SplashWindowWidget * self = g_object_new (
    SPLASH_WINDOW_WIDGET_TYPE, "application", G_APPLICATION (app), "title",
    PROGRAM_NAME, NULL);
  g_return_val_if_fail (Z_IS_SPLASH_WINDOW_WIDGET (self), NULL);

  gtk_progress_bar_set_fraction (self->progress_bar, 0.0);

  self->tick_cb_id = gtk_widget_add_tick_callback (
    (GtkWidget *) self, (GtkTickCallback) splash_tick_cb, self, NULL);

  return self;
}

static void
splash_window_widget_class_init (SplashWindowWidgetClass * _klass)
{
  GtkWidgetClass * wklass = GTK_WIDGET_CLASS (_klass);
  resources_set_class_template (wklass, "splash.ui");
  gtk_widget_class_set_accessible_role (wklass, GTK_ACCESSIBLE_ROLE_DIALOG);

#define BIND_CHILD(x) \
  gtk_widget_class_bind_template_child (wklass, SplashWindowWidget, x)

  BIND_CHILD (img);
  BIND_CHILD (status_label);
  BIND_CHILD (version_label);
  BIND_CHILD (progress_bar);

#undef BIND_CHILD

  GObjectClass * oklass = G_OBJECT_CLASS (_klass);
  oklass->finalize = (GObjectFinalizeFunc) finalize;
}

static void
splash_window_widget_init (SplashWindowWidget * self)
{
  g_type_ensure (CUSTOM_IMAGE_WIDGET_TYPE);

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_add_css_class (GTK_WIDGET (self), "splash");

  GdkTexture * texture =
    z_gdk_texture_new_from_icon_name ("zrythm-splash-png", 580, -1, 1);
  custom_image_widget_set_texture (self->img, texture);

  char * ver = zrythm_get_version (true);
  char   ver_str[800];
  sprintf (ver_str, "<small>%s</small>", ver);
  g_free (ver);
  gtk_label_set_markup (self->version_label, ver_str);

  gtk_accessible_update_relation (
    GTK_ACCESSIBLE (self->progress_bar), GTK_ACCESSIBLE_RELATION_LABELLED_BY,
    self->status_label, NULL, -1);
}
