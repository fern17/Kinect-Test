#ifndef __MYFREENECTDEVICE_H__
#define __MYFREENECTDEVICE_H__

#include <libfreenect.hpp>
#include <vector>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "Mutex.cpp"

using namespace cv;
using namespace std;

class MyFreenectDevice : public Freenect::FreenectDevice {
  public:
    MyFreenectDevice(freenect_context *_ctx, int _index);
    void VideoCallback(void* _rgb, uint32_t timestamp);
    void DepthCallback(void* _depth, uint32_t timestamp);
    bool getVideo(Mat& output);
    bool getDepth(Mat& output);

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
#endif
