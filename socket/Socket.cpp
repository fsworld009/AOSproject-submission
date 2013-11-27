#include "Socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
using namespace std;

#include "SocketEventListener.h"

Socket::Socket(): m_send_thread(this), m_receive_thread(this)
{
    //ctor
    m_socket = -1;
    m_event_listener = 0;
    m_host[0]='\0';

}

Socket::Socket(int socket,char* client_ip): m_socket(socket), m_send_thread(this), m_receive_thread(this)
{
    //ctor
    m_event_listener = 0;
    strcpy(m_ip,client_ip);
    createThreads();
}

int Socket::connectHost(char* host,int port){

    if(m_socket!=-1){
        //close connection
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0){
        cout << "Socket: create socket error" << endl;
        exit(-1);
    }

    if (host == 0) {
        cout << "Socket: no host address" << endl;
        exit(-1);
    }

    //int option=1;
    //setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&option,sizeof(option));

    sockaddr_in server_addr;
    hostent *server = 0;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server = gethostbyname(host);   //gethostname only works with IPv4
    if(server==0){
        cout << "Socket: cannot find host name " << server << endl;
        return 0;
    }
    bcopy((char *)server->h_addr,(char *)&server_addr.sin_addr.s_addr,server->h_length);
    server_addr.sin_port = htons(port);


    if (connect(m_socket,(sockaddr *)&server_addr,sizeof(server_addr)) < 0){
        cout << "Socket: socket connect error" << endl;
        return 0;
    }else{
        cout <<"Socket: connected to " << host << ":" << port << endl;
        strcpy(m_host,host);
        if(m_event_listener != 0){
            m_event_listener->onConnect(this);
        }
        createThreads();
        return 1;
    }
}

int Socket::getHostDNS(char* host){
    strcpy(host,m_host);
    return 0;
}

int Socket::createThreads(){
    //cout << "c" << endl;
    m_thread_running = true;
    m_send_thread.start();
    m_receive_thread.start();
    return 0;
}



int Socket::send(char* message){
    if(m_socket==-1){
        cout << "Socket: no connection yet" << endl;
    }
    m_queue_lock.lock();
    char* temp = new char[SOCKET_MAX_BUFFER_SIZE];
    memcpy(temp,message,SOCKET_MAX_BUFFER_SIZE);
    m_messages.push(temp);
    m_queue_lock.unlock();
    return 1;

}

bool Socket::registerEventListener(SocketEventListener* listener){
    if(m_event_listener != 0){
        //already registered a event listener
        return false;
    }
    m_event_listener = listener;
    return true;
}

bool Socket::unregisterEventListener(){
    if(m_event_listener==0){
        //no event listener
        return false;
    }
    m_event_listener = 0;
    return true;
}

Socket::~Socket()
{
    //dtor


    //close socket
    //disconnect();
}

int Socket::getBoundedIp(char* ip){
    strcpy(ip,m_ip);
    return 0;
}

int Socket::disconnect(){
    if(m_socket != -1){

        int socket_num = m_socket;
        //close sockets (hence unblock send/recv threads)
        shutdown(m_socket,SHUT_RDWR);
        close(m_socket);



        //trun off threads
        m_thread_running = false;
        m_receive_thread.join();
        m_send_thread.join();

        //clear msg queue
        m_queue_lock.lock();
        while(!m_messages.empty()){
            delete[] m_messages.front();
            m_messages.pop();
        }
        m_queue_lock.unlock();
       // cout << "Socket " << socket_num << " : disconnected" << endl;
        m_socket = -1;

    }
    if(m_event_listener != 0){
        m_event_listener->onDisconnect(this);
    }

    return 0;
}

Socket::SendThread::SendThread(Socket* parent){
    m_parent = parent;
}

int Socket::SendThread::run(){
    char* msg;
    int result;
    while(m_parent->m_thread_running){
            m_parent->m_queue_lock.lock();
            if(!m_parent->m_messages.empty()){
                msg = m_parent->m_messages.front();

                m_parent->m_messages.pop();
                result = write(m_parent->m_socket,msg,SOCKET_MAX_BUFFER_SIZE);
                if(result ==-1 || result == 0){
                    //cout << "Socket: send thread close" << endl;
                    m_parent->disconnect();
                    return 0;
                }else{
                    //cout << "Socket: send msg " << msg << endl;
                }


                //deallocate memory
                delete[] msg;
            }
            m_parent->m_queue_lock.unlock();
        //usleep(100);

    }
    //cout << "end send thread" << endl;
    return 0;
}

Socket::ReceiveThread::ReceiveThread(Socket* parent){
    m_parent = parent;
}

int Socket::ReceiveThread::run(){
    int result;
    while(m_parent->m_thread_running){
        bzero(m_parent->m_buffer,SOCKET_MAX_BUFFER_SIZE);
        result = read(m_parent->m_socket,m_parent->m_buffer,SOCKET_MAX_BUFFER_SIZE);
        if(result ==-1 || result == 0){
            //cout << "Socket: receive thread close" << endl;
            m_parent->disconnect();
            return 0;
        }else{
            //cout << "Socket: receive msg " << m_parent->m_buffer << endl;
        }



        if(m_parent->m_event_listener != 0){
            char* temp = new char[SOCKET_MAX_BUFFER_SIZE];
            memcpy(temp,m_parent->m_buffer,SOCKET_MAX_BUFFER_SIZE);
            m_parent->m_event_listener->onReceive(temp,m_parent);
            delete[] temp;
        }
        //usleep(100);

    }
    //cout << "end recv thread" << endl;
    return 0;
}
