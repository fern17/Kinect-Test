#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include <cv.h>
#include <cxcore.h>
#include <iostream>
#include <fstream>

class Triangle{
  public:
    int id;
		Point3_<int> p1;
		unsigned int p1idx;
		Point3_<int> p2;
		unsigned int p2idx;
		Point3_<int> p3;
		unsigned int p3idx;
		Point3_<int> normal;
		int ady1;
		int ady2;
		int ady3;
		Triangle() : ady1(-1) , ady2(-1) , ady3(-1){}
		void print(ofstream &out);
		void calculateNormal();
};
#endif
