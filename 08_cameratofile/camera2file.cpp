#include <cv.h>
#include <highgui.h>
main( int argc, char* argv[] ) {

    CvCapture* capture = NULL;
    capture = cvCreateCameraCapture(0);

	IplImage *frames = cvQueryFrame(capture);

	// get a frame size to be used by writer structure
	CvSize size = cvSize (
		(int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH),
		(int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT)
	);

	// declare writer structure
	// use FOURCC ( Four Character Code ) MJPG, the motion jpeg codec
	// output file is specified by first argument
	CvVideoWriter *writer = cvCreateVideoWriter(
		argv[1],
		CV_FOURCC('M','J','P','G'),
		30, // set fps
		size
	);
	//Create a new window
	cvNamedWindow( "Recording ...press ESC to stop !", CV_WINDOW_AUTOSIZE );
	// show capture in the window and record to a file
	// record until user hits ESC key
	while(1) {

		frames = cvQueryFrame( capture );
		if( !frames ) break;
		cvShowImage( "Recording ...press ESC to stop !", frames );
		cvWriteFrame( writer, frames );

		char c = cvWaitKey(33);
	if( c == 27 ) break;
	}
	cvReleaseVideoWriter( &writer );
	cvReleaseCapture ( &capture );
	cvDestroyWindow ( "Recording ...press ESC to stop !");

	return 0;

}