#ifndef KEYBOARDHANDLER_H
#define KEYBOARDHANDLER_H
#include "header.h"

/* Structure for handling the keyboard and messages over the bus*/

typedef struct _Customdata
{
  std::string path;
  GstElement *pipeline;
  GstElement *volume;
  GMainLoop *loop;
} CustomData;

/* fuction declaration for handling keyboard and messages */

extern gboolean handle_keyboard (GIOChannel *, GIOCondition, CustomData *);

extern gboolean msg_handle (GstBus *, GstMessage *, CustomData *);

#endif
