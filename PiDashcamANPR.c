#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/* 
 * This program implements the following command line tool pipeline:
 * gst-launch-1.0 v4l2src num-buffers=150 ! capsfilter caps=image/jpeg,width=640,height=360 !
 * queue ! splitmuxsink muxer=matroskamux location=/tmp/video%02d.mkv max-size-time=2000000000
*/

/* Structure to contain all elements in pipeline */
typedef struct _PipeElements {
  GstElement *pipeline;
  GstElement *source;
  GstElement *caps;
  GstElement *queue;
  GstElement *sink;
} PipeElements;


int
dashcam_stream_main (int argc, char *argv[])
{
  GstElement *pipeline, *udpsource, *capsfilter, *depay, *dec, *sink;
  PipeElements pipe;
  GstCaps *caps_info;
  GstBus *bus;
  GstMessage *msg;
  const GstStructure *msgstruct;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  pipe.source = gst_element_factory_make ("v4l2src", "source");
  pipe.caps = gst_element_factory_make ("capsfilter", "caps");
  pipe.queue = gst_element_factory_make ("queue", "queue");
  pipe.sink = gst_element_factory_make ("splitmuxsink", "sink");
  

  /* Create the empty pipeline */
  pipe.pipeline = gst_pipeline_new ("dashcam-pipeline");

  if (!pipe.pipeline || !pipe.source || !pipe.caps || !pipe.queue || !pipe.sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipe.pipeline), pipe.source, pipe.caps, pipe.queue, pipe.sink, NULL);
  if (gst_element_link_many (pipe.source, pipe.caps, pipe.queue, pipe.sink, NULL) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipe.pipeline);
    return -1;
  }
  else {
    g_printerr ("Elements linked sucessfully!\n");
  }

  /* Modify the element properties */
  g_object_set (pipe.source, "num-buffers", 150, NULL);
  caps_info = gst_caps_new_simple("image/jpeg", "width", G_TYPE_INT, 640, "height", G_TYPE_INT, 360, NULL);
  g_object_set (pipe.caps, "caps", caps_info, NULL);
  //g_object_set (pipe.sink, "muxer", "matroskamux", NULL); <- The muxer property "value" should be a reference to the desired muxer element
  g_object_set (pipe.sink, "location", "/tmp/video%02d.mkv", NULL);
  g_object_set (pipe.sink, "max-size-time", 2000000000, NULL);
  
  
  /* Start playing */
  ret = gst_element_set_state (pipe.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipe.pipeline);
    return -1;
  }

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipe.pipeline);
  
  while (TRUE) {
    
  
    msg =
        gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_ANY);
    
    msgstruct = gst_message_get_structure(msg);
    g_printerr ("%s\n", gst_structure_get_name(msgstruct));

    /* Parse message */
    if (msg != NULL) {
      GError *err;
      gchar *debug_info;
      
      

      switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR:
          gst_message_parse_error (msg, &err, &debug_info);
          g_printerr ("Error received from element %s: %s\n",
              GST_OBJECT_NAME (msg->src), err->message);
          g_printerr ("Debugging information: %s\n",
              debug_info ? debug_info : "none");
          g_clear_error (&err);
          g_free (debug_info);
          break;
        case GST_MESSAGE_EOS:
          g_print ("End-Of-Stream reached.\n");
          break;
        default:
          g_printerr ("Message received.\n");

      }
    
    }
  }

  /* Free resources */
  gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipe.pipeline, GST_STATE_NULL);
  gst_object_unref (pipe.pipeline);
  return 0;
}

int
main (int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
  return gst_macos_main (dashcam_stream_main, argc, argv, NULL);
#else
  return dashcam_stream_main (argc, argv);
#endif
}
