#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

int main(int argc, char *argv[])
{
  // Nothing but create a window
  cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE); 
  cvMoveWindow("mainWin", 100, 100);
  cvWaitKey(0);
  return 0;
}