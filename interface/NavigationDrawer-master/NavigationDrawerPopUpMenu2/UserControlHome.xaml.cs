using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Azure.Kinect.Sensor;

namespace NavigationDrawerPopUpMenu2
{
    /// <summary>
    /// Interação lógica para UserControlHome.xam
    /// </summary>
    public partial class UserControlHome : UserControl
    {
        Device device = null;
        bool running = true;
        bool depthmapActivated = false;

        public UserControlHome()
        {
            InitializeComponent();

            CameraImage();
        }

        private void BtnDepthmap_Click(object sender, RoutedEventArgs e)
        {
            depthmapActivated = depthmapActivated == true ? false : true;
        }

        private async void CameraImage()
        {
            try {
                device = Device.Open();

                DeviceConfiguration config = new DeviceConfiguration();

                config.ColorFormat = ImageFormat.ColorBGRA32;
                config.ColorResolution = ColorResolution.R1080p;
                config.DepthMode = DepthMode.WFOV_2x2Binned;
                config.SynchronizedImagesOnly = true;
                config.CameraFPS = FPS.FPS30;

                device.StartCameras(config);

                int colorWidth = device.GetCalibration().ColorCameraCalibration.ResolutionWidth;
                int colorHeight = device.GetCalibration().ColorCameraCalibration.ResolutionHeight;

                using (Microsoft.Azure.Kinect.Sensor.Image transformedDepth = new Microsoft.Azure.Kinect.Sensor.Image(ImageFormat.Depth16, colorWidth, colorHeight))
                using (Microsoft.Azure.Kinect.Sensor.Image outputColorImage = new Microsoft.Azure.Kinect.Sensor.Image(ImageFormat.ColorBGRA32, colorWidth, colorHeight))
                using (Transformation transform = device.GetCalibration().CreateTransformation())

                    while (running)
                    {
                        using (Capture capture = await Task.Run(() => { return this.device.GetCapture(); }).ConfigureAwait(true))
                        {


                            Task<BitmapSource> createInputColorBitmapTask = Task.Run(() =>
                            {
                                Microsoft.Azure.Kinect.Sensor.Image color = capture.Color;
                                BitmapSource source = BitmapSource.Create(color.WidthPixels, color.HeightPixels, 96, 96, PixelFormats.Bgra32, null, color.Memory.ToArray(), color.StrideBytes);

                                source.Freeze();
                                return source;
                            });
                            Task<BitmapSource> createOutputColorBitmapTask = Task.Run(() =>
                            {
                                transform.DepthImageToColorCamera(capture, transformedDepth);

                                Span<ushort> depthBuffer = transformedDepth.GetPixels<ushort>().Span;
                                Span<BGRA> colorBuffer = capture.Color.GetPixels<BGRA>().Span;
                                Span<BGRA> outputColorBuffer = outputColorImage.GetPixels<BGRA>().Span;

                                for (int i = 0; i < colorBuffer.Length; i++)
                                {
                                    outputColorBuffer[i] = colorBuffer[i];

                                    if (depthBuffer[i] == 0)
                                    {
                                        outputColorBuffer[i].R = 50;
                                        outputColorBuffer[i].G = 50;
                                        outputColorBuffer[i].B = 50;
                                    }
                                    else if (depthBuffer[i] > 10)
                                    {
                                        outputColorBuffer[i].R = 100;
                                        outputColorBuffer[i].G = 100;
                                        outputColorBuffer[i].B = 100;
                                    }
                                    else if (depthBuffer[i] > 100)
                                    {
                                        outputColorBuffer[i].R = 200;
                                        outputColorBuffer[i].G = 200;
                                        outputColorBuffer[i].B = 200;
                                    }
                                }

                                BitmapSource source = BitmapSource.Create(outputColorImage.WidthPixels, outputColorImage.HeightPixels, 96, 96, PixelFormats.Bgra32, null, outputColorImage.Memory.ToArray(), outputColorImage.StrideBytes);

                                source.Freeze();
                                return source;
                            });

                            BitmapSource inputColorBitmap = await createInputColorBitmapTask.ConfigureAwait(true);
                            BitmapSource outputColorBitmap = await createOutputColorBitmapTask.ConfigureAwait(true);

                            if (!depthmapActivated)
                            {
                                this.displayImg.Source = inputColorBitmap;
                            } else
                            {
                                this.displayImg.Source = outputColorBitmap;
                            }

                        }

                        //if (capture.Color != null)
                        //{

                        ////Working Depth and Color Image
                        //var color = capture.Color;
                        //var depth = capture.Depth;

                        //displayImg.Source = BitmapSource.Create(color.WidthPixels, color.HeightPixels, 96, 96, PixelFormats.Bgr32, null, color.Memory.ToArray(), color.StrideBytes);
                        //displayImg.Source = BitmapSource.Create(depth.WidthPixels, depth.HeightPixels, 96, 96, PixelFormats.Gray16, null, depth.Memory.ToArray(), depth.StrideBytes);
                        //}

                    }

                device.Dispose();
            } catch(Exception ex)
            {
                Console.WriteLine(ex);
            }
        }
    }
}
