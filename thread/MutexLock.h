#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H
#include <pthread.h>
/*
Yu-Chun Lee yxl122130
10.26.2013
a class that proovides abstraction of mutex locks of UNIx (pthread_mutex)
*/

class MutexLock
{
    public:
        MutexLock();
        virtual ~MutexLock();
        bool lock();
        bool unlock();
    protected:
    private:
        pthread_mutex_t m_lock;
};

#endif // MUTEXLOCK_H
