#include <cv.h>
#include <highgui.h>


int main(int argc, char** argv) {

	// Crea una ventana llamada Original Image con un tamaño predeterminado.
	cvNamedWindow("Original Image",	CV_WINDOW_AUTOSIZE);

	// Crea la conexion con la Webcam.
	CvCapture* capture = cvCreateCameraCapture(0);

	// Variable donde se almazenara el frame sacado de la webcam.
	IplImage* originalImg;

	// Hago que el ancho del capture sea de 320px con eso la altura se adaptara.

	if(!cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,320))

		while(true) {
		// Pongo el frame capturado de “capture” dentro de la imagen originalImg.
			originalImg = cvQueryFrame(capture);
			if(!originalImg) break;

			// Hago que se vea el frame dentro de la ventana “Original Image”.
			cvShowImage("Original Image", originalImg);

			// Espero a que me pulsen el ESC para salir del bucle infinito.
			char c = cvWaitKey(10);
			if( c == 27 ) break;
		}

	// Libera la memoria utilizada por la variable capture.
	cvReleaseCapture(&capture);
	// Destruye la ventana “Original Image”.
	cvDestroyWindow("Original Image");

}
