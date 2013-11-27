#include "SocketEventListener.h"

SocketEventListener::SocketEventListener()
{
    //ctor
}

SocketEventListener::~SocketEventListener()
{
    //dtor
}

int SocketEventListener::onConnect(Socket* socket){return 0;}
int SocketEventListener::onReceive(char* message,Socket* socket){return 0;}
int SocketEventListener::onDisconnect(Socket* socket){return 0;}
