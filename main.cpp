#include <iostream>
#include <sstream>
#include "kinect.hpp"

#include <k4a/k4a.h>
#include <k4arecord/playback.h>
#include <string>
#include "transformation_helpers.h"




static bool point_cloud_depth_to_color(k4a_transformation_t transformation_handle,
    const k4a_image_t depth_image,
    const k4a_image_t color_image,
    std::string file_name)
{

    std::cout << "Inside" << std::endl;
    // transform color image into depth camera geometry
    int color_image_width_pixels = k4a_image_get_width_pixels(color_image);
    int color_image_height_pixels = k4a_image_get_height_pixels(color_image);

    std::cout << color_image_width_pixels << std::endl;

    k4a_image_t transformed_depth_image = NULL;
    if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,
        color_image_width_pixels,
        color_image_height_pixels,
        color_image_width_pixels * (int)sizeof(uint16_t),
        &transformed_depth_image))
    {
        printf("Failed to create transformed depth image\n");
        return false;
    }

    k4a_image_t point_cloud_image = NULL;
    if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
        color_image_width_pixels,
        color_image_height_pixels,
        color_image_width_pixels * 3 * (int)sizeof(int16_t),
        &point_cloud_image))
    {
        printf("Failed to create point cloud image\n");
        return false;
    }

    if (K4A_RESULT_SUCCEEDED !=
        k4a_transformation_depth_image_to_color_camera(transformation_handle, depth_image, transformed_depth_image))
    {
        printf("Failed to compute transformed depth image\n");
        return false;
    }

    if (K4A_RESULT_SUCCEEDED != k4a_transformation_depth_image_to_point_cloud(transformation_handle,
        transformed_depth_image,
        K4A_CALIBRATION_TYPE_COLOR,
        point_cloud_image))
    {
        printf("Failed to compute point cloud\n");
        return false;
    }

    tranformation_helpers_write_point_cloud(point_cloud_image, color_image, file_name.c_str());

    k4a_image_release(transformed_depth_image);
    k4a_image_release(point_cloud_image);

    std::cout << "Capture" << std::endl;

    return true;
}

static int capture(std::string output_dir, k4a_device_t device, k4a_calibration_t calibration, uint8_t deviceId = K4A_DEVICE_DEFAULT) //k4a_device????
{
    int returnCode = 1;

    const int32_t TIMEOUT_IN_MS = 10000;
    k4a_transformation_t transformation = NULL;
    k4a_capture_t capture = NULL;
    std::string file_name = "";
   
    k4a_image_t depth_image = NULL;
    k4a_image_t color_image = NULL;
    k4a_image_t color_image_downscaled = NULL;

    //k4a_calibration_t calibration;
    //if (K4A_RESULT_SUCCEEDED !=
    //    k4a_device_get_calibration(device, config.depth_mode, config.color_resolution, &calibration))
    //{
    //    printf("Failed to get calibration\n");
    //    goto Exit;
    //}

    transformation = k4a_transformation_create(&calibration);


    // Get a capture
    switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
    {
    case K4A_WAIT_RESULT_SUCCEEDED:
        break;
    case K4A_WAIT_RESULT_TIMEOUT:
        printf("Timed out waiting for a capture\n");
        goto Exit;
    case K4A_WAIT_RESULT_FAILED:
        printf("Failed to read a capture\n");
        goto Exit;
    }

    // Get a depth image
    depth_image = k4a_capture_get_depth_image(capture);
    if (depth_image == 0)
    {
        printf("Failed to get depth image from capture\n");
        goto Exit;
    }

    // Get a color image
    color_image = k4a_capture_get_color_image(capture);
    if (color_image == 0)
    {
        printf("Failed to get color image from capture\n");
        goto Exit;
    }

    // Compute color point cloud by warping depth image into color camera geometry
#ifdef _WIN32
    file_name = output_dir + ".ply";
    std::cout << file_name << std::endl;
#else
    file_name = output_dir + "/depth_to_color.ply";
#endif
    if (point_cloud_depth_to_color(transformation, depth_image, color_image, file_name.c_str()) == false)
    {
        std::cout << "Capture" << std::endl;
        goto Exit;
    }

Exit:
    if (depth_image != NULL)
    {
        k4a_image_release(depth_image);
    }
    if (color_image != NULL)
    {
        k4a_image_release(color_image);
    }
    if (capture != NULL)
    {
        k4a_capture_release(capture);
    }
    if (transformation != NULL)
    {
        k4a_transformation_destroy(transformation);
    }
    //if (device != NULL)
    //{
    //    k4a_device_close(device);
    //}
    return returnCode;
}



int main(int argc, char* argv[])
{
    try {
        std::string filename = "C:\\Users\\merle\\Documents\\Projekte\\3DRoomSurveillance\\pointClouds\\pc1"; //".\\pointClouds\\pc";
        std::cout << filename << std::endl;
        kinect kinect;
        int i = 0;

        while (true) {
            // Update
            kinect.update();

            // Draw
            kinect.draw();

            // Show
            kinect.show();

            if (i == 0) {
                capture(filename, kinect.device.handle(), kinect.calibration);
                i++;
            }

            // Wait Key
            constexpr int32_t delay = 30;
            const int32_t key = cv::waitKey(delay);
            if (key == 'q' && i > 0) {
                break;
            }

#ifdef HAVE_OPENCV_VIZ
            if (viewer.wasStopped()) {
                break;
            }
#endif
        }

    }
    catch (const k4a::error& error) {
        std::cout << error.what() << std::endl;
    }

    return 0;
}