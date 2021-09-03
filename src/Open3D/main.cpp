#include <iostream>
#include "opencv2/core/utility.hpp"
#include <string>
#include "open3d/Open3D.h"
#include "open3d/geometry/PointCloud.h"
#include "open3d/geometry/TriangleMesh.h"
#include <thread>
#include <stdlib.h>
#include <vector>


using namespace std;
using namespace cv;
using namespace open3d;  


int matching(char* files[])
{
    {
        
        std::cout << "start des Threads" << std::endl;
        open3d::geometry::PointCloud pcd1;
        open3d::geometry::PointCloud pcd2;

        open3d::io::ReadPointCloud(files[1], pcd1);

        open3d::io::ReadPointCloud(files[2], pcd2);

        auto dis_pcd1_pcd2 = pcd1.ComputePointCloudDistance(pcd2);

        
        std::cout << "Value 1: " << dis_pcd1_pcd2[0] << std::endl;
        std::cout << "Value 2: " << dis_pcd1_pcd2[1] << std::endl;


        if (dis_pcd1_pcd2[0] == 0 && dis_pcd1_pcd2[1] == 0) {

            return 0;
        }
        else if (dis_pcd1_pcd2[0] < 2 || dis_pcd1_pcd2[1] <3) {

            return 1;
        }
        else {
            return 2;
        }
    }
}

std::vector<std::string> split(std::string const& input, std::string const& separator = " ")
{
    std::vector<std::string> result;
    std::string::size_type position, start = 0;

    while (std::string::npos != (position = input.find(separator, start)))
    {
        result.push_back(input.substr(start, position - start));
        start = position + separator.size();
    }

    result.push_back(input.substr(start));
    return result;
}

int main(int argc, char* argv[]) {

    cout << argc << endl;
    cout << argv[0] << endl;
    cout << "Argv 1 " << argv[1] << endl;
    cout << "Argv 2 " << argv[2] << endl;



    std::vector<std::string> v = split(argv[1], ".ply");

    std::string s;
    for (std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i)
        s += *i;

    //std::string str(splited.begin(), splited.end() - 1);

    //cout << "Split 1 " << splited[0] << " + " << splited[1] << endl;
    cout << "S " << s << endl;

    if (argc > 1) {
        return matching(argv);
    }

    return 3;
}