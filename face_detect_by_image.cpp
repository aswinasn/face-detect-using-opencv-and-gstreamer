
#include "face_detect_by_video.cpp"

// typedef struct image{
//     GstElement *pipeline, *filesrc, *decodebin, *videoconvert,
//                *facedetect, *videoconvert2, *imagefreez, *autovideosink;
//     GstBus *bus;
// 	GstMessage *msg;
// 	GstStateChangeReturn ret;
// 	gboolean terminate = FALSE;
// } image;

// static void handle_pads(GstElement *src, GstPad *pad, GstElement *des) {
// 	GstPad *sink = gst_element_get_static_pad(des, "sink");
// 	gst_pad_link(pad, sink);
// 	gst_object_unref(sink);
// }

///////////////////////////////////////////////////////////////////////////////////////////

void face_detect_by_image(){

    image i;

    VideoCapture cap(0); // here we can use CAP_GSTREAMER here ----------------------------

    if(!cap.isOpened()){
        cout << "\n fail to open cam : ";
        exit(0);
    }
	double fps = cap.get(CAP_PROP_FPS);  /// getting teh frame rate------------------------- 

    Mat frame;

    cout << "\n press q to end the video and take picture ";
    while(true){
        cap.read(frame); /// reading frames from cap ---------------------------------------
        imshow("video", frame);

        if(waitKey(25) == 'q'){

            imwrite("/home/ee213104/Desktop/image_rec.png", frame); /// writing frames in to file ---------------------------------
            break;
        }
        //waitKey(40);
    }

    cap.release();

    // to gstreamer ------------------------------------------------

    /// creating pipeline ------------------------------------------
    i.pipeline = gst_pipeline_new("display-image");

    // creating elements -------------------------------------------
    i.filesrc = gst_element_factory_make("filesrc", NULL);
    g_object_set(G_OBJECT(i.filesrc), "location", "/home/ee213104/Desktop/image_rec.png", NULL);

    i.decodebin = gst_element_factory_make("decodebin", NULL);
    i.videoconvert = gst_element_factory_make("videoconvert", NULL);
    i.facedetect = gst_element_factory_make("facedetect", NULL);
    g_object_set(G_OBJECT(i.facedetect), "min-size-width", 60, "min-size-height", 60, "profile", "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml", NULL);
    i.videoconvert2 = gst_element_factory_make("videoconvert", NULL);
    i.imagefreez = gst_element_factory_make("imagefreeze", NULL);
    i.autovideosink = gst_element_factory_make("autovideosink", NULL);

    // adding element to the pipeline -------------------------------
    if(!i.filesrc || !i.decodebin || !i.videoconvert || !i.facedetect || !i.videoconvert2 || !i.imagefreez || !i.autovideosink){
        cout << "\n unable to set the pipeline ";
        exit(0);
    }
    gst_bin_add_many(GST_BIN(i.pipeline), i.filesrc, i.decodebin, i.videoconvert, i.facedetect, i.videoconvert2, i.imagefreez, i.autovideosink, NULL);

    /// linking the elements ----------------------------------------
    if(!gst_element_link(i.filesrc, i.decodebin)){
        cout << "\n failed to link filesrc - decodebin";
        exit(0);
    }

    if(!g_signal_connect(i.decodebin, "pad-added", G_CALLBACK(handle_pads), i.videoconvert)){
        cout << "\n failed to connect decodebin to videoconvert";
        exit(0);
    }

    if(!gst_element_link_many(i.videoconvert, i.facedetect, i.videoconvert2, i.imagefreez, i.autovideosink, NULL)){
        cout << "\n failed to link after pad handler ";
        exit(0);
    }

    /// start pipeline to playing state ------------------------------
    i.ret = gst_element_set_state(i.pipeline, GST_STATE_PLAYING);
	if(i.ret == GST_STATE_CHANGE_FAILURE){
		cout << "\n unable to set pipeline to playing state -> ";
		gst_object_unref(i.pipeline);
		exit;
	}

    // Wait for error or end of stream -----------------------------------------
	i.bus = gst_element_get_bus(i.pipeline);
		while(true){
			i.msg = gst_bus_timed_pop_filtered(i.bus, GST_CLOCK_TIME_NONE, GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_STATE_CHANGED));
			/* Parse message */
			if (i.msg != NULL) {
				GError *err;
				gchar *debug_info;
				if (GST_MESSAGE_TYPE (i.msg) == GST_MESSAGE_ERROR) {
						gst_message_parse_error (i.msg, &err, &debug_info);
                        cout << "\nError received from element " << GST_OBJECT_NAME (i.msg->src) << ": " << err->message;
                        cout << "\nDebugging information: " << (debug_info ? debug_info : "none");
						g_clear_error (&err);
						g_free (debug_info);
				}
				if (GST_MESSAGE_TYPE(i.msg) == GST_MESSAGE_EOS) {
						cout << "\n End-Of-Stream reached ";
						i.terminate = TRUE;
						break;
				}
				if (GST_MESSAGE_TYPE(i.msg) == GST_MESSAGE_STATE_CHANGED) {

					// state changed messages from the pipeline -----------------------------------
						if (GST_MESSAGE_SRC (i.msg) == GST_OBJECT (i.pipeline)) {
							GstState old_state, new_state, pending_state;
							gst_message_parse_state_changed (i.msg, &old_state, &new_state, &pending_state);
                            cout << "\nPipeline state changed from " << gst_element_state_get_name(old_state)
                                 << " to " << gst_element_state_get_name(new_state) << ":" << std::endl;
						  }
				}
				gst_message_unref(i.msg);
			}
		}

	// Free resources
	gst_object_unref(i.bus);
	gst_element_set_state(i.pipeline, GST_STATE_NULL);
	gst_object_unref(i.pipeline);

}