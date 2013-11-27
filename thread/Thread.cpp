#include "Thread.h"
#include <pthread.h>
#include <iostream>
using namespace std;

Thread::Thread()
{
    //ctor
    m_thread=0;
}

int Thread::start(){
    int thread_create = pthread_create(&m_thread,0,&Thread::firstrun,this);
    return thread_create==0?1:0;
}

//implement this function on subclasses
int Thread::run(){
    return 0;
}

void* Thread::firstrun(void* ptr){
    ((Thread*)ptr)->run();
    return 0;
}

int Thread::join(){
    int thread_join;
    if(m_thread != 0){
        thread_join = pthread_join(m_thread, 0);
    }
    return thread_join==0?1:0;
}

Thread::~Thread()
{
    //dtor
}
