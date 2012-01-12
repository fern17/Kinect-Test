#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
	bool die(false);
	string filename("snapshot");
	string suffix(".png");
	int i_snap(0);

	Mat depthMat(Size(640,480),CV_8UC3,Scalar(0));
    for(int i = 0; i < 100; i++){
		for(int j = 0; j < 30; j++){
			depthMat.at<int>(i,j) = 200;
		}
	}
				
	namedWindow("depth",CV_WINDOW_AUTOSIZE);

    while (!die) {
		for(int i = 0; i < depthMat.rows; i++){
				for(int j = 0;j < depthMat.cols; j++){
						std::cout<<depthMat.at<int>(i,j)<<' ';
				}
				std::cout<<"\n";
		}
		std::cout<<"-----------------\n";
        cv::imshow("depth",depthMat);
		char k = cvWaitKey(5);
		if( k == 27 ){
		    cvDestroyWindow("depth");
			break;
		}
		if( k == 8 ) { //backspace
			std::ostringstream file;
			file << filename << i_snap << suffix;
			cv::imwrite(file.str(),depthMat);
			i_snap++;
		}
    }
	return 0;
}
