#include <libfreenect.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <time.h>
#include <stdio.h>

using namespace cv;
using namespace std;

class Mutex {
public:
	Mutex() {
		pthread_mutex_init( &m_mutex, NULL );
	}
	void lock() {
		pthread_mutex_lock( &m_mutex );
	}
	void unlock() {
		pthread_mutex_unlock( &m_mutex );
	}
private:
	pthread_mutex_t m_mutex;
};

class MyFreenectDevice : public Freenect::FreenectDevice {
  public:
	MyFreenectDevice(freenect_context *_ctx, int _index) : Freenect::FreenectDevice(_ctx, _index), m_buffer_depth(FREENECT_DEPTH_11BIT),m_buffer_rgb(FREENECT_VIDEO_RGB), m_gamma(2048), m_new_rgb_frame(false), m_new_depth_frame(false),  depthMat(Size(640,480),CV_16UC1), rgbMat(Size(640,480),CV_8UC3,Scalar(0)), ownMat(Size(640,480),CV_8UC3,Scalar(0))
	{
		for( unsigned int i = 0 ; i < 2048 ; i++) {
			float v = i/2048.0;
			v = std::pow(v, 3)* 6;
			m_gamma[i] = v*6*256;
		}
	}
	// Do not call directly even in child
	void VideoCallback(void* _rgb, uint32_t timestamp) {
		//std::cout << "RGB callback" << std::endl;
		m_rgb_mutex.lock();
		uint8_t* rgb = static_cast<uint8_t*>(_rgb);
		rgbMat.data = rgb;
		m_new_rgb_frame = true;
		m_rgb_mutex.unlock();
	};
	// Do not call directly even in child
	void DepthCallback(void* _depth, uint32_t timestamp) {
		//std::cout << "Depth callback" << std::endl;
		m_depth_mutex.lock();
		uint16_t* depth = static_cast<uint16_t*>(_depth);
		depthMat.data = (uchar*) depth;
		m_new_depth_frame = true;
		m_depth_mutex.unlock();
	}

	bool getVideo(Mat& output) {
		m_rgb_mutex.lock();
		if(m_new_rgb_frame) {
			cv::cvtColor(rgbMat, output, CV_RGB2BGR);
			m_new_rgb_frame = false;
			m_rgb_mutex.unlock();
			return true;
		} else {
			m_rgb_mutex.unlock();
			return false;
		}
	}

	bool getDepth(Mat& output) {
			m_depth_mutex.lock();
			if(m_new_depth_frame) {
				depthMat.copyTo(output);
				m_new_depth_frame = false;
				m_depth_mutex.unlock();
				return true;
			} else {
				m_depth_mutex.unlock();
				return false;
			}
		}

  private:
	std::vector<uint8_t> m_buffer_depth;
	std::vector<uint8_t> m_buffer_rgb;
	std::vector<uint16_t> m_gamma;
	Mat depthMat;
	Mat rgbMat;
	Mat ownMat;
	Mutex m_rgb_mutex;
	Mutex m_depth_mutex;
	bool m_new_rgb_frame;
	bool m_new_depth_frame;
};

double diffclock(clock_t clock1,clock_t clock2){
		double diffticks = clock1 - clock2;
		return (diffticks*1000)/CLOCKS_PER_SEC;
}



void locateClosest(Mat mat, int &x, int &y ){
	unsigned int dmax = 1700000000;
	x = 0;
	y = 0;
	for(int i = 0; i < mat.rows; i++){
			for(int j = 0; j < mat.cols; j++){
				unsigned int value = mat.at<unsigned int>(i,j);
//				std::cout<<value<<' ';
				if(value >= 1000000000){
					if(value <= 1700000000){
//						std::cout<<value<<' '<<dmax<<std::endl;
						if(value < dmax){
					//std::cout<<value<<std::endl;
							x = j;
							y = i;
							dmax = value;
						}
					}
				}
			}
			//std::cout<<'\n';
	}
//	std::cout<<dmax<<std::endl;

}

typedef struct Triangle{
		int id;
		Point3_<int> p1;
		unsigned int p1idx;
		Point3_<int> p2;
		unsigned int p2idx;
		Point3_<int> p3;
		unsigned int p3idx;
		int ady1;
		int ady2;
		int ady3;
		Triangle() : ady1(-1) , ady2(-1) , ady3(-1){}
		void print(ofstream &out){
				out<<"ID="<<id<<" ";
				out<<"("<<p1.x<<','<<p1.y<<','<<p1.z<<')';
				out<<"; ("<<p2.x<<','<<p2.y<<','<<p2.z<<')';
				out<<"; ("<<p3.x<<','<<p3.y<<','<<p3.z<<')';
				out<<" ["<<ady1<<','<<ady2<<','<<ady3<<']';
				out<<std::endl;
		}
}Triangle;

//Returns the index of the point at position (i,j) of the matrix M.
unsigned int getPointIndex(unsigned int i, unsigned int j, Mat &M){
		unsigned int rows = M.rows;
		unsigned int cols = M.cols;
		return (i*cols)+j;
}

