// SPDX-FileCopyrightText: © 2020-2022 Alexandros Theodotou <alex@zrythm.org>
// SPDX-License-Identifier: LicenseRef-ZrythmLicense

#include "zrythm-test-config.h"

#include "audio/tempo_track.h"
#include "audio/track.h"
#include "project.h"
#include "utils/dsp.h"
#include "utils/flags.h"
#include "zrythm.h"

#include <glib.h>

#include "helpers/plugin_manager.h"
#include "helpers/project.h"
#include "helpers/zrythm.h"

#include <locale.h>

static void
test_empty_save_load (void)
{
  test_helper_zrythm_init ();

  int ret;
  g_assert_nonnull (PROJECT);

  /* save and reload the project */
  test_project_save_and_reload ();

  /* resave it */
  ret = project_save (
    PROJECT, PROJECT->dir, F_NOT_BACKUP, 0, F_NO_ASYNC);
  g_assert_cmpint (ret, ==, 0);

  test_helper_zrythm_cleanup ();
}

static void
test_save_load_with_data (void)
{
  test_helper_zrythm_init ();

  int ret;
  g_assert_nonnull (PROJECT);

  /* add some data */
  Position p1, p2;
  test_project_rebootstrap_timeline (&p1, &p2);

  /* save the project */
  ret = project_save (PROJECT, PROJECT->dir, 0, 0, F_NO_ASYNC);
  g_assert_cmpint (ret, ==, 0);
  char * prj_file =
    g_build_filename (PROJECT->dir, PROJECT_FILE, NULL);

  /* stop the engine */
  EngineState state;
  engine_wait_for_pause (PROJECT->audio_engine, &state, true);

  /* remove objects */
  chord_track_clear (P_CHORD_TRACK);
  marker_track_clear (P_MARKER_TRACK);
  tempo_track_clear (P_TEMPO_TRACK);
  for (int i = TRACKLIST->num_tracks - 1;
       i > P_MASTER_TRACK->pos; i--)
    {
      Track * track = TRACKLIST->tracks[i];
      tracklist_remove_track (TRACKLIST, track, 1, 1, 0, 0);
    }
  track_clear (P_MASTER_TRACK);

  /* reload the project */
  ret = project_load (prj_file, 0);
  g_assert_cmpint (ret, ==, 0);

  /* stop the engine */
  engine_resume (PROJECT->audio_engine, &state);

  /* verify that the data is correct */
  test_project_check_vs_original_state (&p1, &p2, 0);

  test_helper_zrythm_cleanup ();
}

static void
test_new_from_template (void)
{
  test_helper_zrythm_init ();

  /* add plugins */
  test_plugin_manager_create_tracks_from_plugin (
    TRIPLE_SYNTH_BUNDLE, TRIPLE_SYNTH_URI, true, false, 1);
#ifdef HAVE_CARLA
  test_plugin_manager_create_tracks_from_plugin (
    TRIPLE_SYNTH_BUNDLE, TRIPLE_SYNTH_URI, true, true, 1);
#endif

  /* create region and ensure sound */
  Track * track = tracklist_get_last_track (
    TRACKLIST, TRACKLIST_PIN_OPTION_BOTH, false);
  Position start, end;
  position_init (&start);
  position_set_to_bar (&end, 3);
  ZRegion * r = midi_region_new (
    &start, &end, track_get_name_hash (track), 0, 0);
  track_add_region (
    track, r, NULL, 0, F_GEN_NAME, F_NO_PUBLISH_EVENTS);
  arranger_object_select (
    (ArrangerObject *) r, F_SELECT, F_NO_APPEND,
    F_NO_PUBLISH_EVENTS);
  arranger_selections_action_perform_create (
    TL_SELECTIONS, NULL);
  MidiNote * mn = midi_note_new (&r->id, &start, &end, 45, 45);
  midi_region_add_midi_note (r, mn, F_NO_PUBLISH_EVENTS);
  arranger_object_select (
    (ArrangerObject *) mn, F_SELECT, F_NO_APPEND,
    F_NO_PUBLISH_EVENTS);
  arranger_selections_action_perform_create (
    (ArrangerSelections *) MA_SELECTIONS, NULL);

  /* stop dummy audio engine processing so we can process
   * manually */
  test_project_stop_dummy_engine ();

  /* start playback and process for a couple of cycles */
  transport_set_playhead_to_bar (TRANSPORT, 1);
  transport_request_roll (TRANSPORT, true);
  engine_process (AUDIO_ENGINE, AUDIO_ENGINE->block_length);
  engine_process (AUDIO_ENGINE, AUDIO_ENGINE->block_length);

  g_assert_true (dsp_abs_max (
    &MONITOR_FADER->stereo_out->l->buf[0],
    AUDIO_ENGINE->block_length) > 0.0001f);

  test_project_save_and_reload ();

  /* stop dummy audio engine processing so we can process
   * manually */
  test_project_stop_dummy_engine ();

  /* start playback and process for a couple of cycles */
  transport_set_playhead_to_bar (TRANSPORT, 1);
  transport_request_roll (TRANSPORT, true);
  engine_process (AUDIO_ENGINE, AUDIO_ENGINE->block_length);
  engine_process (AUDIO_ENGINE, AUDIO_ENGINE->block_length);

  g_assert_true (dsp_abs_max (
    &MONITOR_FADER->stereo_out->l->buf[0],
    AUDIO_ENGINE->block_length) > 0.0001f);

  /* create a new project using old one as template */
  char * orig_dir = g_strdup (PROJECT->dir);
  g_assert_nonnull (orig_dir);
  char * filepath =
    g_build_filename (orig_dir, "project.zpj", NULL);
  g_free_and_null (ZRYTHM->create_project_path);
  ZRYTHM->create_project_path =
    g_dir_make_tmp ("zrythm_test_project_XXXXXX", NULL);
  project_load (filepath, true);
  io_rmdir (orig_dir, true);
  g_free_and_null (orig_dir);
  g_free_and_null (filepath);

  /* stop dummy audio engine processing so we can process
   * manually */
  test_project_stop_dummy_engine ();

  /* start playback and process for a couple of cycles */
  transport_set_playhead_to_bar (TRANSPORT, 1);
  transport_request_roll (TRANSPORT, true);
  engine_process (AUDIO_ENGINE, AUDIO_ENGINE->block_length);
  engine_process (AUDIO_ENGINE, AUDIO_ENGINE->block_length);

  g_assert_true (dsp_abs_max (
    &MONITOR_FADER->stereo_out->l->buf[0],
    AUDIO_ENGINE->block_length) > 0.0001f);

  test_helper_zrythm_cleanup ();
}

