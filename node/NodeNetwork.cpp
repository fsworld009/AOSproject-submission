#include "NodeNetwork.h"
#include "Node.h"
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <stdio.h>
using namespace std;



NodeNetwork::NodeNetwork(Node* node, int node_id): m_node(node), m_accept_thread(this)//, m_server_socket(this)
{
    //ctor
    //m_sockets=0;
    m_socket=0;

    m_port=0;
    m_num_of_nodes=0;


    m_socket=0;

    m_node_id = node_id;
    m_thread_running=false;
}

int NodeNetwork::getHostName(int netid, char* host){
    if(netid==0){
        strcpy(host,"localhost");
    }else if(netid>=1 && netid<=9){
        sprintf(host,"net0%d.utdallas.edu",netid);
    }else{
        sprintf(host,"net%d.utdallas.edu",netid);
    }
    return 0;
}


int NodeNetwork::init(){
    //parse config file

    //work without switch
    /*FILE* fp = fopen("./config/socket.txt","r");

    fscanf(fp,"%d",&m_port);
    fscanf(fp,"%d",&m_num_of_switches);
    //m_switch_netid = new int[m_num_of_switches];
    for(int i=0;i<m_num_of_switches;i++){
        fscanf(fp,"%d",&m_switch_netid);
        if(i==(m_node_id-1)/4){
            break;
        }
    }*/

    FILE* fp = fopen("ports.txt","r");

    fscanf(fp,"%d",&m_port);
    fscanf(fp,"%d",&m_port);
    fclose(fp);
    
    //m_port = NODE_SOCKET_PORT;
    cout << "Node id is " << m_node_id << endl;
    //m_switch_netid = 0;
    m_switch_netid = 5*((m_node_id/5)+1);
    cout << "Switch net id is " << m_switch_netid << endl;
    char buff[30];
    getHostName(m_switch_netid,buff);
    cout << "Switch address: " << buff << endl;

    //fscanf(fp,"%d",&m_num_of_nodes);
    /*m_node_netid = new int[m_num_of_nodes];
    for(int i=0;i<m_num_of_nodes;i++){
        fscanf(fp,"%d",&m_node_netid[i]);
    }*/

    //fclose(fp);
    char filename[30];
    sprintf(filename,"./log/log_node%d.txt",m_node_id);
    m_logfile.open(filename);

    m_socket = new Socket();
    m_socket->registerEventListener(this);


    //m_server_socket.init(NODE_SOCKET_PORT);
    //m_server_socket.registerEventListener(this);
    return 0;
}



int NodeNetwork::send(unsigned int from, unsigned int to, unsigned long timestamp, char* message){
    char buff[SOCKET_MAX_BUFFER_SIZE];
    bzero(buff,SOCKET_MAX_BUFFER_SIZE);
    memcpy(buff,&to,1);
    memcpy(buff+1,&from,1);
    memcpy(buff+2,&timestamp,sizeof(long));
    memcpy(buff+2+sizeof(long),message,strlen(message));

    /*buff[0] = (char)to;
    buff[1] = (char)from;
    memcpy(buff+2,&timestamp,sizeof(long));
    memcpy(buff+2+sizeof(long),message,SOCKET_MAX_BUFFER_SIZE-2-sizeof(long));*/

    //unsigned int x1=0,x2=0,x3=0;
    //memcpy(&x1,buff,1);
    //memcpy(&x2,buff+1,1);
    //memcpy(&x3,buff+2,4);
    //cout << "RECOVER: " << x1 << " " << x2 << " " << x3 << endl;

    //cout << "aaaa" << endl;
    //sprintf(buff,"%u%u%4u",to,from,timestamp);

    //printf("STRLEN: %d RAW: %s\n",strlen(buff),buff);
    /*char* a = (char*)&timestamp;
    buff[0] = (char) to;
    buff[1] = (char) from;
    buff[2] = (char) *(a);
    buff[3] = (char) *(a+1);
    buff[4] = (char) *(a+2);
    buff[5] = (char) *(a+3);*/

    //strcpy(&buff[6],message);

    //cout << "RECOVER: " << x1 <<  " " << x2 << " " << x3 << " MSG: " << buff2 << endl;
    /*char buff2[1024];
    cout << "RAW: " << cout.hex << buff << endl;
    sscanf(buff,"%u%u%u",&x1,&x2,&x3);
    strcpy(buff2,buff+6);
    cout << "RECOVER: " << x1 << " " << x2 << " " << x3 << endl;*/

    //Shift offset by -1
    from = from-1;
    to = to-1;


    /*if(m_mode==0){
        //find socket to send
        if(m_sockets[to] == 0){

            m_sockets[to] = new Socket();

            char host[20];

            if(m_netid[to]<10){
                sprintf(host,"net0%d.utdallas.edu",m_netid[to]);
            }else{
                sprintf(host,"net%d.utdallas.edu",m_netid[to]);
            }

            m_sockets[to]->registerEventListener(this);

            m_sockets[to]->connectHost(host,m_port);

        }

        m_sockets[to]->send(buff);
    }else{
        //send to switch
        //m_socket->send(buff);
    }*/

    cout << "NodeNetwork:: send from=" << from+1 << " to=" << to+1 << " timestamp=" << timestamp <<  " msg: " << message << endl;
    m_logfile << "NodeNetwork:: send from=" << from+1 << " to=" << to+1 << " timestamp=" << timestamp <<  " msg: " << message << endl;
    m_socket->send(buff);
    return 0;
}

