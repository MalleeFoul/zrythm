// SPDX-FileCopyrightText: © 2019-2022 Alexandros Theodotou <alex@zrythm.org>
// SPDX-License-Identifier: LicenseRef-ZrythmLicense

/**
 * \file
 *
 * Sample processor schema.
 */

#ifndef __SCHEMAS_AUDIO_SAMPLE_PROCESSOR_H__
#define __SCHEMAS_AUDIO_SAMPLE_PROCESSOR_H__

#include "utils/types.h"

#include "schemas/dsp/fader.h"

typedef struct SampleProcessor SampleProcessor;

typedef struct SampleProcessor_v1
{
  int        schema_version;
  Fader_v1 * fader;
} SampleProcessor_v1;

static const cyaml_schema_field_t sample_processor_fields_schema_v1[] = {
  YAML_FIELD_INT (SampleProcessor_v1, schema_version),
  YAML_FIELD_MAPPING_PTR (SampleProcessor_v1, fader, fader_fields_schema_v1),

  CYAML_FIELD_END
};

static const cyaml_schema_value_t sample_processor_schema_v1 = {
  YAML_VALUE_PTR (SampleProcessor_v1, sample_processor_fields_schema_v1),
};

SampleProcessor *
sample_processor_upgrade_from_v1 (SampleProcessor_v1 * old);

#endif
