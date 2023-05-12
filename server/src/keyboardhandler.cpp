#include "keyboardhandler.h"
#include <iostream>

/* This function will handle the keyboard */
void
handle_menu (CustomData * data, gchar input)
{
  double current_volume;
  if (input == 'p') {
    /* Change State To PLAYING */
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
  } else if (input == 's') {
    /* Change State to PAUSED */
    gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
    g_printerr
        ("Do not keep the pipeline for pause state more than 5 seconds\n");
  } else if (input == 't') {
    /* Track the current position */
    gint64 cur_pos;
    gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &cur_pos);
    g_print ("\n Current Position : %ld Min %ld Second. \n",
        ((cur_pos / 1000000000) / 60), ((cur_pos / 1000000000) % 60));
  } else if (input == 'd') {
    /* Print the Duration of the Stream */
    gint64 duration;
    gst_element_query_duration (data->pipeline, GST_FORMAT_TIME, &duration);
    g_print ("\n Duration : %ld : %ld Min\n", ((duration / 1000000000) / 60),
        ((duration / 1000000000) % 60));

  } else if (input == 'c') {
    /* seek 10 sec forward */
    gint64 cur_pos;
    gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &cur_pos);
    g_print ("\n Current Position : %ld Second\n ", (cur_pos) / 1000000000);
    gst_element_seek_simple (data->pipeline, GST_FORMAT_TIME,
        GstSeekFlags (GST_SEEK_FLAG_FLUSH), (cur_pos + 10 * GST_SECOND));
  } else if (input == 'k') {
    g_print ("\nInteractive mode - keyboard controls:\n\n"
        "     p         :  play\n"
        "     s         :  pause\n"
        "     n         :  play next\n"
        "     k         :  show keyboard shorcuts\n"
        "     d         :  Duration of media\n"
        "     c         :  Seek 10 sec forward\n"
        "     t         :  Current position of media\n"
        "     m         :  Print metadata\n"
        "     v         :  increase volume\n"
        "     u         :  decrease volume\n" "     q         :  quit\n");
  } else if (input == 'n') {
    /* Flush the existing Stream Data and Play the next Stream */
    gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
    gst_event_new_seek (1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, 0);
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    g_main_loop_quit (data->loop);

  } else if (input == 'q') {
    /* Exit the Program */
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    g_main_loop_quit (data->loop);
    exit (0);
  } else if (input == 'm') {
    /* This will calls the metadata funtion */
    g_print ("\n********************************\n");
    metadata_fun (data->path);
    g_print ("\n********************************\n");

  } else if (input == 'v') {
    /* Increasing the volume */
    g_object_get (G_OBJECT (data->volume), "volume", &current_volume, NULL);
    if (current_volume < 1.0) {
      gdouble new_vol = current_volume + 0.05;
      g_object_set (G_OBJECT (data->volume), "volume", new_vol, NULL);
      g_print ("vol: %f \n", new_vol);
      gint new_volume = (gint) (new_vol * 9 + 1);
      g_print ("Volume increased to %d\n", new_volume);
    } else {
      std::cout << "maximum level" << std::endl;
    }
  } else if (input == 'u') {
    /* Increasing the volume */
    g_object_get (G_OBJECT (data->volume), "volume", &current_volume, NULL);
    std::cout << "Volume : " << current_volume << std::endl;
    if (current_volume > 0.0) {
      gdouble new_vol = current_volume - 0.111;
      g_object_set (G_OBJECT (data->volume), "volume", new_vol, NULL);
      gint new_volume = (gint) (new_vol * 9 + 1);
      g_print ("Volume decreased to %d\n", new_volume);
    }
  }

  else {
    g_printerr ("Unexted key received.\n");
  }
}

/* This function will call the handle menu function */
gboolean
handle_keyboard (GIOChannel * source, GIOCondition cond, CustomData * data)
{
  gchar *str = NULL;
  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) != G_IO_STATUS_NORMAL) {
    return TRUE;
  }
  handle_menu (data, g_ascii_tolower (str[0]));
  return TRUE;
}

/* This fucntion can handle messages that comes over bus for every pipeline */
gboolean
msg_handle (GstBus * bus, GstMessage * msg, CustomData * data)
{
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_UNKNOWN:
      g_print ("\n Unknown Message Received.\n");
      break;
    case GST_MESSAGE_EOS:
      g_print ("\n End of Stream Reached.\n");
      gst_element_set_state (data->pipeline, GST_STATE_NULL);
      g_main_loop_quit (data->loop);
      break;

    case GST_MESSAGE_ERROR:{
      GError *error = NULL;
      gchar *debug = NULL;
      gst_message_parse_error (msg, &error, &debug);
      g_print ("\n Error received from %s : %s", GST_OBJECT_NAME (msg->src),
          error->message);
      g_print ("\n Debug Info : %s\n", (debug) ? debug : "None");
      g_error_free (error);
      g_free (debug);
      gst_element_set_state (data->pipeline, GST_STATE_NULL);
      g_main_loop_quit (data->loop);
    }
      break;

    case GST_MESSAGE_WARNING:{
      GError *error = NULL;
      gchar *debug = NULL;
      gst_message_parse_warning (msg, &error, &debug);
      g_print ("\n Error received from %s : %s", GST_OBJECT_NAME (msg->src),
          error->message);
      g_print ("\n Debug Info : %s\n", (debug) ? debug : "None");
      g_error_free (error);
      g_free (debug);
    }
      break;

    case GST_MESSAGE_INFO:{
      GError *error = NULL;
      gchar *debug = NULL;
      gst_message_parse_info (msg, &error, &debug);
      g_print ("\n Error received from %s : %s", GST_OBJECT_NAME (msg->src),
          error->message);
      g_print ("\n Debug Info : %s\n", (debug) ? debug : "None");
      g_error_free (error);
      g_free (debug);
    }
      break;
      /* case GST_MESSAGE_STATE_CHANGED:
         if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->pipeline)) {
         GstState old_state, new_state, pending_state;
         gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
         g_print("Pipeline state changed from '%s' to '%s'\n", gst_element_state_get_name(old_state),
         gst_element_state_get_name(new_state));
         }
         break; */
    default:
      break;
  }

  return true;
}
