#include "clientheader.h"
#include "probe.h"
#include "msghandler.h"

int
remotehost_Mp3_pipeline (int argc, char *argv[])
{
  GstBus *bus;
  GstStateChangeReturn ret;
  RemoteMp3 remote_host;
  GstCaps *audio_caps = NULL;
  CustomData data;

  /* Initialize RemoteHost structure */
  memset (&remote_host, 0, sizeof (remote_host));
  memset (&data, 0, sizeof (data));

  /* Initialize gstreamer */
  gst_init (NULL, NULL);

  /* Initialize gstremer elements */
  remote_host.pipeline = gst_pipeline_new ("Remote-host-mp3");
  remote_host.udp_source = gst_element_factory_make ("udpsrc", NULL);
  remote_host.watchdog = gst_element_factory_make ("watchdog", NULL);
  remote_host.rtp_depay = gst_element_factory_make ("rtpmpadepay", NULL);
  remote_host.parser = gst_element_factory_make ("mpegaudioparse", NULL);
  remote_host.audio_decoder = gst_element_factory_make ("avdec_mp3", NULL);
  remote_host.queue = gst_element_factory_make ("queue", NULL);
  remote_host.audio_convert = gst_element_factory_make ("audioconvert", NULL);
  remote_host.audio_sink = gst_element_factory_make ("autoaudiosink", "asink");

  /* Check if the elements are created */
  if (!remote_host.pipeline || !remote_host.udp_source || !remote_host.watchdog
      || !remote_host.rtp_depay || !remote_host.parser
      || !remote_host.audio_decoder || !remote_host.queue
      || !remote_host.audio_convert || !remote_host.audio_sink) {
    g_printerr ("Not all video elements could be created.\n");
    exit (EXIT_FAILURE);
  }

  /* Add elements to bin */
  gst_bin_add_many (GST_BIN (remote_host.pipeline), remote_host.udp_source,
      remote_host.rtp_depay, remote_host.parser, remote_host.watchdog,
      remote_host.queue, remote_host.audio_decoder,
      remote_host.audio_convert, remote_host.audio_sink, NULL);

  /* Set the Capability */
  audio_caps = gst_caps_new_simple ("application/x-rtp",
      "media", G_TYPE_STRING, "audio",
      "encoding-name", G_TYPE_STRING, "MPA",
      "payload", G_TYPE_INT, 96, "clock-rate", G_TYPE_INT, 9000, NULL);

  /* Set the element properties */
  g_object_set (G_OBJECT (remote_host.udp_source), "caps", audio_caps,
      "port", 5000, NULL);

  /* Set the watchdog property */
  g_object_set (G_OBJECT (remote_host.watchdog), "timeout", 7000, NULL);

  /* Link the elements */
  if (gst_element_link_many (remote_host.udp_source, remote_host.watchdog,
          remote_host.rtp_depay, remote_host.parser, remote_host.audio_decoder,
          remote_host.queue, remote_host.audio_convert, remote_host.audio_sink,
          NULL) != TRUE) {
    g_printerr ("Audio elements not linked.\n");
    exit (EXIT_FAILURE);
  }

  /* Probe for audio */
  GstElement *sink_element =
      gst_bin_get_by_name (GST_BIN (remote_host.pipeline), "asink");
  GstPad *sinkpad = gst_element_get_static_pad (sink_element, "sink");
  gst_pad_add_probe (sinkpad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      probe_callback, NULL, NULL);

  /* Set the pipeline to playing state */
  ret = gst_element_set_state (remote_host.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Colud not set the pipeline to playing state.\n");
    gst_object_unref (remote_host.pipeline);
  }

  /* Watch the pipeline adding to the bus */
  bus = gst_element_get_bus (remote_host.pipeline);
  gst_bus_add_signal_watch (bus);
  remote_host.loop = g_main_loop_new (NULL, FALSE);

  /* Assigning current pipeline to message handler pipeline for handling message */
  data.pipeline = remote_host.pipeline;
  data.loop = remote_host.loop;

  /* Connect signal messages that came from bus */
  g_signal_connect (bus, "message", G_CALLBACK (callback_message), &data);

  /* Start the Main event loop */
  g_main_loop_run (remote_host.loop);

  /* Unreference the pipeline */
  gst_element_set_state (remote_host.pipeline, GST_STATE_NULL);
  gst_object_unref (remote_host.pipeline);
  gst_object_unref (bus);
  g_main_loop_unref (remote_host.loop);

  /* Unref caps */
  gst_caps_unref (audio_caps);

  return 0;
}
