#include "LAKNode.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
using namespace std;

LAKNode::LAKNode(int node_id): Node(node_id)
{
    //ctor

}

LAKNode::~LAKNode()
{
    //dtor
}

int LAKNode::init(){
    	this->quorum_set.clear();
    	this->time_schedule.clear();
    	Node::init();
    	this->timer=0;
	this->seq = 1;
	this->expt_resp = -1;
	this->token_list.clear();
	this->acked_node.clear();
	this->is_inCS = false;

	//assume 1 has token

	if ((*this->quorum_set.begin()) == 1)
	{
		this->token_holder = 1;
		if (node_id == 1)
			this->has_token = true;
		else
			this->has_token = false;
	}
	else
	{
		this->token_holder = -1;
		this->has_token = false;
	}

	return 0;
}

int LAKNode::run(){
	this->timer = 0;
	this->CS_timer=0;
    string recv_message;

    int milisec = 1;
    timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = milisec * 1000000L;

    //if(time_schedule.size()==0){
    //    this->send_end_signal();
    //}

	//while (!done_all_request() || !recv_end_signal())
    while(timer< 60000)
	{
        //handle receive messages
        while(get_message(&recv_message)){
            receive_message(recv_message);
        }


		this->timer+=1;
		if(CS_timer==0 && is_inCS){
            		finishCS();
		}else if(CS_timer>0){
           		CS_timer-=1;
		}
		if(timer%10000==0){
            cout << timer << endl;
		}

		//cout << CS_timer << endl;
		set<unsigned long>::iterator iter = time_schedule.find(this->timer);
		if (iter != time_schedule.end())
		{
			this->num_req++;
            		send_request_cs_msg();
			this->send_request();
		}
		nanosleep(&req, 0);
	}
    cout << "END" << endl;
    return 0;
}



//Message Handlers
void LAKNode::send_request()
{
	if (this->has_token)
	{
		accessCS();
	}
	else
	{
		LAKNode::Message msg;
		msg.from = this->node_id;

		msg.seq = this->seq++;
		msg.type = REQUEST;

		set<int>::iterator iter = this->quorum_set.find(this->token_holder);
		if (iter != this->quorum_set.end())
		{
			msg.to = this->token_holder;
			send(this->node_id, this->token_holder, timer, message_string(msg));
		}
		else
		{
			for (iter = this->quorum_set.begin(); iter != this->quorum_set.end(); iter++)
			{
				if ((*iter) != this->node_id)
				{
					msg.to = (*iter);
					send(this->node_id, (*iter), timer, message_string(msg));
				}
			}
		}


		this->has_token = false;
		this->is_inCS = false;
		this->acked_node.clear();
		this->expt_resp = this->quorum_set.size() - 1;
	}
}

int LAKNode::receive_message(string msgstr){
	LAKNode::Message msg = string_message(msgstr);

	switch (msg.type)
	{
	case REQUEST:
		receive_request(msg);
		break;
	case RELEASE:
		receive_release(msg);
		break;
	case ACK:
		receive_ack(msg);
		break;
	case RECEIVE:
		receive_receive(msg);
		break;
	case RELAY:
		receive_relay(msg);
		break;
	case TOKEN:
		receive_token(msg);
		break;
	default:
        	printf("Unkown message type received!\n");
		break;
	}
	return 0;
}

void LAKNode::receive_request(LAKNode::Message msg)
{
	if (this->seq < msg.seq)
		this->seq = ++msg.seq;
	if (has_token)
	{
		if (is_inCS)
		{
			token_list.insert(msg);
		}
		else
		{
			token_list.insert(msg);
			send_token(msg.from);
			send_release();
		}
	}
	else
	{
		if (token_holder == -1)
		{
			send_ack(msg.from);
		}
		else
		{
			send_relay(msg.from, msg.seq);
		}
	}
}

void LAKNode::receive_ack(LAKNode::Message msg)
{
	acked_node.insert(msg.from);
	expt_resp--;
	if (this->acked_node.size() == this->quorum_set.size()-1)
	{
		printf("Error occurs and the request doesn't satisfied!\n");
	}
}

void LAKNode::receive_token(LAKNode::Message msg)
{
	if(msg.req_list.size()==0){
		this->has_token = true;
		this->token_holder = this->node_id;

		set<int>::iterator iter;
		for (iter = quorum_set.begin(); iter != quorum_set.end(); iter++)
		{
			if ((*iter) != this->node_id)
				send_receive();
		}
	        return;
    	}
	else
	{
 		if (msg.req_list.begin()->from != this->node_id)
		{
			send_token(msg.from);
		}
		else
		{
			this->has_token = true;
			this->token_holder = this->node_id;

			set<int>::iterator iter;
			for (iter = quorum_set.begin(); iter != quorum_set.end(); iter++)
			{
				if ((*iter) != this->node_id)
					send_receive();
			}
			accessCS();
		}
	}
}

void LAKNode::receive_receive(LAKNode::Message msg)
{
	this->token_holder = msg.from;
}

void LAKNode::receive_release(LAKNode::Message msg)
{
	if(this->token_holder != this->node_id)
		this->token_holder = -1;
}

void LAKNode::receive_relay(LAKNode::Message msg)
{
	if (this->seq < msg.seq)
		this->seq = msg.seq;
	if (this->token_holder)
	{
        if (this->is_inCS)
		{
			this->token_list.insert(msg);
		}
        else
		{
			send_token(msg.relay);
			send_release();
		}
	}
}

