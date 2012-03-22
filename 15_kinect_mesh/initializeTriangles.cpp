#ifndef __INITIALIZE_TRIANGLES_CPP__
#define __INITIALIZE_TRIANGLES_CPP__
#include <cv.h>
#include <vector>
#include "Triangle.h"

//reserve space and generate adjacencies
void initializeTriangles(Mat &M, std::vector<Triangle> &triangles){
		int cols = M.cols;
		int rows = M.rows;
		int fullsize = (rows-1)*(cols-1)*2;
		triangles.resize(fullsize);
		for(int i = 0; i < fullsize; i++){
				triangles[i].id = i;
		}
		//generates neighbors
		//vertical triangles
		for(int i = 0; i < fullsize/2; i++){
				if(i-cols > -1) 				triangles[i].ady1 = i-cols;
				if(i+fullsize/2 < fullsize) 	triangles[i].ady2 = i + fullsize/2;
				if(i+fullsize/2 + 1 < fullsize) triangles[i].ady3 = i + fullsize/2 + 1;
		}
		//horizontal triangles
		for(int i = fullsize/2; i < fullsize; i++){
				if(i - fullsize/2 - 1 > -1) 	triangles[i].ady1 = i - fullsize/2 - 1;
				if(i - fullsize/2 > -1) 	triangles[i].ady2 = i - fullsize/2;
				if(i + cols + fullsize/2 < fullsize) triangles[i].ady3 = i + cols + fullsize/2;
		}
}
#endif
