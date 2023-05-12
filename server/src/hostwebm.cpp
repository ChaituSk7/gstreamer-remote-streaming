#include "header.h"
#include "keyboardhandler.h"

/* This function will be called by the pad-added signal */
static void
host_pad_handler (GstElement * src, GstPad * pad, HostWEBMData * data)
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
hostwebm_pipeline (char *argv)
{
  GstStateChangeReturn ret;
  GstBus *bus;
  HostWEBMData webm;
  GIOChannel *io_stdin;
  CustomData data;


  /* Initialize the structure members with 0 */
  memset (&webm, 0, sizeof (webm));
  memset (&data, 0, sizeof (data));

  /* Initialze the gstreamer */
  gst_init (NULL, NULL);

  /* Initialize the elements */
  webm.pipeline = gst_pipeline_new ("WEMB-pipeline");
  webm.source = gst_element_factory_make ("filesrc", NULL);
  webm.demux = gst_element_factory_make ("matroskademux", NULL);
  webm.video_queue = gst_element_factory_make ("queue", NULL);
  webm.video_decoder = gst_element_factory_make ("vp8dec", NULL);
  webm.video_convert = gst_element_factory_make ("videoconvert", NULL);
  webm.video_encoder = gst_element_factory_make ("vp8enc", NULL);
  webm.video_payload = gst_element_factory_make ("rtpvp8pay", NULL);
  webm.udp_video_sink = gst_element_factory_make ("udpsink", NULL);
  webm.audio_queue = gst_element_factory_make ("queue", NULL);
  webm.audio_decoder = gst_element_factory_make ("vorbisdec", NULL);
  webm.audio_convert = gst_element_factory_make ("audioconvert", NULL);
  webm.audio_volume = gst_element_factory_make ("volume", NULL);
  webm.audio_encoder = gst_element_factory_make ("opusenc", NULL);
  webm.audio_payload = gst_element_factory_make ("rtpopuspay", NULL);
  webm.udp_audio_sink = gst_element_factory_make ("udpsink", NULL);

  /* Check the elements are created on not */
  if (!webm.pipeline || !webm.source || !webm.demux || !webm.video_queue
      || !webm.video_decoder || !webm.video_convert || !webm.video_encoder
      || !webm.video_payload || !webm.video_payload || !webm.udp_video_sink
      || !webm.audio_queue || !webm.audio_decoder || !webm.audio_convert
      || !webm.audio_volume || !webm.audio_encoder || !webm.audio_payload
      || !webm.udp_audio_sink) {
    g_printerr ("Not all elements could be created\n");
  }

  /* Add elements to the Bin */
  gst_bin_add_many (GST_BIN (webm.pipeline), webm.source, webm.demux,
      webm.video_queue, webm.video_decoder, webm.video_convert,
      webm.video_encoder, webm.video_payload, webm.udp_video_sink,
      webm.audio_queue, webm.audio_decoder, webm.audio_convert,
      webm.audio_volume, webm.audio_encoder, webm.audio_payload,
      webm.udp_audio_sink, NULL);

  /* Setting the element properties */
  g_object_set (G_OBJECT (webm.source), "location", argv, NULL);
  g_object_set (G_OBJECT (webm.video_encoder), "deadline", 1, NULL);
  g_object_set (G_OBJECT (webm.udp_video_sink), "host", "10.1.137.49",
      "port", 5000, "clients", "10.1.138.194:5000,10.1.137.49:5000", NULL);
  g_object_set (G_OBJECT (webm.udp_audio_sink), "host", "10.1.137.49",
      "port", 5001, "clients", "10.1.138.194:5001,10.1.137.49:5001", NULL);
  gint initial_volume = 2;
  gdouble linear_val = (initial_volume - 1) / 9.0;
  g_object_set (G_OBJECT (webm.audio_volume), "volume", linear_val, NULL);

  /* Linking the elements */
  if (gst_element_link (webm.source, webm.demux) != TRUE) {
    g_printerr ("Source and demuxer not linked.\n");
    exit (EXIT_FAILURE);
  }
  if (gst_element_link_many (webm.video_queue, webm.video_decoder,
          webm.video_convert, webm.video_encoder, webm.video_payload,
          webm.udp_video_sink, NULL) != TRUE) {
    g_printerr ("video elements are not linked.\n");
    exit (EXIT_FAILURE);
  }
  if (gst_element_link_many (webm.audio_queue, webm.audio_decoder,
          webm.audio_convert, webm.audio_volume, webm.audio_encoder,
          webm.audio_payload, webm.udp_audio_sink, NULL) != TRUE) {
    g_printerr ("Audio elements are not linked.\n");
    exit (EXIT_FAILURE);
  }

  GstPad *sinkpad_audio =
      gst_element_get_static_pad (webm.udp_audio_sink, "sink");
  GstPad *sinkpad_video =
      gst_element_get_static_pad (webm.udp_video_sink, "sink");

  /* Call pad add probe function */
  gst_pad_add_probe (sinkpad_audio, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      my_probe_callback, NULL, NULL);
  gst_pad_add_probe (sinkpad_video, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      my_probe_callback, NULL, NULL);

  /* Connect the pad-added signal */
  g_signal_connect (webm.demux, "pad-added", G_CALLBACK (host_pad_handler),
      &webm);

  /* Set the Pipeline to playing state */
  ret = gst_element_set_state (webm.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Could not set the pipeline for playing state.\n");
    exit (EXIT_FAILURE);
  }

  /* Add bus to the pipeline to listen messages */
  bus = gst_element_get_bus (webm.pipeline);
  gst_bus_add_signal_watch (bus);

  /* Start the Main Loop event */
  webm.loop = g_main_loop_new (NULL, FALSE);

  /* Create Struct for Key Board Handler */
  data.pipeline = webm.pipeline;
  data.loop = webm.loop;
  data.path = argv;
  data.volume = webm.audio_volume;

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
  g_main_loop_run (webm.loop);

  /* Unreference the resources */
  g_source_remove (id);
  gst_element_set_state (webm.pipeline, GST_STATE_NULL);
  gst_object_unref (webm.pipeline);
  gst_object_unref (bus);
  g_main_loop_unref (webm.loop);

  return 0;
}
