#ifndef __GENERATE_TRIANGLES_CPP__
#define __GENERATE_TRIANGLES_CPP__

#include <cv.h>
#include <vector>
#include "Triangle.h"

unsigned int getPointIndex(unsigned int i, unsigned int j, unsigned int cols);

//calculate triangles vertices
void generateTriangles(Mat &M, std::vector<Triangle> &triangles){
    int rows = M.rows;
    int cols = M.cols;
    //triangles.clear();
    unsigned int triangleid = 0;	
    for(int i = 0; i < rows-1; i++){
        for(int j = 0; j < cols-1; j++){
            Triangle *t = &triangles[triangleid];
            t->p1idx = getPointIndex(i, j, M.cols);
            t->p2idx = getPointIndex(i, j+1, M.cols);
            t->p3idx = getPointIndex(i+1, j+1, M.cols);
            t->p1 = Point3_<int>(i,j,M.at<unsigned int>(i,j));
            t->p2 = Point3_<int>(i,j+1,M.at<unsigned int>(i,j+1));
            t->p3 = Point3_<int>(i+1,j+1,M.at<unsigned int>(i+1,j+1));
            t->calculateNormal();
            //t->id = triangleid;
            triangleid++;
            //triangles.push_back(t);
        }
    }
    for(int i = 0; i < rows-1; i++){
        for(int j = 0; j < cols-1; j++){
            Triangle *t = &triangles[triangleid];
            t->p1idx = getPointIndex(i, j, M.cols);
            t->p2idx = getPointIndex(i+1, j+1, M.cols);
            t->p3idx = getPointIndex(i+1, j, M.cols);
            t->p1 = Point3_<int>(i,j,M.at<unsigned int>(i,j));
            t->p2 = Point3_<int>(i+1,j+1,M.at<unsigned int>(i+1,j+1));
            t->p3 = Point3_<int>(i+1,j,M.at<unsigned int>(i+1,j));
            t->calculateNormal();
            //t->id = triangleid;
            triangleid++;
            //triangles.push_back(t);
        }
    }
}
unsigned int getPointIndex(unsigned int i, unsigned int j, unsigned int cols){
			return (i*cols)+j;
}

#endif
