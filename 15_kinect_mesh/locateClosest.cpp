#ifndef __LOCATE_CLOSEST_CPP
#define __LOCATE_CLOSEST_CPP

#include <cv.h>
#include <cxcore.h>
#include <iostream>

void locateClosest(Mat mat, int &x, int &y ){
    unsigned int dmax = 1700000000;
    x = 0;
    y = 0;
    for(int i = 0; i < mat.rows; i++){
        for(int j = 0; j < mat.cols; j++){
            unsigned int value = mat.at<unsigned int>(i,j);
            //std::cout<<value<<' ';
            if(value >= 1000000000){
                if(value <= 1700000000){
                    //std::cout<<value<<' '<<dmax<<std::endl;
                    if(value < dmax){
                    //std::cout<<value<<std::endl;
                        x = j;
                        y = i;
                        dmax = value;
                    }
                }
            }
        }
        //std::cout<<'\n';
    }
  //	std::cout<<dmax<<std::endl;

}
#endif
