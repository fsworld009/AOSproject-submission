#ifndef SOCKETEVENTLISTENER_H
#define SOCKETEVENTLISTENER_H
#include "Socket.h"
/*
Yu-Chun Lee yxl122130
10.26.2013
a class that listens to events from Socket such as send/receive a msg
extends this class to become a listener
*/

class SocketEventListener
{
    public:
        SocketEventListener();
        virtual ~SocketEventListener();
        int virtual onConnect(Socket* socket);
        int virtual onReceive(char* message,Socket* socket);
        int virtual onDisconnect(Socket* socket);

        /*
        onQueue
        onSend
        onReceive
        onConnect
        onDisconnect
        onUnregisterListener;
        */
    protected:
    private:
};

#endif // SOCKETEVENTLISTENER_H
