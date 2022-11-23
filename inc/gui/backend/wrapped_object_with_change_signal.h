// SPDX-FileCopyrightText: © 2021-2022 Alexandros Theodotou <alex@zrythm.org>
// SPDX-License-Identifier: LicenseRef-ZrythmLicense

/**
 * \file
 *
 * Object with change signal.
 */

#ifndef __GUI_BACKEND_WRAPPED_OBJECT_WITH_CHANGE_SIGNAL_H__
#define __GUI_BACKEND_WRAPPED_OBJECT_WITH_CHANGE_SIGNAL_H__

#include <glib-object.h>

#define WRAPPED_OBJECT_WITH_CHANGE_SIGNAL_TYPE \
  (wrapped_object_with_change_signal_get_type ())
G_DECLARE_FINAL_TYPE (
  WrappedObjectWithChangeSignal,
  wrapped_object_with_change_signal,
  Z,
  WRAPPED_OBJECT_WITH_CHANGE_SIGNAL,
  GObject)

/**
 * @addtogroup widgets
 *
 * @{
 */

typedef enum WrappedObjectType
{
  WRAPPED_OBJECT_TYPE_TRACK,
  WRAPPED_OBJECT_TYPE_PLUGIN,
  WRAPPED_OBJECT_TYPE_PLUGIN_DESCR,
  WRAPPED_OBJECT_TYPE_CHORD_DESCR,
  WRAPPED_OBJECT_TYPE_CHORD_PSET,
  WRAPPED_OBJECT_TYPE_CHORD_PSET_PACK,
  WRAPPED_OBJECT_TYPE_SUPPORTED_FILE,
  WRAPPED_OBJECT_TYPE_MIDI_MAPPING,
  WRAPPED_OBJECT_TYPE_ARRANGER_OBJECT,
  WRAPPED_OBJECT_TYPE_PROJECT_INFO,
  WRAPPED_OBJECT_TYPE_PORT,
} WrappedObjectType;

typedef struct _WrappedObjectWithChangeSignal
{
  GObject parent_instance;

  WrappedObjectType type;
  void *            obj;
} WrappedObjectWithChangeSignal;

/**
 * Fires the signal.
 */
void
wrapped_object_with_change_signal_fire (
  WrappedObjectWithChangeSignal * self);

/**
 * Returns a display name for the given object,
 * intended to be used where the object should be
 * displayed (eg, a dropdown).
 *
 * This can be used with GtkCclosureExpression.
 */
char *
wrapped_object_with_change_signal_get_display_name (
  void * data);

/**
 * Instantiates a new WrappedObjectWithChangeSignal.
 */
WrappedObjectWithChangeSignal *
wrapped_object_with_change_signal_new (
  void *            obj,
  WrappedObjectType type);

/**
 * @}
 */

#endif
