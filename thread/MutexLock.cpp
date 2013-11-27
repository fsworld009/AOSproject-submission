#include "MutexLock.h"


MutexLock::MutexLock()
{
    //ctor
    pthread_mutex_init(&m_lock, NULL);
}

bool MutexLock::lock(){
    pthread_mutex_lock(&m_lock);
    return true;
}

bool MutexLock::unlock(){
    pthread_mutex_unlock(&m_lock);
    return true;
}

MutexLock::~MutexLock()
{
    //dtor
    pthread_mutex_destroy(&m_lock);
}
