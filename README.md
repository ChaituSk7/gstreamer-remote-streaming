# gstreamer-remote-streaming

Write Gstreamer application which will play audio-video data on remote machine.

- Play command will be given from 1 machine (host machine) and audio/video will be rendered on remote machine.

- Capture all Gstreamer events and print it

- Capture all messages on bus and print it.

- print meta data of audio/video stream on console

- play one file -> pause -> send flush command -> switch to next stream/file

     -> Capture all state-change, events from Gstreamer in application

- Demonstrate use of gst_add_pad_probe api in pipeline.

- Do not use playbin/decodebin in pipepline.

- take user inputs from keyboard (play/pause/seek/flush/query for position, duration)

- change the volume of a pipeline

- Generate a thumbnail from a gst video source
