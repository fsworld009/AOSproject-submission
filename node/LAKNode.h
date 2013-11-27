#ifndef LAKNODE_H
#define LAKNODE_H
#include "Node.h"
/*
Yu-Chun Lee & Jack Lai 11.1.2013
implementation of our alforithm design
*/

class LAKNode: public Node
{
    struct Message
    {
        MessageType type;
        int from;
        int to;
        int seq;
        int relay;
        set<LAKNode::Message> req_list;

        bool operator < (const Message& msg) const
        {
            if (this->seq < msg.seq)
                return true;
            else if ((this->seq == msg.seq) && (this->from < msg.from))
                return true;
            else
                return false;
        }
    };


    public:
        LAKNode(int node_id);
        virtual ~LAKNode();
        int receive_message(string message);
        int virtual init();
    protected:
    private:
        int seq;
        
        int virtual run();
        int token_holder;
        int expt_resp;
        bool is_inCS;
        bool has_token;
        set<int> acked_node;
        set<LAKNode::Message> token_list;



        //mutual exclusion
        //void receive_message(string msgstr);
        void receive_request(LAKNode::Message msg);
        void receive_ack(LAKNode::Message msg);
        void receive_release(LAKNode::Message msg);
        void receive_receive(LAKNode::Message msg);
        void receive_relay(LAKNode::Message msg);
        void receive_token(LAKNode::Message msg);
        void send_request();

        //actions
        void send_token(int to);
        void send_release();
        void send_receive();
        void send_relay(int from, int seq);
        void send_ack(int to);
        void accessCS();
        void finishCS();

        string message_string(LAKNode::Message msg);
        LAKNode::Message string_message(string msgstr);
	
	//debug
	void printState();
};

#endif // LAKNODE_H