void initalizeTriangles(Mat &M, std::vector<Triangle> &triangles){
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

void generateTriangles(Mat &M, std::vector<Triangle> &triangles){
	int rows = M.rows;
	int cols = M.cols;
	//triangles.clear();
    unsigned int triangleid = 0;	
	for(int i = 0; i < rows-1; i++){
			for(int j = 0; j < cols-1; j++){
				Triangle *t = &triangles[triangleid];
				t->p1idx = getPointIndex(i,j,M);
				t->p2idx = getPointIndex(i,j+1,M);
				t->p3idx = getPointIndex(i+1,j+1,M);
				t->p1 = Point3_<int>(i,j,M.at<unsigned int>(i,j));
				t->p2 = Point3_<int>(i,j+1,M.at<unsigned int>(i,j+1));
				t->p3 = Point3_<int>(i+1,j+1,M.at<unsigned int>(i+1,j+1));
				//t->id = triangleid;
				triangleid++;
				//triangles.push_back(t);
			}
	}
	for(int i = 0; i < rows-1; i++){
			for(int j = 0; j < cols-1; j++){
				Triangle *t = &triangles[triangleid];
				t->p1idx = getPointIndex(i,j,M);
				t->p2idx = getPointIndex(i+1,j+1,M);
				t->p3idx = getPointIndex(i+1,j,M);
				t->p1 = Point3_<int>(i,j,M.at<unsigned int>(i,j));
				t->p2 = Point3_<int>(i+1,j+1,M.at<unsigned int>(i+1,j+1));
				t->p3 = Point3_<int>(i+1,j,M.at<unsigned int>(i+1,j));
				//t->id = triangleid;
				triangleid++;
				//triangles.push_back(t);
			}
	}
}

//Detects hands and starts to follow. Needs callibration between the rgb and
//the depth camera. 
//NOTE: Needs to call startCleaning before
void startFollow(Mat & depthf, Mat & rgbMat){
	int delta = 25;
	int followx;
	int followy;
	locateClosest(depthf,followx,followy); //detecta lo mas cercano
//	std::cout<<followx<<' '<<followy<<'\n';
	int inix = 0;
	int iniy = 0;
	int finx = rgbMat.cols;
	int finy = rgbMat.rows;
	if(followx-delta > inix) inix = followx-delta;
	if(followx+delta < finx) finx = followx+delta;
	if(followy-delta > iniy) iniy = followy-delta;
	if(followy+delta < finy) finy = followy+delta;
	std::cout<<inix<<' '<<finx<<' '<<iniy<<' '<<finy<<std::endl;		
	for(int i = iniy; i < finy ; i++){
			for(int j = inix; j < finx; j++){
				rgbMat.at<unsigned int>(i,j) = 25; //cambio color
			}
	}
}

//Cleans the depth map, leaving only a small place to render depth data
void startCleaning(Mat &depthf){
	int limpieza =  1700000000;
	for(int i = 0; i < depthf.rows; i++){
		for(int j = 0;j < depthf.cols; j++){
			unsigned int dep = depthf.at<unsigned int>(i,j);
			if(dep >= limpieza){
				depthf.at<unsigned int>(i,j) = -1;
			}
		}
	}
}



int main(int argc, char **argv) {
	bool die = false;    	
	std::vector<Triangle> triangles;

	string filename("snapshot");
	string suffix(".png");
	int i_snap(0);

	Mat depthMat(Size(640,480),CV_16UC1);
	Mat depthf  (Size(640,480),CV_8UC1);
	Mat rgbMat(Size(640,480),CV_8UC3,Scalar(0));

    Freenect::Freenect freenect;
    MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

	namedWindow("rgb",CV_WINDOW_AUTOSIZE);
	namedWindow("depth",CV_WINDOW_AUTOSIZE);
	device.startVideo();
	device.startDepth();
	
	//initalize triangles vector size
	device.getVideo(rgbMat);
    device.getDepth(depthMat);
	depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);
	initalizeTriangles(depthf,triangles);
	
	bool iniciarLimpieza = false;
	bool iniciarSeguimiento = false;
	bool generarTriangulos = true;

	int iter = 0;
	time_t inicio;
	time_t fin;
	time(&inicio);
	
	while (!die) {
    	device.getVideo(rgbMat);
    	device.getDepth(depthMat);
    	
		depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);
		if(iniciarLimpieza 		== true) 	startCleaning(depthf);
		if(iniciarSeguimiento 	== true) 	startFollow(depthf, rgbMat);
		
		cv::imshow("rgb", rgbMat);
        cv::imshow("depth",depthf);

		char k = cvWaitKey(5);
		if( k == 27 ){
		    cvDestroyWindow("rgb");
		    cvDestroyWindow("depth");
			break;
		}

		if(k == 8 ) { //backspace
			std::cout<<"saving"<<std::endl;
			std::ostringstream file;
			file << filename << i_snap << suffix;
			cv::imwrite(file.str(),depthf);
			std::ostringstream file2;
			file2 << filename << "rgb" << i_snap << suffix;
			cv::imwrite(file2.str(),rgbMat);
			std::stringstream ss;
			ss<<i_snap;
			std::string fname;
			ss>>fname;
			fname += ".txt";
			fname = "out" + fname;
			std::ofstream archi(fname.c_str());
			if(archi.is_open()){
				for(int i = 0; i < depthf.rows; i++){
					for(int j = 0;j < depthf.cols; j++)
						archi<<depthf.at<unsigned int>(i,j)/1000000<<' ';
					archi<<'\n';
				}
			}
			i_snap++;

		}
		
		if(generarTriangulos == true){
			generateTriangles(depthf,triangles);
		}
		
		if(k == 116){// tecla t 
				ofstream out("triangles.txt");
				if(out.is_open()){
					for(int i = 0; i < triangles.size(); i++){
							triangles[i].print(out);
					}
				}
		}


/*		time(&fin);
		if(difftime(fin,inicio) == 1)
				std::cout<<iter<<std::endl;*/
		//if(iter > 10) break;
		iter++;
    }
	std::cout<<iter;
   	device.stopVideo();
	device.stopDepth();
	return 0;
}
