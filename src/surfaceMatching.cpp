#include "opencv2/surface_matching.hpp"
#include <iostream>
#include "opencv2/surface_matching/ppf_helpers.hpp"
#include "opencv2/core/utility.hpp"

using namespace std;
using namespace cv;
using namespace ppf_match_3d;
Mat pc;
int i = 0;
int64 tick1, tick2;
vector<Pose3DPtr> matchingResult;
ppf_match_3d::PPF3DDetector detector(0.025, 0.05);


static void help(const string& errorMessage)
{
    cout << "Program init error : " << errorMessage << endl;
    cout << "\nUsage : ppf_matching [input model file] [input scene file]" << endl;
    cout << "\nPlease start again with new parameters" << endl;
}

void modellTraining()
{
    cout << "Training..." << endl;
    detector.trainModel(pc);
}

void loadModel(int argc, char** argv) {
    string modelFileName = (string)argv[1];

    pc = loadPLYSimple(modelFileName.c_str(), 1);

}
void matching(int argc, char** argv) {
    // welcome message
    //cout << "****************************************************" << endl;
    //cout << "* Surface Matching demonstration : demonstrates the use of surface matching"
    //    " using point pair features." << endl;
    //cout << "* The sample loads a model and a scene, where the model lies in a different"
    //    " pose than the training.\n* It then trains the model and searches for it in the"
    //    " input scene. The detected poses are further refined by ICP\n* and printed to the "
    //    " standard output." << endl;
    //cout << "****************************************************" << endl;

    //if (argc < 3)
    //{
    //    help("Not enough input arguments");
    //    exit(1);
    //}

#if (defined __x86_64__ || defined _M_X64)
    cout << "Running on 64 bits" << endl;
#else
    cout << "Running on 32 bits" << endl;
#endif

#ifdef _OPENMP
    cout << "Running with OpenMP" << endl;
#else
    cout << "Running without OpenMP and without TBB" << endl;
#endif

    string sceneFileName = (string)argv[2];

    Mat pcTest = loadPLYSimple(sceneFileName.c_str(), 1);
    //cout << "Training..." << endl;
    //tick1 = cv::getTickCount();
    //ppf_match_3d::PPF3DDetector detector(0.025, 0.05);
    //detector.trainModel(pc);
    //tick2 = cv::getTickCount();
    //cout << endl << "Training complete in "
    //    << (double)(tick2 - tick1) / cv::getTickFrequency()
    //    << " sec" << endl << "Loading model..." << endl;
   

    // Match the model to the scene and get the pose
    cout << endl << "Starting matching..." << endl;
    vector<Pose3DPtr> results;
    tick1 = cv::getTickCount();
    detector.match(pcTest, results, 1.0 / 40.0, 0.05);
    tick2 = cv::getTickCount();
    cout << endl << "PPF Elapsed Time " <<
        (tick2 - tick1) / cv::getTickFrequency() << " sec" << endl;

    // Get only first N results
    int N = 2; 
    vector<Pose3DPtr> resultsSub(results.begin(), results.begin() + N);

    // Create an instance of ICP
    ICP icp(100, 0.005f, 2.5f, 8);
    int64 t1 = cv::getTickCount();

    // Register for all selected poses
    cout << endl << "Performing ICP on " << N << " poses..." << endl;
    icp.registerModelToScene(pc, pcTest, resultsSub);
    int64 t2 = cv::getTickCount();

    cout << endl << "ICP Elapsed Time " <<
        (t2 - t1) / cv::getTickFrequency() << " sec" << endl;

    cout << "Poses: " << endl;
    // debug first five poses
    for (size_t i = 0; i < resultsSub.size(); i++)
    {
        Pose3DPtr result = resultsSub[i];
        cout << "Pose Result " << i << endl;
        result->printPose();

        if (i == 0)
        {
            Mat pctemp = pc;
            Mat pct = transformPCPose(pctemp, result->pose);
            //writePLY(pct, "para6700PCTrans.ply");
        }

    }
    matchingResult = resultsSub;
}


int matchingausfuehren(int argc, char** argv) {

    matching(argc, argv);

    for (size_t i = 0; i < matchingResult.size(); i++) {

        cout << matchingResult[i]->modelIndex << endl;
    }
    if (matchingResult[0]->modelIndex > 0) {
        return 1;
    }
    else return 0;
}

int main(int argc, char** argv)
{


    argv[1] = "E:\\resources\\parasaurolophus_6700.ply";
    argv[2] = "E:\\resources\\rs22_proc2.ply";
    loadModel(argc, argv);
    modellTraining();

    int temp1 = matchingausfuehren(argc, argv);
 
    argv[2] = "E:\\resources\\rs22_proc2.ply";


    int temp2 = matchingausfuehren(argc, argv);

    if (temp1 == temp2) {
        cout << "Keine Veraenderung" << endl;
    }
    else
        cout << "OMG eine Veraenderung" << endl;


    return 0;
}
