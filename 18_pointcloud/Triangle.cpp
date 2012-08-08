#include "Triangle.h"

Triangle::Triangle(){
    p1 = -1;
    p2 = -1;
    p3 = -1;
}

Triangle::Triangle(GLubyte _p1, GLubyte _p2, GLubyte _p3){
   calculateNormal(); 
}
void Triangle::calculateNormal(Point3_<uint32> p1, Point3_<uint32> p2, Point3_<uint32> p3){
    
}

