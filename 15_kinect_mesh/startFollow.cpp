#ifndef __STARTFOLLOW_CPP__
#define __STARTFOLLOW_CPP__

#include <cv.h>
#include "locateClosest.cpp"
#include <iostream>

void startCleaning(Mat &depthf);

//Detects hands and starts to follow. Needs callibration between the rgb and
//the depth camera. 
void startFollow(Mat & depthf, Mat & rgbMat){
    startCleaning(depthf); //NOTE: Needs to call startCleaning before
    int delta = 25;
    int followx;
    int followy;
    locateClosest(depthf,followx,followy); //detecta lo mas cercano
  //	std::cout<<followx<<' '<<followy<<'\n';
    int inix = 0;
    int iniy = 0;
    int finx = rgbMat.cols;
    int finy = rgbMat.rows;
    if(followx-delta > inix) inix = followx-delta;
    if(followx+delta < finx) finx = followx+delta;
    if(followy-delta > iniy) iniy = followy-delta;
    if(followy+delta < finy) finy = followy+delta;
    std::cout<<inix<<' '<<finx<<' '<<iniy<<' '<<finy<<std::endl;		
    for(int i = iniy; i < finy ; i++){
        for(int j = inix; j < finx; j++){
            rgbMat.at<unsigned int>(i,j) = 25; //cambio color
        }
    }
}

//Cleans the depth map, leaving only a small place to render depth data
void startCleaning(Mat &depthf){
    int limpieza =  1700000000;
    for(int i = 0; i < depthf.rows; i++){
        for(int j = 0;j < depthf.cols; j++){
            unsigned int dep = depthf.at<unsigned int>(i,j);
            if(dep >= limpieza){
                depthf.at<unsigned int>(i,j) = -1;
            }
        }
    }
}
#endif
