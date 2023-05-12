#include "header.h"
#include "keyboardhandler.h"

/* This function will be called by the pad-added signal */
static void
host_pad_handler (GstElement * src, GstPad * pad, HostAVIData * data)
{

  GstPad *video_sink_pad =
      gst_element_get_static_pad (data->video_queue, "sink");
  GstPad *audio_sink_pad =
      gst_element_get_static_pad (data->audio_queue, "sink");

  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  new_pad_caps = gst_pad_get_current_caps (pad);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);

  if (g_str_has_prefix (new_pad_type, "video/")) {
    ret = gst_pad_link (pad, video_sink_pad);
    if (GST_PAD_LINK_FAILED (ret)) {
      g_print ("Type is '%s' but link failed.\n", new_pad_type);
    }
  }
  if (g_str_has_prefix (new_pad_type, "audio/")) {
    ret = gst_pad_link (pad, audio_sink_pad);
    if (GST_PAD_LINK_FAILED (ret)) {
      g_print ("Type is '%s' but link failed.\n", new_pad_type);
    }
  }
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the capabilities */
  gst_object_unref (video_sink_pad);
  gst_object_unref (audio_sink_pad);
}

int
hostavi_pipeline (char *argv)
{
  GIOChannel *io_stdin;
  GstStateChangeReturn ret;
  GstBus *bus;
  HostAVIData avi;
  CustomData data;

  /* Initialize structure members with zero */
  memset (&avi, 0, sizeof (avi));
  memset (&data, 0, sizeof (data));

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  /* Create the elements */
  avi.pipeline = gst_pipeline_new ("AVI-pipeline");
  avi.source = gst_element_factory_make ("filesrc", NULL);
  avi.demux = gst_element_factory_make ("avidemux", NULL);
  avi.video_queue = gst_element_factory_make ("queue", NULL);
  avi.video_parser = gst_element_factory_make ("mpeg4videoparse", NULL);
  avi.video_decoder = gst_element_factory_make ("avdec_mpeg4", NULL);
  avi.video_convert = gst_element_factory_make ("videoconvert", NULL);
  avi.video_encoder = gst_element_factory_make ("vp8enc", NULL);
  avi.video_payload = gst_element_factory_make ("rtpvp8pay", NULL);
  avi.udp_video_sink = gst_element_factory_make ("udpsink", NULL);
  avi.audio_queue = gst_element_factory_make ("queue", NULL);
  avi.audio_parser = gst_element_factory_make ("mpegaudioparse", NULL);
  avi.audio_decoder = gst_element_factory_make ("avdec_mp3", NULL);
  avi.audio_convert = gst_element_factory_make ("audioconvert", NULL);
  avi.audio_volume = gst_element_factory_make ("volume", NULL);
  avi.audio_encoder = gst_element_factory_make ("opusenc", NULL);
  avi.audio_payload = gst_element_factory_make ("rtpopuspay", NULL);
  avi.udp_audio_sink = gst_element_factory_make ("udpsink", NULL);

  /* Checking all the elemnents are created or not */
  if (!avi.pipeline || !avi.source || !avi.demux || !avi.video_queue
      || !avi.video_parser || !avi.video_decoder || !avi.video_convert
      || !avi.video_encoder || !avi.video_payload || !avi.video_payload
      || !avi.udp_video_sink || !avi.audio_queue || !avi.audio_parser
      || !avi.audio_decoder || !avi.audio_convert || !avi.audio_volume
      || !avi.audio_encoder || !avi.audio_payload || !avi.udp_audio_sink) {
    g_printerr ("Not all elements could be created\n");
  }
  /* Add all the elements to bin */
  gst_bin_add_many (GST_BIN (avi.pipeline), avi.source, avi.demux,
      avi.video_queue, avi.video_parser, avi.video_decoder, avi.video_convert,
      avi.video_encoder, avi.video_payload, avi.udp_video_sink, avi.audio_queue,
      avi.audio_parser, avi.audio_decoder, avi.audio_convert, avi.audio_volume,
      avi.audio_encoder, avi.audio_payload, avi.udp_audio_sink, NULL);

  /* Setting the element properties */
  g_object_set (G_OBJECT (avi.source), "location", argv, NULL);
  g_object_set (G_OBJECT (avi.video_encoder), "deadline", 1, NULL);
  g_object_set (G_OBJECT (avi.udp_video_sink), "host", "10.1.137.49",
      "port", 5000, "clients", "10.1.138.194:5000, 10.1.136.123:5000", NULL);
  g_object_set (G_OBJECT (avi.udp_audio_sink), "host", "10.1.137.49",
      "port", 5001, "clients", "10.1.138.194:5001, 10.1.136.123:5001", NULL);
  gint initial_volume = 2;
  gdouble linear_val = (initial_volume - 1) / 9.0;
  g_object_set (G_OBJECT (avi.audio_volume), "volume", linear_val, NULL);

  /* link the elements */
  if (gst_element_link (avi.source, avi.demux) != TRUE) {
    g_printerr ("Source and demuxer not linked.\n");
    exit (EXIT_FAILURE);
  }

  if (gst_element_link_many (avi.video_queue, avi.video_parser,
          avi.video_decoder, avi.video_convert, avi.video_encoder,
          avi.video_payload, avi.udp_video_sink, NULL) != TRUE) {
    g_printerr ("video elements are not linked.\n");
    exit (EXIT_FAILURE);
  }

  if (gst_element_link_many (avi.audio_queue, avi.audio_parser,
          avi.audio_decoder, avi.audio_convert, avi.audio_volume,
          avi.audio_encoder, avi.audio_payload, avi.udp_audio_sink,
          NULL) != TRUE) {
    g_printerr ("Audio elements are not linked.\n");
    exit (EXIT_FAILURE);
  }

  GstPad *sinkpad_audio =
      gst_element_get_static_pad (avi.udp_audio_sink, "sink");
  GstPad *sinkpad_video =
      gst_element_get_static_pad (avi.udp_video_sink, "sink");

  /* Call pad add probe function */
  gst_pad_add_probe (sinkpad_audio, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      my_probe_callback, NULL, NULL);
  gst_pad_add_probe (sinkpad_video, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      my_probe_callback, NULL, NULL);

  /* Connect to the pad-added signal */
  g_signal_connect (avi.demux, "pad-added", G_CALLBACK (host_pad_handler),
      &avi);

  /* Start playing */
  ret = gst_element_set_state (avi.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Could not set the pipeline for playing state.\n");
    exit (EXIT_FAILURE);
  }

  /* Listen to the bus */
  bus = gst_element_get_bus (avi.pipeline);
  gst_bus_add_signal_watch (bus);

  /* Start the Main Loop event */
  avi.loop = g_main_loop_new (NULL, FALSE);

  /* Create Struct for Key Board Handler */
  data.pipeline = avi.pipeline;
  data.loop = avi.loop;
  data.path = argv;
  data.volume = avi.audio_volume;

  /* Connect signal messages that came from bus */
  g_signal_connect (bus, "message", G_CALLBACK (msg_handle), &data);

  g_print ("\n\nPress 'k' to see a list of keyboard shortcuts\n\n");

  /* Set up IO handler */
#ifdef G_OS_WIN32
  io_stdin = g_io_channel_win32_new_fd (fileno (stdin));
#else
  io_stdin = g_io_channel_unix_new (fileno (stdin));
#endif
  guint id =
      g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc) handle_keyboard, &data);

  /* Run the GMainLoop */
  g_main_loop_run (avi.loop);

  /* Unreference the resoureces */
  g_source_remove (id);
  gst_element_set_state (avi.pipeline, GST_STATE_NULL);
  gst_object_unref (avi.pipeline);
  gst_object_unref (bus);
  g_main_loop_unref (avi.loop);

  return 0;
}