int NodeNetwork::start(){
    /*if(m_mode==0){
        m_server_socket.start();
    }else{
        //work with switch
        //m_socket = new Socket();
        //config this socket to connect to switch
    }*/
    //m_server_socket.start();
    char host[24];
    this->getHostName(m_switch_netid,host);
    m_socket->connectHost(host,m_port);
    m_thread_running = true;
    m_accept_thread.start();
    return 0;
}

int NodeNetwork::close_me(){


    /*if(m_sockets !=0){
        for(int i=0;i<m_num_of_nodes;i++){
            if(m_sockets[i] != 0){
                m_sockets[i]->disconnect();
                delete m_sockets[i];
                m_sockets[i]=0;
            }
        }
        delete m_sockets;
        m_sockets=0;
    }*/

    /*for(int i=0;i<m_accept_socket.size();i++){
        if(m_accept_socket[i]!=0){
            //m_accept_socket[i]->disconnect();
            delete m_accept_socket[i];
            m_accept_socket[i]=0;
        }
    }
    m_accept_socket.clear();
    m_server_socket.disconnect();*/


    if(m_socket != 0){
        m_socket->disconnect();
        delete m_socket;
        m_socket=0;

    }

    //m_server_socket.disconnect();
    if(m_logfile.is_open()){
        m_logfile.close();
    }
    return 0;
}

NodeNetwork::~NodeNetwork()
{
    //dtor
    close_me();
}
/*
int NodeNetwork::onAccept(Socket* socket){
    cout << "accept connect" << endl;
    socket->registerEventListener(this);
    m_accept_socket.push_back(socket);

    //m_socket = socket;
    /*if(m_mode==0){
        char ip[20];
        socket->getBoundedIp(ip);

        //parse ip into 4 ints
        int ip_num[4];
        sscanf(ip,"%d.%d.%d.%d",&ip_num[0],&ip_num[1],&ip_num[2],&ip_num[3]);


        //analize the last number of ip address to get net machine number
        //64~99 ->   X-63 (net01~36)
        //62 -> 36 (net37)
        //101~108 -> X-63 (net38~45)


        int netid;
        if(ip_num[3]==62){
            netid = 36;
        }else{
            netid = ip_num[3]-63;
        }
        printf("NodeNetwork:: acceptted socket is from net%d\n",netid);
        for(int i=0;i<m_num_of_nodes;i++){
            if(m_netid[i]==netid){
                m_sockets[i] = socket;
            }
        }

    }


    return 0;*/
    /*return 0;
}
int NodeNetwork::onDisconnect(ServerSocket* serverSocket){
    return 0;
}*/

int NodeNetwork::onConnect(Socket* socket){
    char buff[10];
    //sprintf(buff,"NODE %d",m_node_id);
    sprintf(buff,"%d",m_node_id);
    socket->send(buff);
    return 0;
}

int NodeNetwork::send_end_signal(){
    char buff[10];
    sprintf(buff,"END %d",m_node_id);
    send(m_node_id,44,0,buff);
    return 0;
}

