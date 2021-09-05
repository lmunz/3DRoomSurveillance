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
#include "resource.h"
#include <SDKDDKVer.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <k4a/k4a.h>
#include <k4arecord/record.h>
#include <k4arecord/playback.h>
#include <string>
#include "transformation_helpers.h"

using namespace std;

string pcArray[2];
int result = 6;
bool saved = false;

//typedef struct
//{
//    uint32_t biSize;
//    uint32_t biWidth;
//    uint32_t biHeight;
//    uint16_t biPlanes;
//    uint16_t biBitCount;
//    uint32_t biCompression;
//    uint32_t biSizeImage;
//    uint32_t biXPelsPerMeter;
//    uint32_t biYPelsPerMeter;
//    uint32_t biClrUsed;
//    uint32_t biClrImportant;
//} BITMAPINFOHEADER;

void fill_bitmap_header(uint32_t width, uint32_t height, BITMAPINFOHEADER* out);
void fill_bitmap_header(uint32_t width, uint32_t height, BITMAPINFOHEADER* out)
{
    out->biSize = sizeof(BITMAPINFOHEADER);
    out->biWidth = width;
    out->biHeight = height;
    out->biPlanes = 1;
    out->biBitCount = 16;
    out->biCompression = FOURCC("YUY2");
    out->biSizeImage = sizeof(uint16_t) * width * height;
    out->biXPelsPerMeter = 0;
    out->biYPelsPerMeter = 0;
    out->biClrUsed = 0;
    out->biClrImportant = 0;
}

//// Global Objects
WNDCLASSEX HostWindowClass; /// Our Host Window Class Object
MSG loop_message; /// Loop Message for Host Window
HINSTANCE hInstance = GetModuleHandle(NULL); /// Application Image Base Address
HWND cpphwin_hwnd; /// Host Window Handle
HWND wpf_hwnd; /// WPF Wrapper Handle
#define HWIN_TITLE L"Camera Application"
TCHAR greeting[] = _T("Hello, Windows desktop!");

//// Global Configs
const wchar_t cpphwinCN[] = L"CppMAppHostWinClass"; /// Host Window Class Name
bool isHWindowRunning = false; /// Host Window Running State


//// Host Window Callback, NOTE :Define This Before Your Entrypoint Function
LRESULT CALLBACK HostWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // TODO: Zeichencode, der hdc verwendet, hier einfügen...
        TextOut(hdc,5, 5,greeting, _tcslen(greeting));
        EndPaint(hwnd, &ps);
    }
    break;
    case WM_DESTROY:
        isHWindowRunning = false;
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static int createHostWindow() {
    /// Creating Icon Object From Resources, Don't forget to include resource.h!
    HICON app_icon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_APPICON));

    /// Defining Our Host Window Class
    HostWindowClass.cbSize = sizeof(WNDCLASSEX); HostWindowClass.lpfnWndProc = HostWindowProc;
    HostWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    HostWindowClass.cbClsExtra = 0; HostWindowClass.style = 0;
    HostWindowClass.cbWndExtra = 0;    HostWindowClass.hInstance = hInstance;
    HostWindowClass.hIcon = app_icon; HostWindowClass.hIconSm = app_icon;
    HostWindowClass.lpszClassName = cpphwinCN; HostWindowClass.lpszMenuName = NULL;

    //// Register Window
    if (!RegisterClassEx(&HostWindowClass))
    {
        cout << "Error, Code :" << GetLastError() << endl;
        getchar(); return 0;
    }

    /// Creating Unmanaged Host Window
    cpphwin_hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        cpphwinCN,
        HWIN_TITLE,
        //GetSTR_Res(APPDATA_HWINDOW_NAME),
        WS_THICKFRAME | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 500,
        NULL, NULL, hInstance, NULL);

    /// Check if How Window is valid
    if (cpphwin_hwnd == NULL)
    {
        cout << "Error, Code :" << GetLastError() << endl;
        getchar(); return 0;
    }
}

static void showHostWindow() {
    /// Centering Host Window
    RECT window_r; RECT desktop_r;
    GetWindowRect(cpphwin_hwnd, &window_r); GetWindowRect(GetDesktopWindow(), &desktop_r);
    int xPos = (desktop_r.right - (window_r.right - window_r.left)) / 2;
    int yPos = (desktop_r.bottom - (window_r.bottom - window_r.top)) / 2;

    /// Set Window Position
    ::SetWindowPos(cpphwin_hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    /// Display Window
    ShowWindow(cpphwin_hwnd, SW_SHOW);
    UpdateWindow(cpphwin_hwnd);
    BringWindowToTop(cpphwin_hwnd);
    isHWindowRunning = true;

    /// Adding Message Loop
    while (GetMessage(&loop_message, NULL, 0, 0) > 0 && isHWindowRunning)
    {
        TranslateMessage(&loop_message);
        DispatchMessage(&loop_message);
    }
}

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

    saved = true;
    return returnCode;
}



