#include "header.h"
#include "keyboardhandler.h"

/* This function will be called by the pad-added signal */
static void
host_pad_handler (GstElement * src, GstPad * pad, HostMP4Data * data)
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

  gst_object_unref (video_sink_pad);
  gst_object_unref (audio_sink_pad);
}

int
hostmp4_pipeline (char *argv)
{
  GIOChannel *io_stdin;
  GstBus *bus;
  GstStateChangeReturn ret;
  HostMP4Data server_data;
  CustomData data;

  /* Initializing structure varilable to zero */
  memset (&server_data, 0, sizeof (server_data));
  memset (&data, 0, sizeof (data));

  /* Initialize gstreamer */
  gst_init (NULL, NULL);

  /* Initilize elements */
  server_data.pipeline = gst_pipeline_new ("host-pipeline");
  server_data.source = gst_element_factory_make ("filesrc", NULL);
  server_data.demuxer = gst_element_factory_make ("qtdemux", NULL);
  server_data.video_decoder = gst_element_factory_make ("avdec_h264", NULL);
  server_data.video_queue = gst_element_factory_make ("queue", NULL);
  server_data.video_convert = gst_element_factory_make ("videoconvert", NULL);
  server_data.video_encoder = gst_element_factory_make ("x264enc", NULL);
  server_data.rtp_payload = gst_element_factory_make ("rtph264pay", NULL);
  server_data.udp_sink_video = gst_element_factory_make ("udpsink", NULL);
  server_data.audio_decoder = gst_element_factory_make ("faad", NULL);
  server_data.audio_queue = gst_element_factory_make ("queue", NULL);
  server_data.audio_convert = gst_element_factory_make ("audioconvert", NULL);
  server_data.audio_resample = gst_element_factory_make ("audioresample", NULL);
  server_data.audio_volume = gst_element_factory_make ("volume", NULL);
  server_data.audio_encoder = gst_element_factory_make ("opusenc", NULL);
  server_data.rtp_audio_payload = gst_element_factory_make ("rtpopuspay", NULL);
  server_data.udp_sink_audio = gst_element_factory_make ("udpsink", NULL);

  /* Check the video elements are created or not */
  if (!server_data.pipeline || !server_data.source || !server_data.demuxer ||
      !server_data.video_decoder || !server_data.video_queue
      || !server_data.video_convert || !server_data.video_encoder
      || !server_data.rtp_payload || !server_data.udp_sink_video) {
    g_printerr ("Not all the elements could be created.\n");
    exit (EXIT_FAILURE);
  }

  /* Check the audio elements are created or not */
  if (!server_data.audio_decoder || !server_data.audio_queue
      || !server_data.audio_convert || !server_data.audio_resample
      || !server_data.audio_volume || !server_data.audio_encoder
      || !server_data.rtp_audio_payload || !server_data.udp_sink_audio) {
    g_printerr ("Not all audio elements could be created.\n");
    exit (EXIT_FAILURE);
  }
  /* Add elements to bin */
  gst_bin_add_many (GST_BIN (server_data.pipeline), server_data.source,
      server_data.demuxer, server_data.video_decoder, server_data.video_queue,
      server_data.video_convert, server_data.video_encoder,
      server_data.rtp_payload, server_data.udp_sink_video,
      server_data.audio_decoder, server_data.audio_queue,
      server_data.audio_convert, server_data.audio_resample,
      server_data.audio_volume, server_data.audio_encoder,
      server_data.rtp_audio_payload, server_data.udp_sink_audio, NULL);

  /* Set the element properties */
  g_object_set (G_OBJECT (server_data.source), "location", argv, NULL);

  g_object_set (G_OBJECT (server_data.video_encoder), "speed-preset", 1, NULL);
  g_object_set (G_OBJECT (server_data.udp_sink_video), "host", "10.1.137.49",
      NULL);
  g_object_set (G_OBJECT (server_data.udp_sink_video), "port", 5000, NULL);
  g_object_set (G_OBJECT (server_data.udp_sink_video), "clients",
      "10.1.138.194:5000,10.1.137.49:5000", NULL);

  g_object_set (G_OBJECT (server_data.udp_sink_audio), "host", "10.1.137.49",
      NULL);
  g_object_set (G_OBJECT (server_data.udp_sink_audio), "port", 5001, NULL);
  g_object_set (G_OBJECT (server_data.udp_sink_audio), "clients",
      "10.1.138.194:5001,10.1.137.49:5001", NULL);

  gint initial_volume = 2;
  gdouble linear_val = (initial_volume - 1) / 9.0;
  g_object_set (G_OBJECT (server_data.audio_volume), "volume", linear_val,
      NULL);

  /* Link the elements */
  if (gst_element_link (server_data.source, server_data.demuxer) != TRUE) {
    g_printerr ("Sorce to qtdemux not linked.\n");
    exit (EXIT_FAILURE);
  }

  if (gst_element_link_many (server_data.video_queue, server_data.video_decoder,
          server_data.video_convert, server_data.video_encoder,
          server_data.rtp_payload, server_data.udp_sink_video, NULL) != TRUE) {
    g_printerr ("Decoder to video udpsink not linked.\n");
    exit (EXIT_FAILURE);
  }

  if (gst_element_link_many (server_data.audio_queue, server_data.audio_decoder,
          server_data.audio_convert, server_data.audio_resample,
          server_data.audio_volume, server_data.audio_encoder,
          server_data.rtp_audio_payload, server_data.udp_sink_audio,
          NULL) != TRUE) {
    g_printerr ("Decoder to audio udpsink not linked.\n");
    exit (EXIT_FAILURE);
  }

  GstPad *sinkpad_audio =
      gst_element_get_static_pad (server_data.udp_sink_audio, "sink");
  GstPad *sinkpad_video =
      gst_element_get_static_pad (server_data.udp_sink_video, "sink");

  /* Call pad add probe function */
  gst_pad_add_probe (sinkpad_audio, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      my_probe_callback, NULL, NULL);
  gst_pad_add_probe (sinkpad_video, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      my_probe_callback, NULL, NULL);


  /* Connect pad-added signal */
  g_signal_connect (server_data.demuxer, "pad-added",
      G_CALLBACK (host_pad_handler), &server_data);

  /* Set the pipeline for playing state */
  ret = gst_element_set_state (server_data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Could not set the pipeline for playing.\n");
    exit (EXIT_FAILURE);
  }

  /* Add bus to the pipeline to listen the messages */
  bus = gst_element_get_bus (server_data.pipeline);
  gst_bus_add_signal_watch (bus);

  /* Start the Main Loop event */
  server_data.loop = g_main_loop_new (NULL, FALSE);

  /* Create Struct for Key Board Handler */
  data.pipeline = server_data.pipeline;
  data.loop = server_data.loop;
  data.path = argv;
  data.volume = server_data.audio_volume;

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

  /* Run the GMailLoop */
  g_main_loop_run (server_data.loop);

  /* Unreference the resoureces */
  g_source_remove (id);
  gst_element_set_state (server_data.pipeline, GST_STATE_NULL);
  gst_object_unref (server_data.pipeline);
  gst_object_unref (bus);
  g_main_loop_unref (server_data.loop);

  return 0;
}
