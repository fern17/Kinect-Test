

#include <cv.h>
//Returns the index of the point at position (i,j) of the matrix M.
unsigned int getPointIndex(unsigned int i, unsigned int j, unsigned int cols){
			return (i*cols)+j;
}

