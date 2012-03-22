#include "Triangle.h"
#include <cxcore.h>
#include <cv.h>

void Triangle::print(ofstream &out){
    out<<"ID="<<id<<" ";
    out<<"("<<p1.x<<','<<p1.y<<','<<p1.z<<')';
    out<<"; ("<<p2.x<<','<<p2.y<<','<<p2.z<<')';
    out<<"; ("<<p3.x<<','<<p3.y<<','<<p3.z<<')';
    out<<" ["<<ady1<<','<<ady2<<','<<ady3<<']';
    out<<"NORM=("<<normal.x<<','<<normal.y<<','<<normal.z<<')';
    out<<std::endl;
}

void Triangle::calculateNormal(){
    Point3_<int> v1 = p3-p1;
    Point3_<int> v2 = p2-p1;
    normal = v1.cross(v2);
}
