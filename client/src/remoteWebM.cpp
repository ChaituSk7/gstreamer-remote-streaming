#include "header.h"
#include "probe.h"
#include "msghandler.h"

int
remotehost_WebM_pipeline (int argc, char *argv[])
{
  GstBus *bus;
  GstStateChangeReturn ret;
  RemoteWebM remote_host;
  GstCaps *audio_caps = NULL;
  GstCaps *video_caps = NULL;
  CustomData data;

  /* Initialize RemoteHost structure */
  memset (&remote_host, 0, sizeof (remote_host));
  memset (&data, 0, sizeof (data));


  /* Initialize gstreamer */
  gst_init (NULL, NULL);

  /* Initialize gstremer elements */
  remote_host.pipeline = gst_pipeline_new ("Remote-host-Avi");
  remote_host.udp_video_source = gst_element_factory_make ("udpsrc", NULL);
  remote_host.video_watchdog = gst_element_factory_make ("watchdog", NULL);
  remote_host.video_rtp_buffer =
      gst_element_factory_make ("rtpjitterbuffer", NULL);
  remote_host.video_rtp_depay = gst_element_factory_make ("rtpvp8depay", NULL);
  remote_host.video_queue = gst_element_factory_make ("queue", NULL);
  remote_host.video_decoder = gst_element_factory_make ("vp8dec", NULL);
  remote_host.video_convert = gst_element_factory_make ("videoconvert", NULL);
  remote_host.video_sink = gst_element_factory_make ("autovideosink", "vsink");

  remote_host.udp_audio_source = gst_element_factory_make ("udpsrc", NULL);
  remote_host.audio_watchdog = gst_element_factory_make ("watchdog", NULL);
  remote_host.audio_rtp_buffer =
      gst_element_factory_make ("rtpjitterbuffer", NULL);
  remote_host.audio_rtp_depay = gst_element_factory_make ("rtpopusdepay", NULL);
  remote_host.audio_decoder = gst_element_factory_make ("opusdec", NULL);
  remote_host.audio_queue = gst_element_factory_make ("queue", NULL);
  remote_host.audio_convert = gst_element_factory_make ("audioconvert", NULL);
  remote_host.audio_sink = gst_element_factory_make ("autoaudiosink", "asink");

  /* Check if the elements are created */
  if (!remote_host.pipeline || !remote_host.udp_video_source
      || !remote_host.video_rtp_depay || !remote_host.video_rtp_buffer
      || !remote_host.video_watchdog || !remote_host.video_decoder
      || !remote_host.video_queue || !remote_host.video_convert
      || !remote_host.video_sink || !remote_host.audio_watchdog
      || !remote_host.udp_audio_source || !remote_host.audio_rtp_buffer
      || !remote_host.audio_rtp_depay || !remote_host.audio_decoder
      || !remote_host.audio_queue || !remote_host.audio_convert
      || !remote_host.audio_sink) {
    g_printerr ("Not all video elements could be created.\n");
    exit (EXIT_FAILURE);
  }

  /* Add elements to bin */
  gst_bin_add_many (GST_BIN (remote_host.pipeline),
      remote_host.udp_video_source, remote_host.video_rtp_buffer,
      remote_host.video_rtp_depay, remote_host.video_watchdog,
      remote_host.video_decoder, remote_host.video_queue,
      remote_host.video_convert, remote_host.video_sink,
      remote_host.audio_watchdog, remote_host.udp_audio_source,
      remote_host.audio_rtp_buffer, remote_host.audio_rtp_depay,
      remote_host.audio_decoder, remote_host.audio_queue,
      remote_host.audio_convert, remote_host.audio_sink, NULL);

  /* Set the video Capability */
  video_caps = gst_caps_new_simple ("application/x-rtp",
      "media", G_TYPE_STRING, "video",
      "encoding-name", G_TYPE_STRING, "VP8",
      "clock-rate", G_TYPE_INT, 90000, NULL);

  /* Set the video element properties */
  g_object_set (G_OBJECT (remote_host.udp_video_source), "caps", video_caps,
      "port", 5000, NULL);
  /* Set the video watchdog property */
  g_object_set (G_OBJECT (remote_host.video_watchdog), "timeout", 10000, NULL);

  /* Set the audio Capability */
  audio_caps = gst_caps_new_simple ("application/x-rtp",
      "media", G_TYPE_STRING, "audio",
      "encoding-name", G_TYPE_STRING, "OPUS", "payload", G_TYPE_INT, 96, NULL);

  /* Set the audio element properties */
  g_object_set (G_OBJECT (remote_host.udp_audio_source), "caps", audio_caps,
      "port", 5001, NULL);
  /* Set the audio watchdog property */
  g_object_set (G_OBJECT (remote_host.audio_watchdog), "timeout", 10000, NULL);


  /* Link the video elements */
  if (gst_element_link_many (remote_host.udp_video_source,
          remote_host.video_watchdog, remote_host.video_rtp_buffer,
          remote_host.video_rtp_depay, remote_host.video_queue,
          remote_host.video_decoder, remote_host.video_convert,
          remote_host.video_sink, NULL) != TRUE) {
    g_printerr ("Video elements not linked.\n");
    exit (EXIT_FAILURE);
  }


  /* Link the audio elements */
  if (gst_element_link_many (remote_host.udp_audio_source,
          remote_host.audio_watchdog, remote_host.audio_rtp_buffer,
          remote_host.audio_rtp_depay, remote_host.audio_queue,
          remote_host.audio_decoder, remote_host.audio_convert,
          remote_host.audio_sink, NULL) != TRUE) {
    g_printerr ("Audio elements not linked.\n");
    exit (EXIT_FAILURE);
  }

  /* Probe for audio */
  GstElement *sink_element =
      gst_bin_get_by_name (GST_BIN (remote_host.pipeline), "asink");
  GstPad *sinkpad = gst_element_get_static_pad (sink_element, "sink");
  gst_pad_add_probe (sinkpad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      probe_callback, NULL, NULL);

  /* Probe for video */
  sink_element = gst_bin_get_by_name (GST_BIN (remote_host.pipeline), "vsink");
  sinkpad = gst_element_get_static_pad (sink_element, "sink");
  gst_pad_add_probe (sinkpad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
      probe_callback, NULL, NULL);

  /* Set the pipeline to playing state */
  ret = gst_element_set_state (remote_host.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Could not set the pipeline to playing state.\n");
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

  /* Unrefrence the pipeline */
  gst_element_set_state (remote_host.pipeline, GST_STATE_NULL);
  gst_object_unref (remote_host.pipeline);
  gst_object_unref (bus);
  g_main_loop_unref (remote_host.loop);

  /* Unref caps */
  gst_caps_unref (video_caps);
  gst_caps_unref (audio_caps);

  return 0;
}