static void
test_save_as_load_w_pool (void)
{
  test_helper_zrythm_init ();

  Position p1, p2;
  test_project_rebootstrap_timeline (&p1, &p2);

  /* save the project elsewhere */
  char * orig_dir = g_strdup (PROJECT->dir);
  g_assert_nonnull (orig_dir);
  char * new_dir =
    g_dir_make_tmp ("zrythm_test_project_XXXXXX", NULL);
  int ret =
    project_save (PROJECT, new_dir, false, false, F_NO_ASYNC);
  g_assert_cmpint (ret, ==, 0);

  /* free the project */
  object_free_w_func_and_null (project_free, PROJECT);

  /* load the new one */
  char * filepath =
    g_build_filename (new_dir, "project.zpj", NULL);
  ret = project_load (filepath, 0);
  g_assert_cmpint (ret, ==, 0);

  io_rmdir (orig_dir, true);

  test_helper_zrythm_cleanup ();
}

static void
test_save_backup_w_pool_and_plugins (void)
{
  test_helper_zrythm_init ();

  Position p1, p2;
  test_project_rebootstrap_timeline (&p1, &p2);

  /* add a plugin and create a duplicate track */
  int track_pos = test_plugin_manager_create_tracks_from_plugin (
    TEST_INSTRUMENT_BUNDLE_URI, TEST_INSTRUMENT_URI, true,
    true, 1);
  Track * track = TRACKLIST->tracks[track_pos];
  track_select (
    track, F_SELECT, F_EXCLUSIVE, F_NO_PUBLISH_EVENTS);
  tracklist_selections_action_perform_copy (
    TRACKLIST_SELECTIONS, PORT_CONNECTIONS_MGR,
    TRACKLIST->num_tracks, NULL);

  char * dir = g_strdup (PROJECT->dir);

  /* save a project backup */
  int ret = project_save (
    PROJECT, PROJECT->dir, F_BACKUP, false, F_NO_ASYNC);
  g_assert_cmpint (ret, ==, 0);
  g_assert_nonnull (PROJECT->backup_dir);
  char * backup_dir = g_strdup (PROJECT->backup_dir);

  /* free the project */
  object_free_w_func_and_null (project_free, PROJECT);

  /* load the backup directly */
  char * filepath =
    g_build_filename (backup_dir, PROJECT_FILE, NULL);
  ret = project_load (filepath, false);
  g_free (filepath);
  g_assert_cmpint (ret, ==, 0);

  /* test undo and redo cloning the track */
  undo_manager_undo (UNDO_MANAGER, NULL);
  undo_manager_redo (UNDO_MANAGER, NULL);

  /* free the project */
  object_free_w_func_and_null (project_free, PROJECT);

  /* attempt to open the latest backup (mimic
   * behavior from UI) */
  ZRYTHM->open_newer_backup = true;
  filepath = g_build_filename (dir, "project.zpj", NULL);
  ret = project_load (filepath, false);
  g_free (filepath);
  g_assert_cmpint (ret, ==, 0);

  g_free (backup_dir);
  g_free (dir);

  test_helper_zrythm_cleanup ();
}

static void
test_load_v1_0_0_beta_2_1_1 (void)
{
  test_helper_zrythm_init ();

  /* TODO test opening a project from 1.0.0 beta 2.1.1 that
   * has undo history */

  /*yaml_set_log_level (CYAML_LOG_DEBUG);*/
  /*project_load ("/home/alex/.var/app/org.zrythm.Zrythm/data/zrythm/projects/プロジェクト名未設定 (3)/project.zpj", false);*/

  test_helper_zrythm_cleanup ();
}

int
main (int argc, char * argv[])
{
  g_test_init (&argc, &argv, NULL);

#define TEST_PREFIX "/project/"

  g_test_add_func (
    TEST_PREFIX "test new from template",
    (GTestFunc) test_new_from_template);
  g_test_add_func (
    TEST_PREFIX "test load v1.0.0-beta.2.1.1",
    (GTestFunc) test_load_v1_0_0_beta_2_1_1);
  g_test_add_func (
    TEST_PREFIX "test save backup w pool and plugins",
    (GTestFunc) test_save_backup_w_pool_and_plugins);
  g_test_add_func (
    TEST_PREFIX "test empty save load",
    (GTestFunc) test_empty_save_load);
  g_test_add_func (
    TEST_PREFIX "test save load with data",
    (GTestFunc) test_save_load_with_data);
  g_test_add_func (
    TEST_PREFIX "test save as load w pool",
    (GTestFunc) test_save_as_load_w_pool);

  return g_test_run ();
}
