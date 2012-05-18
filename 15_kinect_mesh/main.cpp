
#include <libfreenect.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <time.h>
#include <stdio.h>
#include "Mutex.cpp"
#include "MyFreenectDevice.h"
#include "locateClosest.cpp"
#include "startFollow.cpp"
#include "Triangle.h"
#include "generateTriangles.cpp"
#include "initializeTriangles.cpp"
#include <GL/glut.h>

using namespace cv;
using namespace std;

double diffclock(clock_t clock1,clock_t clock2){
		double diffticks = clock1 - clock2;
		return (diffticks*1000)/CLOCKS_PER_SEC;
}


int main(int argc, char **argv) {
    bool die = false;    	
    std::vector<Triangle> triangles;

    string filename("snapshot");
    string suffix(".png");
    int i_snap(0);

    Mat depthMat(Size(640,480),CV_16UC1);
    Mat depthf  (Size(640,480),CV_8UC1);
    Mat rgbMat(Size(640,480),CV_8UC3,Scalar(0));

    Freenect::Freenect freenect;
    MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

    namedWindow("rgb",CV_WINDOW_AUTOSIZE);
    namedWindow("depth",CV_WINDOW_AUTOSIZE);
    device.startVideo();
    device.startDepth();
    
    //initialize triangles vector size
    device.getVideo(rgbMat);
    device.getDepth(depthMat);
    depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);
    initializeTriangles(depthf,triangles);
    
    bool iniciarLimpieza = false;
    bool iniciarSeguimiento = false;
    bool generarTriangulos = true;

    int iter = 0;
    time_t inicio;
    //time_t fin;
    time(&inicio);
    
    while (!die) {
        device.getVideo(rgbMat);
        device.getDepth(depthMat);
        
        depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);
        if(iniciarLimpieza 		== true) 	startCleaning(depthf);
        if(iniciarSeguimiento 	== true) 	startFollow(depthf, rgbMat);
        
        cv::imshow("rgb", rgbMat);
        cv::imshow("depth",depthf);

        char k = cvWaitKey(5);
        if( k == 27 ){
            cvDestroyWindow("rgb");
            cvDestroyWindow("depth");
            break;
        }

        if(k == 8 ) { //backspace
            std::cout<<"saving"<<std::endl;
            std::ostringstream file;
            file << filename << i_snap << suffix;
            cv::imwrite(file.str(),depthf);
            std::ostringstream file2;
            file2 << filename << "rgb" << i_snap << suffix;
            cv::imwrite(file2.str(),rgbMat);
            std::stringstream ss;
            ss<<i_snap;
            std::string fname;
            ss>>fname;
            fname += ".txt";
            fname = "out" + fname;
            std::ofstream archi(fname.c_str());
            if(archi.is_open()){
                for(int i = 0; i < depthf.rows; i++){
                    for(int j = 0;j < depthf.cols; j++)
                        archi<<depthf.at<unsigned int>(i,j)/1000000<<' ';
                    archi<<'\n';
                }
            }
            i_snap++;
        }
      
        if(generarTriangulos == true){
            generateTriangles(depthf,triangles);
        }
      
        if(k == 116){// tecla t 
            ofstream out("triangles.txt");
            if(out.is_open()){
                for(unsigned int i = 0; i < triangles.size(); i++){
                    triangles[i].print(out);
                }
            }
        }

        /*time(&fin);
        if(difftime(fin,inicio) == 1)
            std::cout<<iter<<std::endl;*/
        //if(iter > 10) break;
        
        
        iter++;
    }
    std::cout<<iter;
    device.stopVideo();
    device.stopDepth();
    return 0;
}
