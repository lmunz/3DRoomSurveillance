#include <iostream>
#include "opencv2/core/utility.hpp"
#include <string>
#include "open3d/Open3D.h"
#include "open3d/geometry/PointCloud.h"
#include "open3d/geometry/TriangleMesh.h"
#include <thread>
#include <stdlib.h>

using namespace std;
using namespace cv;
char* argv[3];
using namespace open3d;


int matching(int z)
{
    {

        open3d::geometry::PointCloud pcd1;
        open3d::geometry::PointCloud pcd2;

        open3d::io::ReadPointCloud(argv[0], pcd1);
        //open3d::io::ReadPointCloud("E:\\resources\\withoutFaces\\four_objects.ply", pcd2);

        open3d::io::ReadPointCloud(argv[z], pcd2);


        auto dis_pcd1_pcd2 = pcd1.ComputePointCloudDistance(pcd2);

        //std::cout << "Value 1: " << dis_pcd1_pcd2[0] << std::endl;
        //std::cout << "Value 2: " << dis_pcd1_pcd2[1] << std::endl;

        if (dis_pcd1_pcd2[0] == 0 && dis_pcd1_pcd2[1] == 0) {
            std::cout << "   Keine Veraenderungen in Thread" + z << std::endl;
            std::cout << "\n" << std::endl;
            return 0;
        }
        else {
            std::cout << "   Veraenderungen in Thread" + z << std::endl;
            std::cout << "\n" << std::endl;
            return 1;
        }
    }
}

int main() {
    
    argv[0] = "E:\\resources\\depth_three_objects.ply";
    argv[1] = "E:\\resources\\depth_three_objects.ply";
    argv[2] = "E:\\resources\\depth_no_weight.ply";

    thread th1(matching, 1);
    //Sleep(10000);
    thread th2(matching, 2);

    th1.join();
    th2.join();



    return 0;
}