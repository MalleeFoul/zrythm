// SPDX-FileCopyrightText: © 2018-2022 Alexandros Theodotou <alex@zrythm.org>
// SPDX-License-Identifier: LicenseRef-ZrythmLicense

/**
 * \file
 *
 * Project schema.
 */

#ifndef __SCHEMAS_PROJECT_H__
#define __SCHEMAS_PROJECT_H__

#include "schemas/dsp/engine.h"
#include "schemas/dsp/midi_mapping.h"
#include "schemas/dsp/port_connections_manager.h"
#include "schemas/dsp/quantize_options.h"
#include "schemas/dsp/region_link_group_manager.h"
#include "schemas/dsp/snap_grid.h"
#include "schemas/dsp/tracklist.h"
#include "schemas/gui/backend/audio_selections.h"
#include "schemas/gui/backend/automation_selections.h"
#include "schemas/gui/backend/chord_selections.h"
#include "schemas/gui/backend/clip_editor.h"
#include "schemas/gui/backend/midi_arranger_selections.h"
#include "schemas/gui/backend/mixer_selections.h"
#include "schemas/gui/backend/timeline.h"
#include "schemas/gui/backend/timeline_selections.h"
#include "schemas/gui/backend/tracklist_selections.h"

typedef enum SelectionType_v1
{
  SELECTION_TYPE_TRACKLIST_v1,
  SELECTION_TYPE_TIMELINE_v1,
  SELECTION_TYPE_INSERT_v1,
  SELECTION_TYPE_MIDI_FX_v1,
  SELECTION_TYPE_INSTRUMENT_v1,
  SELECTION_TYPE_MODULATOR_v1,
  SELECTION_TYPE_EDITOR_v1,
} SelectionType_v1;

static const cyaml_strval_t selection_type_strings_v1[] = {
  {"Tracklist",   SELECTION_TYPE_TRACKLIST_v1 },
  { "Timeline",   SELECTION_TYPE_TIMELINE_v1  },
  { "Insert",     SELECTION_TYPE_INSERT_v1    },
  { "MIDI FX",    SELECTION_TYPE_MIDI_FX_v1   },
  { "Instrument", SELECTION_TYPE_INSTRUMENT_v1},
  { "Modulator",  SELECTION_TYPE_MODULATOR_v1 },
  { "Editor",     SELECTION_TYPE_EDITOR_v1    },
};

typedef struct Project_v1
{
  int               schema_version;
  char *            title;
  char *            datetime_str;
  char *            version;
  Tracklist_v1 *    tracklist;
  ClipEditor *      clip_editor;
  Timeline *        timeline;
  SnapGrid *        snap_grid_timeline;
  QuantizeOptions * quantize_opts_timeline;
  SnapGrid *        snap_grid_editor;
  QuantizeOptions * quantize_opts_editor;
  //AutomationSelections *   automation_selections;
  //AudioSelections *        audio_selections;
  //ChordSelections *        chord_selections;
  //TimelineSelections_v1 *  timeline_selections;
  //MidiArrangerSelections * midi_arranger_selections;
  TracklistSelections_v1 * tracklist_selections;
  //MixerSelections *        mixer_selections;
  RegionLinkGroupManager * region_link_group_manager;
  PortConnectionsManager * port_connections_manager;
  AudioEngine_v1 *         audio_engine;
  MidiMappings *           midi_mappings;
  SelectionType            last_selection;
} Project_v1;

static const cyaml_schema_field_t project_fields_schema_v1[] = {
  YAML_FIELD_INT (Project_v1, schema_version),
  YAML_FIELD_STRING_PTR (Project_v1, title),
  YAML_FIELD_STRING_PTR (Project_v1, datetime_str),
  YAML_FIELD_STRING_PTR (Project_v1, version),
  YAML_FIELD_MAPPING_PTR (Project_v1, tracklist, tracklist_fields_schema_v1),
  YAML_FIELD_MAPPING_PTR (Project_v1, clip_editor, clip_editor_fields_schema),
  YAML_FIELD_MAPPING_PTR (Project_v1, timeline, timeline_fields_schema),
  YAML_FIELD_MAPPING_PTR (Project_v1, snap_grid_timeline, snap_grid_fields_schema),
  YAML_FIELD_MAPPING_PTR (Project_v1, snap_grid_editor, snap_grid_fields_schema),
  YAML_FIELD_MAPPING_PTR (
    Project_v1,
    quantize_opts_timeline,
    quantize_options_fields_schema),
  YAML_FIELD_MAPPING_PTR (
    Project_v1,
    quantize_opts_editor,
    quantize_options_fields_schema),
  YAML_FIELD_MAPPING_PTR (Project_v1, audio_engine, engine_fields_schema_v1),
  YAML_FIELD_IGNORE_OPT ("mixer_selections"),
  YAML_FIELD_IGNORE_OPT ("timeline_selections"),
  YAML_FIELD_IGNORE_OPT ("midi_arranger_selections"),
  YAML_FIELD_IGNORE_OPT ("automation_selections"),
  YAML_FIELD_IGNORE_OPT ("chord_selections"),
  YAML_FIELD_IGNORE_OPT ("audio_selections"),
  YAML_FIELD_MAPPING_PTR (
    Project_v1,
    tracklist_selections,
    tracklist_selections_fields_schema_v1),
  YAML_FIELD_MAPPING_PTR (
    Project_v1,
    region_link_group_manager,
    region_link_group_manager_fields_schema),
  YAML_FIELD_MAPPING_PTR (
    Project_v1,
    port_connections_manager,
    port_connections_manager_fields_schema),
  YAML_FIELD_MAPPING_PTR (Project_v1, midi_mappings, midi_mappings_fields_schema),
  /* ignore undo history */
  YAML_FIELD_IGNORE_OPT ("undo_manager"),
  YAML_FIELD_ENUM (Project_v1, last_selection, selection_type_strings),

  CYAML_FIELD_END
};

static const cyaml_schema_value_t project_schema_v1 = {
  YAML_VALUE_PTR (Project_v1, project_fields_schema_v1),
};

Project *
schemas_project_upgrade_from_v1 (Project_v1 * v1);

Project *
schemas_project_deserialize (const char * yaml);

#endif