//actions
void LAKNode::send_token(int to)
{
	LAKNode::Message msg;
	msg.from = this->node_id;
	msg.to = to;
	msg.type = TOKEN;
	msg.req_list = this->token_list;

	send(this->node_id, to, timer, message_string(msg));
	this->token_holder = to;
	this->has_token = false;
	this->token_list.clear();
}

void LAKNode::send_ack(int to)
{
	LAKNode::Message msg;
	msg.from = this->node_id;
	msg.to = to;
	msg.type = ACK;

	send(this->node_id, to, timer, message_string(msg));
}

void LAKNode::send_release()
{
	set<int>::iterator iter;
	for (iter = quorum_set.begin(); iter != quorum_set.end(); iter++)
	{
		if ((*iter) != this->node_id)
		{
			LAKNode::Message msg;
			msg.from = this->node_id;
			msg.to = (*iter);
			msg.type = RELEASE;

			send(this->node_id, (*iter), timer, message_string(msg));
		}
	}
}

void LAKNode::send_receive()
{
	set<int>::iterator iter;
	for (iter = quorum_set.begin(); iter != quorum_set.end(); iter++)
	{
		if ((*iter) != this->node_id)
		{
			LAKNode::Message msg;
			msg.from = this->node_id;
			msg.to = (*iter);
			msg.type = RECEIVE;

			send(this->node_id, (*iter), timer, message_string(msg));
		}
	}
}


void LAKNode::send_relay(int from, int seq)
{
	LAKNode::Message msg;
	msg.from = from;
	msg.to = this->token_holder;
	msg.type = RELAY;
	msg.relay = this->node_id;
	msg.seq = seq;
	send(this->node_id, this->token_holder, timer, message_string(msg));
}

void LAKNode::accessCS()
{
    	this->is_inCS = true;
    	this->CS_timer = CS_time;
		cout << "The node is entering CS now ..." << endl;
	send_access_cs_msg();
}

void LAKNode::finishCS()
{	
    	cout << "The node is exiting CS now ..." << endl;
	//cout<<"num_req:"<<this->num_req<<endl;
	//cout<<"schedule size:"<<this->time_schedule.size()<<endl;
	//if(this->num_req == this->time_schedule.size())
		//this->send_end_signal();

    	send_finish_cs_msg();
	if (token_list.empty() != true)
	{
        //token_list.erase(token_list.begin());
		send_token(token_list.begin()->from);
		send_release();
	}
	this->is_inCS = false;
}

string LAKNode::message_string(Message msg)
{
	string str="";
	stringstream ss;
	switch (msg.type)
	{
	case REQUEST:
		ss << msg.type << " " << msg.from << " " << msg.to << " " << msg.seq;
		str = ss.str();
		break;
    	case RELAY:
		ss << msg.type << " " << msg.from << " " << msg.to << " " << msg.seq << " " << msg.relay;
		str = ss.str();
		break;
	case TOKEN:
		ss << msg.type << " " << msg.from << " " << msg.to;
		for (set<Message>::iterator iter = msg.req_list.begin(); iter != msg.req_list.end(); iter++)
		{
			ss << " " << message_string(*iter);
		}
		str = ss.str();
		break;
	default:
		ss << msg.type << " " << msg.from << " " << msg.to;
		str = ss.str();
		break;
	}
	return str;
}

LAKNode::Message LAKNode::string_message(string msgstr)
{
	Message msg;
	string type = msgstr.substr(0,msgstr.find_first_of(" "));
	msg.type = (MessageType)atoi(type.c_str());
	msgstr = msgstr.substr(msgstr.find_first_of(" ")+1);
	set<Message> req_list;
	switch (msg.type)
	{
	case REQUEST:
		msg.from = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.to = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.seq = atoi(msgstr.c_str());
		break;
	case RELAY:
		msg.from = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.to = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.seq = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.relay = atoi(msgstr.c_str());
		break;
	case TOKEN:
		msg.from = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.to = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		while (msgstr.length() > 2)
		{
			Message rq;
                rq.type = (MessageType) atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
                msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

                rq.from = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
                msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

                rq.to = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
                msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

                rq.seq = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
                msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);
                if(rq.type==RELAY){
                    //RELAY has an extra field "relay node id"
                    rq.seq = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
                    msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);
                }
                req_list.insert(rq);
                /*if(msgstr.length()==1){
                    cout <<"OPPS" << endl;
                    while(1){}
                }*/

		}
		msg.req_list = req_list;
		break;
	//for RELEASE, ACK, RECEIVE
	default:
		msg.from = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.to = atoi(msgstr.c_str());
		break;
	}
    return msg;
}


void LAKNode::printState()
{
	cout<<"token holder: "<<this->token_holder<<endl;
	cout<<"expt_resp: "<<this->expt_resp<<endl;
	cout<<"is_inCS: "<<this->is_inCS<<endl;
	cout<<"has_token: "<<this->has_token<<endl;
	cout<<"seq: "<<this->seq<<endl;

	cout<<"acked_node: ";
	set<int>::iterator a_iter;
	for(a_iter = this->acked_node.begin(); a_iter != this->acked_node.end(); a_iter++)
	{
		cout<<(*a_iter)<<" ";
	}
	cout<<endl;

	cout<<"token list : ";
	set<LAKNode::Message>::iterator iter;
	for(iter = this->token_list.begin(); iter != this->token_list.end(); iter++)
	{
		cout<<"("<<(*iter).from<<" "<<(*iter).seq <<" "<<(*iter).type<<")";
	}
	cout<<endl;
}
