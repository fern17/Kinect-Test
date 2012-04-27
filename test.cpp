#include <cmath>
#include <iostream>

using namespace std;

int main(){
    float m_gamma[2048];
    for (size_t i = 0; i < 2048; i++){
            const float k1 = 1.1863;
            const float k2 = 2842.5;
            const float k3 = 0.1236;
            const float d = k3 * std::tan(i/k2 + k1);
            m_gamma[i] = d;
            std::cout<<i<<' '<<d<<std::endl;
        }

    return 0;
}
