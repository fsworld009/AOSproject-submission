#ifndef NODE_H
#define NODE_H
#include <set>
#include <string>
#include "NodeNetwork.h"
#include <queue>
using namespace std;

/*
Yu-Chun Lee yxl122130 11.1.2013
master class of algorithm nodes
*/

enum MessageType
    {
        REQUEST,
        RELEASE,
        RECEIVE,
        ACK,
        RELAY,
        TOKEN,
	LOCK,
	FAIL,
	INQUIRE,
	RELINQUISH
    };

class Node
{
    public:
        Node(int node_id);
        virtual ~Node();
        //int virtual receive(int from, int to, int timestamp, string message)=0;
        //int virtual receive_message(string message)=0;
        int receive(string message);

        int send(unsigned int from, unsigned int to, unsigned int timestamp, string message);
        int virtual init();
        int start();

        int start_signal();
        int disconnect_signal();

        bool recv_end_signal();
        int send_end_signal();
        bool done_all_request();


        bool get_message(string* msg);


    protected:
        const int node_id;
        unsigned long CS_time;
        set<int> quorum_set;
        set<unsigned long> time_schedule;
        unsigned long timer;
        unsigned long CS_timer;

	int num_req;
        int send_access_cs_msg();
        int send_finish_cs_msg();
        int send_request_cs_msg();
    private:
        NodeNetwork m_node_network;
        //MutexLock m_wait_lock;
        bool m_start_signaled;
        bool m_disconnect_signaled;
        int parse_quorum();
        int parse_schedule();
        int virtual run()=0;
        bool m_done;
        bool end();
        MutexLock m_queue_lock;
        queue<string> m_message_queue;
};

#endif // NODE_H