static void recording(k4a_device_t device, k4a_device_configuration_t device_config, k4a_calibration_t sensor_calibration, const char* filename) {
    k4a_record_t recording;
    k4a_record_create(filename, device, device_config, &recording);

    uint32_t depth_width = (uint32_t)sensor_calibration.depth_camera_calibration.resolution_width;
    uint32_t depth_height = (uint32_t)sensor_calibration.depth_camera_calibration.resolution_height;

    BITMAPINFOHEADER codec_header;
    fill_bitmap_header(depth_width, depth_height, &codec_header);

    k4a_record_video_settings_t video_settings;
    video_settings.width = depth_width;
    video_settings.height = depth_height;
    video_settings.frame_rate = 30;

    k4a_record_add_custom_video_track(recording,
        "PROCESSED_DEPTH",
        "V_MS/VFW/FOURCC",
        (uint8_t*)(&codec_header),
        sizeof(codec_header),
        &video_settings);

    k4a_record_write_header(recording);

    for (int frame = 0; frame < 100; frame++)
    {
        k4a_capture_t capture;
        k4a_wait_result_t get_capture_result = k4a_device_get_capture(device, &capture, K4A_WAIT_INFINITE);
        if (get_capture_result == K4A_WAIT_RESULT_SUCCEEDED)
        {
            // Write the capture to the built-in tracks
            k4a_record_write_capture(recording, capture);

            // Get the depth image from the capture so we can write a processed copy to our custom track.
            k4a_image_t depth_image = k4a_capture_get_depth_image(capture);
            if (depth_image)
            {
                // The YUY2 image format is the same stride as the 16-bit depth image, so we can modify it in-place.
                uint8_t* depth_buffer = k4a_image_get_buffer(depth_image);
                size_t depth_buffer_size = k4a_image_get_size(depth_image);
                for (size_t i = 0; i < depth_buffer_size; i += 2)
                {
                    // Convert the depth value (16-bit, in millimeters) to the YUY2 color format.
                    // The YUY2 format should be playable in video players such as VLC.
                    uint16_t depth = (uint16_t)(depth_buffer[i + 1] << 8 | depth_buffer[i]);
                    // Clamp the depth range to ~1 meter and scale it to fit in the Y channel of the image (8-bits).
                    if (depth > 0x3FF)
                    {
                        depth_buffer[i] = 0xFF;
                    }
                    else
                    {
                        depth_buffer[i] = (uint8_t)(depth >> 2);
                    }
                    // Set the U/V channel to 128 (i.e. grayscale).
                    depth_buffer[i + 1] = 128;
                }

                (k4a_record_write_custom_track_data(recording,
                    "PROCESSED_DEPTH",
                    k4a_image_get_device_timestamp_usec(depth_image),
                    depth_buffer,
                    (uint32_t)depth_buffer_size));

                k4a_image_release(depth_image);
            }

            k4a_capture_release(capture);
        }
        else if (get_capture_result == K4A_WAIT_RESULT_TIMEOUT)
        {
            // TIMEOUT should never be returned when K4A_WAIT_INFINITE is set.
            printf("k4a_device_get_capture() timed out!\n");
            break;
        }
        else
        {
            printf("k4a_device_get_capture() returned error: %d\n", get_capture_result);
            break;
        }
    }

    k4a_record_flush(recording);
    k4a_record_close(recording);
}

static int matching(string file_pc) {

    int matchResult = 6;
    int size = sizeof(pcArray);

    if (pcArray[0] == "") {
        cout << "NULL" << endl;
        pcArray[0] = file_pc;
        matchResult = 0;
    }
    else if (pcArray[1] == "") {
        pcArray[1] = file_pc;
        cout << "FIRST" << endl;

        matchResult = system(("C:\\Users\\merle\\Documents\\Projekte\\3DRoomSurveillance\\executable\\PointCloudMatching.exe \"" + pcArray[0] + "\" HelloWorld \"" + pcArray[1]).c_str());
    }

    switch (matchResult) {
    case 0:
        result = 0;
        pcArray[1] = "";
        return result;
        break;
    case 1:
        result = 1;
        pcArray[0] = pcArray[1];
        pcArray[1] = "";
        return result;
        break;
    case 2:
        result = 1;
        pcArray[0] = pcArray[1];
        pcArray[1] = "";
        return result;
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
    //createHostWindow();
    //thread t0(showHostWindow);
    //t0.detach();

    try { 
        std::string file_name = "";
        string recording_filename = "D:\\azureKinect\\capture";
        string mkv = ".mkv";
        string filename = "C:\\Users\\merle\\Documents\\Projekte\\3DRoomSurveillance\\pointClouds\\pc"; //".\\pointClouds\\pc";
        kinect kinect;
        int i = 1;
        bool working = false;

        time_t timer;
        time_t old_timer;

        time(&old_timer);

        while (true) {
            // Update
            kinect.update();

            // Draw
            kinect.draw();

            // Show
            kinect.show();

            time(&timer);

            

            if (difftime(timer, old_timer) == 30) {

                string output_dir = filename + std::to_string(i);

                string recording_output_dir = recording_filename + std::to_string(i) + ".mp4";

                const char* charfile = recording_output_dir.c_str();

                int returnCode = 1;

                const int32_t TIMEOUT_IN_MS = 10000;
                
                k4a_transformation_t transformation = NULL;
                k4a_capture_t capture = NULL;
                k4a_image_t depth_image = NULL;
                k4a_image_t color_image = NULL;
                k4a_device_t device = kinect.device.handle();
                k4a_device_configuration_t device_config = kinect.device_configuration;
                k4a_calibration_t calibration = kinect.calibration;

                thread t_recording(recording, device, device_config, calibration, charfile);
                t_recording.detach();

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
                color_image = k4a_capture_get_color_image(capture);
                if (color_image == 0)
                {
                    printf("Failed to get color image from capture\n");

                }
#ifdef _WIN32
                file_name = output_dir + ".ply";
#else
                file_name = output_dir + "/depth_to_color.ply";
#endif
                thread th1(getPointCloud, transformation, depth_image, color_image, file_name);
                th1.detach();

                old_timer = timer;
                
                if (i >= 10) {
                    i = 1;
                }
                else {
                    i++;
                }
            }

            if (saved) {
                cout << result << endl;
                thread t2(matching, file_name);
                t2.detach();
                saved = false;
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