#ifndef NODENETWORK_H
#define NODENETWORK_H
#include <fstream>
#include <vector>
#include "../socket/Socket.h"
#include "../socket/SocketEventListener.h"

using namespace std;

/*
Yu-Chun Lee, yxl122130 11.8.2013
network layer of algorithm nodes

set m_mode=0 for algorithm tests and 1 for cooperation to switches
*/

class Node;

class NodeNetwork: public SocketEventListener
{
    public:
        NodeNetwork(Node* node,int node_id);
        int init();
        int start();

        int send(unsigned int from,unsigned int to,unsigned long timestamp, char* message);

        virtual ~NodeNetwork();



        //SocketEventListener
        int onConnect(Socket* socket);
        int onReceive(char* message,Socket* socket);
        int onDisconnect(Socket* socket);

        //ServerSocketEventListener
        /*int onAccept(Socket* socket);
        int onDisconnect(ServerSocket* socket);*/

        int close_me();
        int send_end_signal();
        int m_port;
    protected:
    private:
        int getHostName(int netId,char* host);
        //Socket** m_sockets; //used when working without switch
        Socket* m_socket; //used when working with switch

        //ServerSocket m_server_socket;
        //vector<Socket*> m_accept_socket;
        //int m_node_number;
        int m_num_of_nodes;
        //int* m_node_netid;
        //int* m_switch_netid;

        int m_switch_netid;
        int m_num_of_switches;

        int m_node_id;

        Node* m_node;

        
        std::ofstream m_logfile;

        bool m_thread_running;

        class AcceptThread: public Thread{
            public:
                AcceptThread(NodeNetwork* parent);
                virtual int run();
                int disconnect();
            private:
                NodeNetwork* m_parent;
                int m_socket;
                sockaddr_in m_addr;
        };

        AcceptThread m_accept_thread;
        //int m_mode;
};

#endif // NODENETWORK_H
