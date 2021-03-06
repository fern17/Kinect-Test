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
//		void print(ofstream &out);
//		void calculateNormal();
    Triangle() : ady1(-1) , ady2(-1) , ady3(-1){}
    void print(ofstream &out){
        out<<"ID="<<id<<" ";
        out<<"("<<p1.x<<','<<p1.y<<','<<p1.z<<')';
        out<<"; ("<<p2.x<<','<<p2.y<<','<<p2.z<<')';
        out<<"; ("<<p3.x<<','<<p3.y<<','<<p3.z<<')';
        out<<" ["<<ady1<<','<<ady2<<','<<ady3<<']';
        out<<"NORM=("<<normal.x<<','<<normal.y<<','<<normal.z<<')';
        out<<std::endl;
    }

    void calculateNormal(){
        Point3_<int> v1 = p3-p1;
        Point3_<int> v2 = p2-p1;
        normal = v1.cross(v2);
    }
};
#endif
