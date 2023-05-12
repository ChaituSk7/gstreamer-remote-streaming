#include "header.h"
#include "msghandler.h"

int
thumbnail_pipeline (int argc, char *argv[])
{
  GstBus *bus;
  GstStateChangeReturn ret;
  Thumbnail remote_host;
  CustomData data;
  GstCaps *caps;
  GstMessage *msg;

  /* Initialize thumbnail structure */
  memset (&remote_host, 0, sizeof (remote_host));
  memset (&data, 0, sizeof (data));

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  /* Create the elements */
  remote_host.pipeline = gst_pipeline_new ("thumbnail");
  remote_host.udpsrc = gst_element_factory_make ("udpsrc", NULL);
  remote_host.watchdog = gst_element_factory_make ("watchdog", NULL);
  remote_host.caps_filter = gst_element_factory_make ("capsfilter", NULL);
  remote_host.rtp_depay = gst_element_factory_make ("rtpjpegdepay", NULL);
  remote_host.decoder = gst_element_factory_make ("jpegdec", NULL);
  remote_host.videoconvert = gst_element_factory_make ("videoconvert", NULL);
  remote_host.sink = gst_element_factory_make ("ximagesink", NULL);

  /* Set the properties of the elements */
  g_object_set (G_OBJECT (remote_host.udpsrc), "port", 5003, NULL);
  g_object_set (G_OBJECT (remote_host.watchdog), "timeout", 7000, NULL);

  /* Set the capability */
  caps = gst_caps_new_simple ("application/x-rtp",
      "encoding-name", G_TYPE_STRING, "JPEG", "payload", G_TYPE_INT, 26, NULL);
  g_object_set (G_OBJECT (remote_host.caps_filter), "caps", caps, NULL);

  /* Add the elements to the pipeline */
  gst_bin_add_many (GST_BIN (remote_host.pipeline), remote_host.udpsrc,
      remote_host.watchdog, remote_host.caps_filter, remote_host.rtp_depay,
      remote_host.decoder, remote_host.videoconvert, remote_host.sink, NULL);

  /* Link the elements together */
  if (!gst_element_link_many (remote_host.udpsrc, remote_host.watchdog,
          remote_host.caps_filter, remote_host.rtp_depay, remote_host.decoder,
          remote_host.videoconvert, remote_host.sink, NULL)) {
    g_printerr ("Failed to link elements\n");
    exit (0);
  }

  /* Start playing */
  ret = gst_element_set_state (remote_host.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Could not set the pipeline to playing state.\n");
    gst_object_unref (remote_host.pipeline);
  }

  /* Watch the pipeline adding to the bus */
  bus = gst_element_get_bus (remote_host.pipeline);
  gst_bus_add_signal_watch (bus);
  remote_host.loop = g_main_loop_new (NULL, FALSE);

  /* Initializing the message handler pipeline to the current pipeline */
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
  gst_caps_unref (caps);

  return 0;
}
