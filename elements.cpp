
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <gst/gst.h>

using namespace std;
using namespace cv;

////// gst elements for video ----------------------------------

typedef struct video{
	GstElement *pipeline, *filesrc, *decodebin, *videoconvert,
               *facedetect, *videoconvert2, *autovideosink;
    GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

} video;

// // gst elements for image -------------------------------------

// typedef struct image{
//     GstElement *pipeline, *filesrc, *decodebin, *videoconvert,
//                *facedetect, *videoconvert2, *imagefreez, *autovideosink;
//     GstBus *bus;
// 	GstMessage *msg;
// 	GstStateChangeReturn ret;
// 	gboolean terminate = FALSE;
// } image;

// pad handler -----------------------------------------------

static void handle_pads(GstElement *src, GstPad *pad, GstElement *des) {
	GstPad *sink = gst_element_get_static_pad(des, "sink");
	gst_pad_link(pad, sink);
	gst_object_unref(sink);
}