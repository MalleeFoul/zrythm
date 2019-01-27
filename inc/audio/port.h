/*
 * audio/ports.h - ports to pass when processing the audio
 *   signal
 *
 * copyright (c) 2019 alexandros theodotou
 *
 * this file is part of zrythm
 *
 * zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the gnu general public license as published by
 * the free software foundation, either version 3 of the license, or
 * (at your option) any later version.
 *
 * zrythm is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 *
 * you should have received a copy of the gnu general public license
 * along with zrythm.  if not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __AUDIO_PORTS_H__
#define __AUDIO_PORTS_H__

/** \file
 * Port API. */

#include "audio/midi.h"

#include <jack/jack.h>

#define MAX_DESTINATIONS 23
#define FOREACH_SRCS(port) \
  for (int i = 0; i < port->num_srcs; i++)
#define FOREACH_DESTS(port) \
  for (int i = 0; i < port->num_dests; i++)

/**
 * Special ID for owner_pl, owner_ch, etc. to indicate that
 * the port is not owned.
 */
#define PORT_NOT_OWNED -1

typedef struct Plugin Plugin;

typedef enum PortFlow {
	FLOW_UNKNOWN,
	FLOW_INPUT,
	FLOW_OUTPUT
} PortFlow;

typedef enum PortType {
	TYPE_UNKNOWN,
	TYPE_CONTROL,
	TYPE_AUDIO,
	TYPE_EVENT,
	TYPE_CV
} PortType;

/**
 * What the internal data is
 */
typedef enum PortInternalType
{
  INTERNAL_LV2_PORT,                ///< LV2_Port (see lv2_plugin.c)
  INTERNAL_JACK_PORT                ///< jack_port_t
} PortInternalType;

typedef jack_default_audio_sample_t   sample_t;
typedef jack_nframes_t                nframes_t;

typedef struct LV2_Port LV2_Port;
typedef struct Channel Channel;
typedef enum PanAlgorithm PanAlgorithm;
typedef enum PanLaw PanLaw;

/**
 * Must ONLY be created via port_new()
 */
typedef struct Port
{
  /**
   * Unique ID.
   */
  int                 id;

  char *              label; ///< human readable label

  /**
   * Buffer to be reallocated every time the buffer size
   * changes.
   *
   * The buffer size is AUDIO_ENGINE->block_length.
   */
  float *             buf;

  /**
   * Contains raw MIDI data (MIDI ports only)
   */
  MidiEvents          midi_events;

	PortType            type; ///< Data type
	PortFlow            flow; ///< Data flow direction

  /**
   * Inputs and Outputs.
   */
  int                 src_ids[MAX_DESTINATIONS];
  int                 dest_ids[MAX_DESTINATIONS];
  int                 num_srcs;
  int                 num_dests;

  /**
   * Cache.
   */
  struct Port *       srcs[MAX_DESTINATIONS];
  struct Port *       dests[MAX_DESTINATIONS];

  /**
   * Indicates whether data or lv2_port should be used.
   */
  PortInternalType    internal_type;
  LV2_Port *          lv2_port; ///< used for LV2

  /**
   * Pointer to arbitrary data.
   *
   * Use internal_type to check what data it is.
   */
  void *              data;

  /* ====== flags to indicate port owner ====== */
  int                 owner_jack; ///< 1 if owner is JACK
  int                 is_piano_roll; ///< 1 if piano roll

  /**
   * ID of owner plugin, if any.
   *
   * If the port is not owned by a plugin this will be
   * PORT_NOT_OWNED.
   */
  int                 owner_pl_id;

  /**
   * ID of owner channel, if any.
   *
   * If the port is not owned by a channel this will be
   * PORT_NOT_OWNED.
   */
  int                 owner_ch_id;

  /* ====== cache ====== */
  Plugin              * owner_pl;
  Channel             * owner_ch;

  /**
   * FIXME ???
   * used in xml project file export
   */
  int                 exported;
} Port;

/**
 * L & R port, for convenience.
 *
 * Must ONLY be created via stereo_ports_new()
 */
typedef struct StereoPorts
{
  int        l_id;
  int        r_id;

  /**
   * Cache.
   */
  Port       * l;
  Port       * r;
} StereoPorts;

/**
 * Creates port (used when loading projects).
 */
Port *
port_get_or_create_blank (int id);

/**
 * Creates port.
 */
Port *
port_new (char * label);

/**
 * Creates port.
 */
Port *
port_new_with_type (PortType     type,
                    PortFlow     flow,
                    char         * label);

/**
 * Creates port and adds given data to it
 */
Port *
port_new_with_data (PortInternalType internal_type, ///< the internal data format
                    PortType     type,
                    PortFlow     flow,
                    char         * label,
                    void         * data);   ///< the data

/**
 * Creates stereo ports.
 */
StereoPorts *
stereo_ports_new (Port * l, Port * r);

/**
 * Deletes port, doing required cleanup and updating counters.
 */
void
port_free (Port * port);

/**
 * Connets src to dest.
 */
int
port_connect (Port * src, Port * dest);

/**
 * Disconnects src from dest.
 */
int
port_disconnect (Port * src, Port * dest);

/**
 * Apply given fader value to port.
 */
void
port_apply_fader (Port * port, float amp);

/**
 * First sets port buf to 0, then sums the given port signal from its inputs.
 */
void
port_sum_signal_from_inputs (Port * port);

/**
 * if port buffer size changed, reallocate port buffer, otherwise memset to 0.
 */
//void
//port_init_buf (Port *port, nframes_t nframes);
//

int
ports_connected (Port * src, Port * dest);

/**
 * Prints all connections.
 */
void
port_print_connections_all ();

/**
 * Clears the port buffer.
 */
void
port_clear_buffer (Port * port);

/**
 * Disconnects all srcs and dests from port.
 */
int
port_disconnect_all (Port * port);

/**
 * Applies the pan to the given L/R ports.
 */
void
port_apply_pan_stereo (Port *       l,
                       Port *       r,
                       float        pan,
                       PanLaw       pan_law,
                       PanAlgorithm pan_algo);

#endif
