
#include "elements.cpp"

////////////////////////////////////////////////////////////////////////////////////////////

void face_detect_by_video(){

    video v;

    VideoCapture cap(0); /// we can use CAP_GSTREAMER here ---------------------------------

    if(!cap.isOpened()){
        cout << "\n fail to open cam : ";
        exit(0);
    }
	double fps = cap.get(CAP_PROP_FPS);  /// getting teh frame rate------------------------- 

    // Get video properties ----------------------------------------------------------------
	VideoWriter output("/home/ee213104/Desktop/face_detection_project/video_rec.mp4", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT)));

    Mat frame;

    cout << "\n press q to end the video from record mode ";
    while(true){
        cap.read(frame); /// reading frames from cap ---------------------------------------
        imshow("video", frame);

        output.write(frame); /// writing frames in to file ---------------------------------

        // cout << "\n press q to end the video from record mode "; 
        if(waitKey(25) == 'q'){
            break;
        }
        //waitKey(40);
    }

    output.release();
    cap.release();

    /// to gstreamer ----------------------------------------------

    int argc; char** argv;
    gst_init(&argc, &argv);

    // creating gstreamer pipeline --------------------------------
    v.pipeline = gst_pipeline_new("mp4-player");

    v.filesrc = gst_element_factory_make("filesrc", NULL);
    g_object_set(G_OBJECT(v.filesrc), "location", "/home/ee213104/Desktop/face_detection_project/video_rec.mp4", NULL);

    // v.decodebin = gst_element_factory_make("decodebin", NULL);
    v.decodebin = gst_element_factory_make("decodebin", NULL);
    v.videoconvert = gst_element_factory_make("videoconvert", NULL);
    v.facedetect = gst_element_factory_make("facedetect", NULL);
    g_object_set(G_OBJECT(v.facedetect), "min-size-width", 50, "min-size-height", 50, "profile", "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml", NULL);
    v.videoconvert2 = gst_element_factory_make("videoconvert", NULL);
    v.autovideosink = gst_element_factory_make("autovideosink", NULL);

    /// adding elements to the pipeline ----------------------------
    if(!v.filesrc || !v.decodebin || !v.videoconvert || !v.facedetect || !v.videoconvert2 || !v.autovideosink){
        cout << "\n unable to set the pipeline ";
        exit(0);
    }
    gst_bin_add_many(GST_BIN(v.pipeline), v.filesrc, v.decodebin, v.videoconvert, v.facedetect, v.videoconvert2, v.autovideosink, NULL);

    /// linking elements ----------------------------
    if(!gst_element_link(v.filesrc, v.decodebin)){
        cout << "\n failed to link filesrc - decodebin ";
        exit(0);
    }

    if(!g_signal_connect(v.decodebin, "pad-added", G_CALLBACK(handle_pads), v.videoconvert)){
        cout << "\n failed to connect decodebin - videoconvert / fail occur in pad-handler ";
        exit(0);
    }

    if(!gst_element_link_many(v.videoconvert, v.facedetect, v.videoconvert2, v.autovideosink, NULL)){
        cout << "\n failed to link after pad handler ";
        exit(0);
    }

    // start pipeline to playing state ------------------------------
    v.ret = gst_element_set_state(v.pipeline, GST_STATE_PLAYING);
	if(v.ret == GST_STATE_CHANGE_FAILURE){
		cout << "\n unable to set pipeline to playing state -> ";
		gst_object_unref(v.pipeline);
		exit;
	}

    // Wait for error or end of stream -------------------------
	v.bus = gst_element_get_bus(v.pipeline);
		while(true){
			v.msg = gst_bus_timed_pop_filtered(v.bus, GST_CLOCK_TIME_NONE, GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_STATE_CHANGED));
			/* Parse message */
			if (v.msg != NULL) {
				GError *err;
				gchar *debug_info;
				if (GST_MESSAGE_TYPE (v.msg) == GST_MESSAGE_ERROR) {
						gst_message_parse_error (v.msg, &err, &debug_info);
                        cout << "\nError received from element " << GST_OBJECT_NAME (v.msg->src) << ": " << err->message;
                        cout << "\nDebugging information: " << (debug_info ? debug_info : "none");
						g_clear_error (&err);
						g_free (debug_info);
				}
				if (GST_MESSAGE_TYPE(v.msg) == GST_MESSAGE_EOS) {
						cout << "\n End-Of-Stream reached ";
						v.terminate = TRUE;
						break;
				}
				if (GST_MESSAGE_TYPE(v.msg) == GST_MESSAGE_STATE_CHANGED) {

					// state changed messages from the pipeline -----------------------------------------
						if (GST_MESSAGE_SRC (v.msg) == GST_OBJECT (v.pipeline)) {
							GstState old_state, new_state, pending_state;
							gst_message_parse_state_changed (v.msg, &old_state, &new_state, &pending_state);
                            cout << "\nPipeline state changed from " << gst_element_state_get_name(old_state)
                                 << " to " << gst_element_state_get_name(new_state) << ":" << std::endl;
						  }
				}
				gst_message_unref(v.msg);
			}
		}

	// Free resources
	gst_object_unref(v.bus);
	gst_element_set_state(v.pipeline, GST_STATE_NULL);
	gst_object_unref(v.pipeline);

}