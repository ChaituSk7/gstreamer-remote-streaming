#ifndef PROBE_H
#define PROBE_H

#include "header.h"
#include <iostream>
#include <string>

/* Probe Use as a Call back Function to Print Events and Capability */
GstPadProbeReturn probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
  GstEvent *event = GST_PAD_PROBE_INFO_EVENT(info);
  GstTagList *tag_list;

  /* Print Segment */
  if (GST_EVENT_TYPE(event) == GST_EVENT_SEGMENT) {
    const GstSegment *segment;
    gst_event_parse_segment(event, &segment);
    g_print("Segment: %" GST_SEGMENT_FORMAT "\n", GST_SEGMENT_FORMAT);
  } 

  /* Print Capability of SinkPad */
  else if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
    g_print("\n*..........Capabilties Of Pad:..........*\n");
    GstCaps *caps;
    gst_event_parse_caps(event, &caps);
    gchar *caps_string = gst_caps_to_string(caps);
    size_t pos = 0;
    std :: string line;
    std :: string cap = std :: string(caps_string);
    while ((pos = cap.find(',')) != std :: string :: npos) {
      line = cap.substr(0, pos);
      std :: cout << line << std :: endl;
      cap.erase(0, pos + 1);
    }
    std :: cout << cap << std :: endl;
    g_free(caps_string);
  }
    else if (GST_EVENT_TYPE(event) == GST_EVENT_TAG) {
    gst_event_parse_tag(event, &tag_list);
  }

  /* Print Event get from SinkPad */
  else {
    if(GST_EVENT_TYPE_NAME(event)!= "tag"){
      g_print("Event: %s\n", GST_EVENT_TYPE_NAME(event));
    }
  }
  return GST_PAD_PROBE_OK;
}

#endif