#ifndef __MYFREENECTDEVICE_H__
#define __MYFREENECTDEVICE_H__

#include <libfreenect.hpp>
#include <vector>
//#include <cv.h>
//#include <cxcore.h>
//#include <highgui.h>
#include "Mutex.cpp"

//using namespace cv;
using namespace std;
class MyFreenectDevice : public Freenect::FreenectDevice {

  private:
    static const unsigned int w = 640, h = 480;
    //std::vector<uint8_t> m_buffer_depth_rgb;
    std::vector<uint16_t> m_buffer_depth;
    std::vector<uint8_t> m_buffer_video;
    //std::vector<uint16_t> m_gamma;
    //Mat depthMat;
    //Mat rgbMat;
    //Mat ownMat;
    Mutex m_rgb_mutex;
    Mutex m_depth_mutex;
    bool m_new_rgb_frame;
    bool m_new_depth_frame;
  public:
    MyFreenectDevice (freenect_context *_ctx, int _index) : 
            Freenect::FreenectDevice(_ctx, _index), 
            //m_buffer_depth_rgb(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),
            m_buffer_depth(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),
            //m_buffer_depth(FREENECT_DEPTH_11BIT), 
            m_buffer_video(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes), 
            //m_gamma(2048), 
            //depthMat(Size(640,480),CV_16UC1), 
            //rgbMat(Size(640,480),CV_8UC3,Scalar(0)), 
            //ownMat(Size(640,480),CV_8UC3,Scalar(0)),
            m_new_rgb_frame(false), m_new_depth_frame(false){
        /*for( unsigned int i = 0 ; i < 2048 ; i++) {
            float v = i/2048.0;
            v = std::pow(v, 3)* 6;
            m_gamma[i] = v*6*256;
        }*/
        // Source: http://groups.google.com/group/openkinect/browse_thread/thread/31351846fd33c78/e98a94ac605b9f21
        /*
        for (size_t i = 0; i < 2048; i++){
            const float k1 = 1.1863;
            const float k2 = 2842.5;
            const float k3 = 0.1236;
            const float d = k3 * std::tan(i/k2 + k1);
            m_gamma[i] = d;
        }*/
    }
    
    // Do not call directly even in child
    void VideoCallback(void* _rgb, uint32_t timestamp) {
        m_rgb_mutex.lock();
        uint8_t* rgb = static_cast<uint8_t*>(_rgb);
        std::copy(rgb, rgb+getVideoBufferSize(), m_buffer_video.begin());
        //rgbMat.data = rgb;
        m_new_rgb_frame = true;
        m_rgb_mutex.unlock();
    }

    // Do not call directly even in child
    void DepthCallback(void* _depth, uint32_t timestamp) {
        
        m_depth_mutex.lock();
        uint16_t* depth = static_cast<uint16_t*>(_depth);
        
        //SOURCE: http://code.google.com/p/3d-kinect-modelling/source/browse/trunk/headers/MyFreenectDevice.h
		/*
        for( unsigned int i = 0 ; i < 640*480 ; i++) {
			m_buffer_depth[i] = tan((float)depth[i]/1024.0f + 0.5f)*33.825f + 5.7f;
			
			int pval = m_gamma[depth[i]];
			int lb = pval & 0xff;
            std::cout<<lb<<std::endl;
			
            std::cerr<<"this is a debug msg"<<std::endl;
            switch (pval>>8) {
				case 0:
					m_buffer_depth_rgb[3*i+0] = 255;
					m_buffer_depth_rgb[3*i+1] = 255-lb;
					m_buffer_depth_rgb[3*i+2] = 255-lb;
					break;
				case 1:
					m_buffer_depth_rgb[3*i+0] = 255;
					m_buffer_depth_rgb[3*i+1] = lb;
					m_buffer_depth_rgb[3*i+2] = 0;
					break;
				case 2:
					m_buffer_depth_rgb[3*i+0] = 255-lb;
					m_buffer_depth_rgb[3*i+1] = 255;
					m_buffer_depth_rgb[3*i+2] = 0;
					break;
				case 3:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 255;
					m_buffer_depth_rgb[3*i+2] = lb;
					break;
				case 4:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 255-lb;
					m_buffer_depth_rgb[3*i+2] = 255;
					break;
				case 5:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 0;
					m_buffer_depth_rgb[3*i+2] = 255-lb;
					break;
				default:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 0;
					m_buffer_depth_rgb[3*i+2] = 0;
					break;
			}
		}*/
		std::copy(depth, depth+w*h, m_buffer_depth.begin());
		//this->depthMat.data = (uchar*) depth;
		//this->depthMat.data = (uchar*) m_buffer_depth_rgb[0];
        
		m_new_depth_frame = true;
        m_depth_mutex.unlock();
    }


    //bool getVideo(Mat& output) {
    bool getVideo(std::vector<uint8_t> &buffer) {
        m_rgb_mutex.lock();
        if(m_new_rgb_frame) {
            //cv::cvtColor(rgbMat, output, CV_RGB2BGR);
            buffer.swap(m_buffer_video);
            m_new_rgb_frame = false;
            m_rgb_mutex.unlock();
            return true;
        } else {
            m_rgb_mutex.unlock();
            return false;
        }
    }

    //bool getDepth(Mat& output) {
    bool getDepth(double depth[h][w]) {
        m_depth_mutex.lock();
        if(m_new_depth_frame) {
            //this->depthMat.copyTo(output);
            //
            //Codigo de lucas, cristian y emma
            for(unsigned int i=0; i<this->h; ++i) {
                for(unsigned int j=0; j<this->w; ++j) {
                    depth[i][j] = double(m_buffer_depth[i*this->w+j]);
                }
            }
            
            m_new_depth_frame = false;
            m_depth_mutex.unlock();
            return true;
        } else {
            m_depth_mutex.unlock();
            return false;
        }
	}
};
#endif
