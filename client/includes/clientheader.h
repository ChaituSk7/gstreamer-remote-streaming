#ifndef CLIENTHEADER_H
#define CLIENTHEADER_H
#include <gst/gst.h>

typedef struct _RemoteMp4 {
    GstElement *pipeline;
    GstElement *udp_source;
    GstElement *video_watchdog;
    GstElement *caps_filter;
    GstElement *rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_sink;

    GstElement *udp_audio_source;
    GstElement *audio_watchdog;
    GstElement *audio_rtp_buffer;
    GstElement *audio_rtp_depay;
    GstElement *audio_decoder;
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    gboolean is_live;
    GMainLoop *loop;
}RemoteMp4;

typedef struct _RemoteMp3 {
    GstElement *pipeline;
    GstElement *udp_source;
    GstElement *watchdog;
    GstElement *rtp_depay;
    GstElement *parser;
    GstElement *audio_decoder;
    GstElement *queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    GMainLoop *loop;
}RemoteMp3;

typedef struct _RemoteWebM {
    GstElement *pipeline;
    GstElement *udp_video_source;
    GstElement *video_watchdog;
    GstElement *video_rtp_buffer;
    GstElement *video_rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_convert;
    GstElement *video_sink;

    GstElement *udp_audio_source;
    GstElement *audio_watchdog;
    GstElement *audio_rtp_buffer;
    GstElement *audio_rtp_depay;
    GstElement *audio_decoder;
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    GMainLoop *loop;
}RemoteWebM;

typedef struct _RemoteAvi {
    GstElement *pipeline;
    GstElement *udp_video_source;
    GstElement *video_watchdog;
    GstElement *video_rtp_buffer;
    GstElement *video_rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_convert;
    GstElement *video_sink;

    GstElement *udp_audio_source;
    GstElement *audio_watchdog;
    GstElement *audio_rtp_buffer;
    GstElement *audio_rtp_depay;
    GstElement *audio_decoder;
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    GMainLoop *loop;
}RemoteAvi;

typedef struct _Thumbnail {
    GstElement *pipeline;
    GstElement *udpsrc;
    GstElement *watchdog;
    GstElement *caps_filter;
    GstElement *rtp_depay;
    GstElement *decoder;
    GstElement *videoconvert;
    GstElement *sink;
    GMainLoop *loop;
}Thumbnail;

extern int remotehost_Mp4_pipeline (int, char *[]);

extern int remotehost_Mp3_pipeline (int, char *[]);
    
extern int remotehost_WebM_pipeline (int, char *[]);

extern int remotehost_Avi_pipeline (int, char *[]);

extern int thumbnail_pipeline (int, char *[]);

#endif