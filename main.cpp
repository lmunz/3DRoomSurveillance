#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <thread>
#include "kinect.hpp"
#include <string>
#include <semaphore>
#include <Windows.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include <mutex>
#include <process.h>
#include <stdio.h>

#include "opencv2/core/utility.hpp"
#include <k4a/k4a.h>
#include <k4arecord/playback.h>
#include <string>
#include "transformation_helpers.h"


using namespace std;

string pcArray[2];

static bool point_cloud_depth_to_color(k4a_transformation_t transformation_handle,
    const k4a_image_t depth_image,
    const k4a_image_t color_image,
    std::string file_name)
{

    // transform color image into depth camera geometry
    int color_image_width_pixels = k4a_image_get_width_pixels(color_image);
    int color_image_height_pixels = k4a_image_get_height_pixels(color_image);

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

    return true;
}

static int getPointCloud(k4a_transformation_t transformation, k4a_image_t depth_image, k4a_image_t color_image, string file_name) {
    int returnCode = 1;

    if (point_cloud_depth_to_color(transformation, depth_image, color_image, file_name.c_str()) == false)
    {
        std::cout << "Capture" << std::endl;
        goto Exit;
    }

Exit:
    if (transformation != NULL)
    {
        k4a_transformation_destroy(transformation);
    }
    std::cout << "exit" << std::endl;

    return returnCode;
}

static int capture(std::string output_dir, k4a_device_t device, k4a_calibration_t calibration) //k4a_device????
{
//    std::cout << "start" << std::endl;
//    int returnCode = 1;
//
//    const int32_t TIMEOUT_IN_MS = 10000;
//    k4a_transformation_t transformation = NULL;
//    k4a_capture_t capture = NULL;
//    std::string file_name = "";
//    k4a_image_t depth_image = NULL;
//    k4a_image_t color_image = NULL;
//
//    //k4a_calibration_t calibration;
//    //if (K4A_RESULT_SUCCEEDED !=
//    //    k4a_device_get_calibration(device, config.depth_mode, config.color_resolution, &calibration))
//    //{
//    //    printf("Failed to get calibration\n");
//    //    goto Exit;
//    //}
//
//    transformation = k4a_transformation_create(&calibration);
//
//
//    // Get a capture
//    switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
//    {
//    case K4A_WAIT_RESULT_SUCCEEDED:
//        break;
//    case K4A_WAIT_RESULT_TIMEOUT:
//        printf("Timed out waiting for a capture\n");
//        //goto Exit;
//    case K4A_WAIT_RESULT_FAILED:
//        printf("Failed to read a capture\n");
//        //goto Exit;
//    }
//
//    // Get a depth image
//    depth_image = k4a_capture_get_depth_image(capture);
//    if (depth_image == 0)
//    {
//        printf("Failed to get depth image from capture\n");
//        //goto Exit;
//    }
//    std::cout << "depth" << std::endl;
//    // Get a color image
//    color_image = k4a_capture_get_color_image(capture);
//    if (color_image == 0)
//    {
//        printf("Failed to get color image from capture\n");
//        //goto Exit;
//    }
//    std::cout << "color" << std::endl;
//    // Compute color point cloud by warping depth image into color camera geometry
//#ifdef _WIN32
//    file_name = output_dir + ".ply";
//#else
//    file_name = output_dir + "/depth_to_color.ply";
//#endif
//    thread th1(getPointCloud, transformation, depth_image, color_image, file_name);
//    th1.join();
//    std::cout << "point" << std::endl;
////Exit:
////    if (depth_image != NULL)
////    {
////        k4a_image_release(depth_image);
////    }
////    if (color_image != NULL)
////    {
////        k4a_image_release(color_image);
////    }
////    if (capture != NULL)
////    {
////        k4a_capture_release(capture);
////    }
////    std::cout << "exit" << std::endl;
//    //if (device != NULL)
//    //{
//    //    k4a_device_close(device);
//    //}
//    //std::this_thread::sleep_for(std::chrono::minutes(1));
//    return returnCode;
}

static int matching(string file_pc) {

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    int matchResult = 6;
    int size = sizeof(pcArray);

    if (pcArray[0] == "") {
        pcArray[0] = file_pc;
        matchResult = 3;
    }
    else if (pcArray[1] == "") {
        pcArray[1] = file_pc;

        string file1 = pcArray[0];
        string file2 = pcArray[1];

        matchResult = system(("C:\\Users\\merle\\Documents\\Projekte\\3DRoomSurveillance\\executable\\PointCloudMatching.exe \" " + pcArray[0] + "\" HelloWorld \"" + pcArray[1]).c_str());
    }
    else {
        matchResult = 3;
    }

    //cout << matchResult << endl;

    switch (matchResult) {
    case 0:
        return 0;
        break;
    case 1:
        pcArray[0] = pcArray[1];
        pcArray[1] = "";
        return 0;
        break;
    case 2:
        pcArray[0] = pcArray[1];
        pcArray[1] = "";
        return 1;
        break;
    case 3:
        return 2;
        break;
    default:
        pcArray[0] = "";
        pcArray[1] = "";
        return 6;
        break;
    }
    
}

int main(int argc, char* argv[])
{
    try { 
        string filename = "C:/Users/merle/Documents/Projekte/3DRoomSurveillance/pointClouds/pc"; //".\\pointClouds\\pc";
        kinect kinect;
        int i = 1;
        bool working = false;

        time_t timer;
        time_t old_timer;
        time_t join_timer;

        time(&old_timer);
        time(&join_timer);

        while (true) {
            // Update
            kinect.update();

            // Draw
            kinect.draw();

            // Show
            kinect.show();

            time(&timer);

            if (difftime(timer, old_timer) == 10) {
                string output_dir = filename + std::to_string(i);

                std::cout << "start" << std::endl;
                int returnCode = 1;

                const int32_t TIMEOUT_IN_MS = 10000;
                std::string file_name = "";
                k4a_transformation_t transformation = NULL;
                k4a_capture_t capture = NULL;
                k4a_image_t depth_image = NULL;
                k4a_image_t color_image = NULL;
                k4a_device_t device = kinect.device.handle();
                k4a_calibration_t calibration = kinect.calibration;

                transformation = k4a_transformation_create(&calibration);

                switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
                {
                case K4A_WAIT_RESULT_SUCCEEDED:
                    break;
                case K4A_WAIT_RESULT_TIMEOUT:
                    printf("Timed out waiting for a capture\n");
                case K4A_WAIT_RESULT_FAILED:
                    printf("Failed to read a capture\n");
                }
                depth_image = k4a_capture_get_depth_image(capture);
                if (depth_image == 0)
                {
                    printf("Failed to get depth image from capture\n");
                }
                std::cout << "depth" << std::endl;
                color_image = k4a_capture_get_color_image(capture);
                if (color_image == 0)
                {
                    printf("Failed to get color image from capture\n");

                }
                std::cout << "color" << std::endl;
#ifdef _WIN32
                file_name = output_dir + ".ply";
#else
                file_name = output_dir + "/depth_to_color.ply";
#endif
                thread th1(getPointCloud, transformation, depth_image, color_image, file_name);
                th1.detach();
                //if (difftime(timer, join_timer) == 20) {
                //    th1.join();
                //    join_timer = timer;
                //}
                std::cout << "point" << std::endl;
                //async(getPointCloud, transformation, depth_image, color_image, file_name);
                old_timer = timer;
                i++;

                cout << "Main " << matching(file_name) << endl;
                
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