#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "clientheader.h"

/* Creating CustomData structure for holding pipeline and loop. */
typedef struct _CustomData{
    GstElement *pipeline;
    GMainLoop *loop;
}CustomData;

/* This function handles messages from pipeline and
  takes appropriate actions based on the message type. */
static void callback_message (GstBus *bus, GstMessage *msg, CustomData *data) {

  /* Handling error message */
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error (msg, &err, &debug);
        g_print ("\nError: %s\n", err->message);
        g_error_free (err);
        g_free (debug);
        gst_element_set_state(data->pipeline, GST_STATE_NULL);
        g_main_loop_quit (data->loop);
    }
    break;
  /* Handling warning message */
    case GST_MESSAGE_WARNING:{
			GError *error = NULL;
			gchar *debug = NULL;
			gst_message_parse_warning(msg, &error, &debug);
			g_print("\n Warning received from %s : %s", GST_OBJECT_NAME(msg->src), error->message);
			g_print("\n Debug Info : %s\n",(debug)? debug : "None");
			g_error_free(error);
			g_free(debug);
		}
		break;
  /* Handling pipeline state change message */
    case GST_MESSAGE_STATE_CHANGED: {
      if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->pipeline)) {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
        g_print("\nPipeline state changed from '%s' to '%s'\n", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
      }
    }
    break;
  /* Handling message info */
    case GST_MESSAGE_INFO:	{
			GError *error = NULL;
			gchar *debug = NULL;
			gst_message_parse_info(msg, &error, &debug);
			g_print("\n Info received from %s : %s", GST_OBJECT_NAME(msg->src), error->message);
			g_print("\n Debug Info : %s\n",(debug)? debug : "None");
			g_error_free(error);
			g_free(debug);
		}
		break;
    default: 
    break;
  }
}

#endif
