#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    std::string pipeline = "udpsrc port=5000 ! application/x-rtp, payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink";

    GstElement *pipe = gst_parse_launch(pipeline.c_str(), NULL);
    if (!pipe) {
        std::cerr << "Failed to create pipeline" << std::endl;
        return -1;
    }

    gst_element_set_state(pipe, GST_STATE_PLAYING);

    GstBus *bus = gst_element_get_bus(pipe);
    if (!bus) {
        std::cerr << "Failed to get bus" << std::endl;
        return -1;
    }

    while (true) {
        GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

        if (msg) {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR:
                    gst_message_parse_error(msg, &err, &debug_info);
                    std::cerr << "Error received from element " << GST_OBJECT_NAME(msg->src) << ": " << err->message << std::endl;
                    g_clear_error(&err);
                    g_free(debug_info);
                    goto done;
                case GST_MESSAGE_EOS:
                    std::cout << "End-Of-Stream reached." << std::endl;
                    goto done;
                default:
                    g_printerr("Unexpected message received.\n");
            }
            gst_message_unref(msg);
        }
    }

done:
    gst_element_set_state(pipe, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipe);

    return 0;
}
