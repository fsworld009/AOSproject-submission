/*
Yu-Chun Lee yxl122130
10.26.2013
intend to repackage pthread
and make it easier to use for C++ classes
currently only support Joinable threads

client is in charge of start/terminate threads
*/

#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>

class Thread
{
    public:
        Thread();
        int start();
        virtual int run();
        int join();
        virtual ~Thread();
    protected:
    private:
        pthread_t m_thread;
        static void* firstrun(void* args);
};

#endif // THREAD_H
