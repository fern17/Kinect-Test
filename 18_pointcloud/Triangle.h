
#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

class Triangle{
    public:
        //Indices de los vertices que forman este triangulo
        GLubyte p1;
        GLubyte p2;
        Glubyte p3;
        float normal;
        Triangle();
        Triangle(GLubyte _p1, GLubyte _p2, GLubyte _p3) : p1(_p1), p2(_p2), p3(_p3);
        calculateNormal();
};
#endif