int NodeNetwork::onReceive(char* message,Socket* socket){
    /*int to, from, timestamp;
    char buff[SOCKET_MAX_BUFFER_SIZE];
    from = (int)message[0];
    to = (int)message[1];
    timestamp = (int) ((message[2]<<3)|(message[3]<<2)|(message[4]<<1)|(message[5]));

    strcpy(buff,message+6);*/
    cout << "NodeNetwork::recv response " << message << endl;

    return 0;
}
int NodeNetwork::onDisconnect(Socket* socket){
    /*if(m_mode==0){
        if(m_sockets != 0){
            for(int i=0;i<m_num_of_nodes;i++){
                if(m_sockets[i] != 0){
                    if(m_sockets[i]==socket){
                        delete m_sockets[i];
                        m_sockets[i] = 0;
                    }
                }
            }
        }
    }*/
    return 0;
}


NodeNetwork::AcceptThread::AcceptThread(NodeNetwork* node_network){
    m_parent = node_network;
}

int NodeNetwork::AcceptThread::disconnect(){
    if(m_socket != -1){
        //close sockets (hence unblock send/recv threads)
        shutdown(m_socket,SHUT_RDWR);
         close(m_socket);
        m_socket = -1;
    }
    return 0;
}

int NodeNetwork::AcceptThread::run(){
    cout << "NodeNetwork: Listen thread start"  << endl;

    //create server socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0){
        cout << "NodeNetwork: Socket opening error" << endl;
        //exit(-1);
    }
    bzero((char *) &m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    hostent *server = 0;
    server = gethostbyname("localhost");    //gethostname only works with IPv4
    bcopy((char *)server->h_addr,(char *)&m_addr.sin_addr.s_addr,server->h_length);
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_addr.sin_port = htons(m_parent->m_port);


    cout << "Addr: " << inet_ntoa(m_addr.sin_addr) << " Port: " << ntohs(m_addr.sin_port) << endl;



    //bind
    int option=1;
    setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&option,sizeof(option));
    int bind_result = bind(m_socket, (struct sockaddr *) &m_addr,sizeof(m_addr));

    if (bind_result < 0){
        cout << "NodeNetwork: Socket bind error " << bind_result << endl;
        //exit(-1);
    }


    sockaddr client_addr;
    socklen_t client_len = sizeof(client_addr);
    sockaddr_in* client_addr_in = (sockaddr_in*)&client_addr;
    int accept_socket;
    listen(m_socket,3);

    char message[SOCKET_MAX_BUFFER_SIZE];
    char buff[SOCKET_MAX_BUFFER_SIZE];
    unsigned int from,to;
    unsigned long timestamp;

    while(m_parent->m_thread_running){
        accept_socket = accept(m_socket, &client_addr, &client_len);
        getpeername(accept_socket,&client_addr, &client_len);

        if(accept_socket != -1){
            //cout << "ServerSocket: accept socket " << accept_socket << " from " << inet_ntoa(client_addr_in->sin_addr) << endl;
            //temp, need improved
            //m_parent->m_accepted_socket = new Socket(accept_socket);

            bzero(message,SOCKET_MAX_BUFFER_SIZE);
            read(accept_socket,message,SOCKET_MAX_BUFFER_SIZE);
            //unsigned int from=0,to=0;
            //cout << "FORCE READ " << message << endl;
            if(strcmp("START",message)==0){
                    m_parent->m_node->start_signal();
                    cout << "RECEIVE START SIGNAL" << endl;
            }else if(strcmp("END",message)==0){
                    m_parent->m_node->disconnect_signal();
                    cout << "RECEIVE END SIGNAL" << endl;
            }else{
                from=0;
                to=0;
                timestamp=0;
                memcpy(&to,message,1);
                memcpy(&from,message+1,1);
                memcpy(&timestamp,message+2,sizeof(long));

                bzero(buff,SOCKET_MAX_BUFFER_SIZE);
                memcpy(&buff,message+2+sizeof(long),SOCKET_MAX_BUFFER_SIZE-2-sizeof(long));

                cout << "NodeNetwork:: recv from=" << from << " to=" << to << " timestamp=" << timestamp <<  " msg: " << buff << endl;
                m_parent->m_logfile << "NodeNetwork:: recv from=" << from << " to=" << to << " timestamp=" << timestamp <<  " msg: " << buff << endl;
                //m_node->receive(from,to,timestamp,buff);
                if(to==m_parent->m_node_id){
                    string msg_string(buff);
                    m_parent->m_node->receive(msg_string);
                }
            }
            //usleep(1000);
            close(accept_socket);



            //upper listener is responsible for memory management of this Socket
            //if(m_parent->m_event_listener != 0){
            //    m_parent->m_event_listener->onAccept(new Socket(accept_socket,inet_ntoa(client_addr_in->sin_addr)));
            //}

        }
        //usleep(100);
    }
    return 0;
}
