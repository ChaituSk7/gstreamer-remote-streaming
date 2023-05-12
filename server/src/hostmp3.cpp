#include "header.h"
#include "keyboardhandler.h"

int
hostmp3_pipeline (char *argv)
{
  GstStateChangeReturn ret;
  GstBus *bus;
  HostMP3Data mp3;
  GIOChannel *io_stdin;
  CustomData data;

  /* Initialize structure members with zero */
  memset (&data, 0, sizeof (data));

  /* Initialize Gstreamer */
  gst_init (NULL, NULL);

  /* Initialize elements */
  mp3.pipeline = gst_pipeline_new ("mp3-pipeline");
  mp3.filesrc = gst_element_factory_make ("filesrc", NULL);
  mp3.audio_parse = gst_element_factory_make ("mpegaudioparse", NULL);
  mp3.audio_decoder = gst_element_factory_make ("avdec_mp3", NULL);
  mp3.audio_queue = gst_element_factory_make ("queue", NULL);
  mp3.audio_convert = gst_element_factory_make ("audioconvert", NULL);
  mp3.audio_volume = gst_element_factory_make ("volume", NULL);
  mp3.audio_encoder = gst_element_factory_make ("lamemp3enc", NULL);
  mp3.audio_payloader = gst_element_factory_make ("rtpmpapay", NULL);
  mp3.audio_udp_sink = gst_element_factory_make ("udpsink", NULL);

  /* Check the elements are created or not */
  if (!mp3.pipeline || !mp3.filesrc || !mp3.audio_parse || !mp3.audio_decoder
      || !mp3.audio_queue || !mp3.audio_convert || !mp3.audio_volume
      || !mp3.audio_encoder || !mp3.audio_payloader || !mp3.audio_udp_sink) {
    g_printerr ("Not all the elements could be created.\n");
    exit (EXIT_FAILURE);
  }

  /* Add all the elements to the Bin */
  gst_bin_add_many (GST_BIN (mp3.pipeline), mp3.filesrc, mp3.audio_parse,
      mp3.audio_decoder, mp3.audio_queue, mp3.audio_convert, mp3.audio_volume,
      mp3.audio_encoder, mp3.audio_payloader, mp3.audio_udp_sink, NULL);

  /* Set the element properties */
  g_object_set (G_OBJECT (mp3.filesrc), "location", argv, NULL);

  g_object_set (G_OBJECT (mp3.audio_udp_sink), "host", "10.1.137.49",
      "port", 5000, "clients", "10.1.138.194:5000,10.1.137.49:5000", NULL);
  gint initial_volume = 2;
  gdouble linear_val = (initial_volume - 1) / 9.0;
  g_object_set (G_OBJECT (mp3.audio_volume), "volume", linear_val, NULL);

  /* Link the elements */
  if (gst_element_link_many (mp3.filesrc, mp3.audio_parse, mp3.audio_decoder,
          mp3.audio_queue, mp3.audio_convert, mp3.audio_volume,
          mp3.audio_encoder, mp3.audio_payloader, mp3.audio_udp_sink,
          NULL) != TRUE) {
    g_printerr ("Elements are not linked.\n");
    exit (EXIT_FAILURE);
  }

  GstPad *sinkpad_audio =
      gst_element_get_static_pad (mp3.audio_udp_sink, "sink");

  /* Call pad add probe function */
  gst_pad_add_probe (sinkpad_audio, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      my_probe_callback, NULL, NULL);

  /* Set the pipeline for playing State */
  ret = gst_element_set_state (mp3.pipeline, GST_STATE_PLAYING);
  if (ret = GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Could not set the pipeline to playing.\n");
    exit (EXIT_FAILURE);
  }

  /* Listen to the bus */
  bus = gst_element_get_bus (mp3.pipeline);
  gst_bus_add_signal_watch (bus);

  /* Start the Main Loop event */
  mp3.loop = g_main_loop_new (NULL, FALSE);

  /* Create Struct for Key Board Handler */
  data.pipeline = mp3.pipeline;
  data.loop = mp3.loop;
  data.path = argv;
  data.volume = mp3.audio_volume;

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
  g_main_loop_run (mp3.loop);

  /* Unreference the resoureces */
  g_source_remove (id);
  gst_element_set_state (mp3.pipeline, GST_STATE_NULL);
  gst_object_unref (mp3.pipeline);
  gst_object_unref (bus);
  g_main_loop_unref (mp3.loop);

  return 0;
}
