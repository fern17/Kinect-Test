#ifndef __MUTEX_CPP_
#define __MUTEX_CPP_

#include <pthread.h>

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
#endif
