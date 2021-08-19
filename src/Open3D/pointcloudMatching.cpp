#include <iostream>
#include "opencv2/core/utility.hpp"
#include <string>
#include "open3d/Open3D.h"
#include "open3d/geometry/PointCloud.h"
#include "open3d/geometry/TriangleMesh.h"

using namespace std;
using namespace cv;
using namespace open3d;

int main(int argc, char* argv[]) {
    open3d::geometry::PointCloud pcd1;
    open3d::geometry::PointCloud pcd2;

    open3d::io::ReadPointCloud("C:\\Users\\merle\\Documents\\capturePLY\\save\\scenes\\four_objects\\depth_to_color.ply", pcd1);
    //open3d::io::ReadPointCloud("C:\\Users\\merle\\Documents\\capturePLY\\save\\scenes\\four_objects\\depth_to_color.ply", pcd2);
    open3d::io::ReadPointCloud("C:\\Users\\merle\\Documents\\capturePLY\\save\\scenes\\without_bunny\\depth_to_color.ply", pcd2);

    const Eigen::Vector3d orientationPoint = Eigen::Vector3d(0.0, 0.0, -1.0);

    pcd1.EstimateNormals();
    pcd1.NormalizeNormals();
    pcd1.OrientNormalsToAlignWithDirection(orientationPoint);

    pcd2.EstimateNormals();
    pcd2.NormalizeNormals();
    pcd2.OrientNormalsToAlignWithDirection(orientationPoint);


    if (pcd1.HasNormals()) {
        std::cout << "True Normals, PCD1" << std::endl;
    }
    else {
        std::cout << "False Normals, PCD1" << std::endl;
    }

    if (pcd2.HasNormals()) {
        std::cout << "True Normals, PCD2" << std::endl;
    }
    else {
        std::cout << "False Normals, PCD2" << std::endl;
    }

    auto dis_pcd1_pcd2 = pcd1.ComputePointCloudDistance(pcd2);

    std::cout << "Value 1: " << dis_pcd1_pcd2[0] << std::endl;
    std::cout << "Value 2: " << dis_pcd1_pcd2[1] << std::endl;

    if (dis_pcd1_pcd2[0] == 0 && dis_pcd1_pcd2[1] == 0) {
        std::cout << "Keine Veraenderungen" << std::endl;
    }
    else {
        std::cout << "Veraenderungen" << std::endl;
    }

    return 0;
}