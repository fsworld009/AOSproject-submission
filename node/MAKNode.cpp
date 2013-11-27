#include "MAKNode.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
using namespace std;

MAKNode::MAKNode(int node_id) : Node(node_id)
{
	//ctor
	this->timer = 0;
	this->is_locked = false;
	this->is_inCS = false;
	this->seq = 1;
	this->locked_for = make_pair(-1,-1);		//locked_for(seq#, node#)
	this->waiting_queue.clear();			//for the requests haven't been processed
	this->response_map.clear();			//collect the feedbacks of other nodes in the same quorum
}

MAKNode::~MAKNode()
{
	//dtor
}

//initiate requests according to the time schedule
int MAKNode::run()
{
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
        while(get_message(&recv_message))
		{
	            receive_message(recv_message);
        }


		this->timer+=1;
		if(CS_timer==0 && is_inCS)
		{
	       		finishCS();
		}
		else if(CS_timer>0)
		{
	            	CS_timer-=1;
		}
		if(timer%10000==0){
            cout << timer << endl;
		}
		set<unsigned long>::iterator iter = time_schedule.find(this->timer);
		if (iter != time_schedule.end())
		{
			this->num_req++;
			this->send_request();
		}
		nanosleep(&req, 0);
	}
	cout << "END" << endl;
    	return 0;
}

//Message Handlers
void MAKNode::send_request()
{
	MAKNode::Message msg;
	msg.from = this->node_id;
	msg.seq = this->seq++;
	msg.type = REQUEST;

	this->waiting_queue.insert(msg);

	this->locked_for = make_pair(msg.seq, msg.from);
	this->is_locked = true;

	set<int>::iterator iter;
	for (iter = quorum_set.begin(); iter != quorum_set.end(); iter++)
	{
		if ((*iter) != this->node_id)
		{
			send_request_cs_msg();
			send(this->node_id, (*iter), timer, message_string(msg));
		}
	}
}

int MAKNode::receive_message(string msgstr){

	MAKNode::Message msg = string_message(msgstr);

	//switch according to different kinds of messages
	switch (msg.type)
	{
	case REQUEST:
		receive_request(msg);
		break;
	case RELEASE:
		receive_release(msg);
		break;
	case LOCK:
		receive_locked(msg);
		break;
	case FAIL:
		receive_failed(msg);
		break;
	case INQUIRE:
		receive_inquiry(msg);
		break;
	case RELINQUISH:
		receive_relinquish(msg);
		break;
	default:
		printf("Unexpected message received!\n");
		break;
	}
	return 0;
}

void MAKNode::receive_request(MAKNode::Message msg)
{
	this->waiting_queue.insert(msg);

	//if current node is locked
	if (this->is_locked)
	{
		//if message with highest priority in the queue is just the one we received
		MAKNode::Message first = *this->waiting_queue.begin();
		if ((first.from == msg.from) && (first.seq == msg.seq))
		{
			//if we currently is locked for another node, we send inquiry message to it
			if(this->locked_for.second != this-> node_id)
				send_inquiry(this->locked_for.second);

			//if not, we lock current node for this new request and send locked message back
			else
			{
				this->locked_for = make_pair(msg.seq, msg.from);
				send_locked(msg.from);
			}
		}
		//if not, we send back a fail message
		else
			send_failed(msg.from);
	}
	else
	{
		this->is_locked = true;
		this->locked_for = make_pair(msg.seq, msg.from);
		send_locked(msg.from);
	}
}

void MAKNode::receive_locked(MAKNode::Message msg)
{
	//check whether the node has already responded in the past, if so, update the value, if not, insert the value
	map<int, MessageType>::iterator f_iter = this->response_map.find(msg.from);
	if(f_iter == this->response_map.end())
		this->response_map.insert(make_pair(msg.from, msg.type));
	else
		f_iter->second = msg.type;

	//if we receive enough responds
	if (this->response_map.size() == this->quorum_set.size() - 1)
	{
		//if there is any fail in the responds, we can just return and wait until next lock message
		map<int, MessageType>::iterator iter;
		for (iter = response_map.begin(); iter != response_map.end(); iter++)
		{
			//if there is a fail msg in all the quorum response, just do nothing and return
			if ((*iter).second == FAIL)
			{
				return;
			}
		}

		//if there is no fail in the responds, the node can enter its critical section
		this->accessCS();
	}
}

void MAKNode::receive_inquiry(MAKNode::Message msg)
{
	//if the process is in CS, just ignore the inquiry.
	if (this->is_inCS == true)
		return;

	//if the process is not in CS, first we check whether there is any fail response.
	map<int, MessageType>::iterator iter;
	for (iter = response_map.begin(); iter != response_map.end(); iter++)
	{
		if ((*iter).second == FAIL)
		{
			send_relinquish(msg.from);
			return;
		}
	}

	//if there is no fail response and the process is not in CS, then it means there is still some responses need to be received. We just save the inquiry.
	this->inquiry_list.insert(msg);
}

void MAKNode::receive_failed(MAKNode::Message msg)
{
	//check whether the node has already responded in the past, if so, update the value, if not, insert the value
	map<int, MessageType>::iterator f_iter = this->response_map.find(msg.from);
	if(f_iter == this->response_map.end())
		this->response_map.insert(make_pair(msg.from, msg.type));
	else
		f_iter->second = msg.type;

	//if current node receive a fail message, then it can send relinquish message to all the inquiry nodes
	set<MAKNode::Message>::iterator iter;
	for (iter = this->inquiry_list.begin(); iter != this->inquiry_list.end(); iter ++)
	{
		send_relinquish((*iter).from);
	}

	this->inquiry_list.clear();
}

void MAKNode::receive_relinquish(MAKNode::Message msg)
{
	Message first = *this->waiting_queue.begin();
	this->locked_for = make_pair(first.seq, first.from);
	this->is_locked = true;
	send_locked(first.from);
}

void MAKNode::receive_release(MAKNode::Message msg)
{
	//delete the request message from the waiting queue, once receives the release message
	set<MAKNode::Message>::iterator iter;
	for (iter = this->waiting_queue.begin(); iter != this->waiting_queue.end(); iter++)
	{
		if ((*iter).from == msg.from)
		{
			this->waiting_queue.erase(iter);
			break;
		}
	}

	//if the waiting queue is empty, just update the info
	if (this->waiting_queue.empty())
	{
		this->is_locked = false;
		this->locked_for = make_pair(-1,-1);
	}

	//if the waiting queue is not empty, update the info and if the highest priority request is not for current node, send out lock message
	else
	{
		MAKNode::Message temp = *this->waiting_queue.begin();
		this->locked_for = make_pair(temp.seq,temp.from);
		if(temp.from != this->node_id)
			send_locked(temp.from);
	}
}

//actions
void MAKNode::send_locked(int to)
{
	MAKNode::Message msg;
	msg.from = this->node_id;
	msg.type = LOCK;

	send(this->node_id, to, timer, message_string(msg));
}

void MAKNode::accessCS()
{
	if(this->is_inCS)
		return;
	this->is_inCS = true;
	this->CS_timer = CS_time;
	cout << "The node is entering CS now ..." <<endl;
	send_access_cs_msg();
}

void MAKNode::finishCS()
{
	cout << "The node is exiting CS now ..." << endl;
	//cout<<"num_req:"<<this->num_req<<endl;
	//cout<<"schedule size:"<<this->time_schedule.size()<<endl;
	//if(this->num_req == this->time_schedule.size())
		//this->send_end_signal();   
 	
	send_finish_cs_msg();
    	this->is_locked = false;
	this->response_map.clear();
	this->inquiry_list.clear();
	this->locked_for = make_pair(-1, -1);
	this->waiting_queue.erase(this->waiting_queue.begin());

	this->is_inCS = false;

	//send release message after finish CS
	set<int>::iterator iter;
	for (iter = this->quorum_set.begin(); iter != this->quorum_set.end(); iter++)
	{
		if ((*iter) != this->node_id)
		{
			send_release((*iter));
		}
	}

	if(this->waiting_queue.empty())
		return;

	//if there is any request left in the queue,
	//1. for the current node, send request for all other nodes in its quorum
	//2. for other node, send lock message to that node
	MAKNode::Message msg = *this->waiting_queue.begin();

	this->is_locked = true;
	this->locked_for = make_pair(msg.seq, msg.from);

	if(msg.from == this->node_id)
	{
		set<int>::iterator iter;
		for (iter = quorum_set.begin(); iter != quorum_set.end(); iter++)
		{
			if ((*iter) != node_id)
			{
				msg.to = (*iter);
				send_request_cs_msg();
				send(this->node_id, (*iter), timer, message_string(msg));
			}
		}
	}
	else
		send_locked(msg.from);
}

void MAKNode::send_failed(int to)
{
	MAKNode::Message msg;
	msg.from = this->node_id;
	msg.type = FAIL;

	send(this->node_id, to, timer, message_string(msg));
}

void MAKNode::send_inquiry(int to)
{
	MAKNode::Message msg;
	msg.from = this->node_id;
	msg.type = INQUIRE;

	send(this->node_id, to, timer, message_string(msg));
}

void MAKNode::send_relinquish(int to)
{
	MAKNode::Message msg;
	msg.from = this->node_id;
	msg.type = RELINQUISH;

	send(this->node_id, to, timer, message_string(msg));
}

void MAKNode::send_release(int to)
{
	MAKNode::Message msg;
	msg.from = this->node_id;
	msg.type = RELEASE;

	send(this->node_id, to, timer, message_string(msg));
}

string MAKNode::message_string(Message msg)
{
	string str = "";
	stringstream ss;

	switch (msg.type)
	{
	case REQUEST:
		ss << msg.type << " " << msg.from << " " << msg.seq;
		str = ss.str();
		break;
	default:
		ss << msg.type << " " << msg.from;
		str = ss.str();
		break;
	}
	return str;
}

MAKNode::Message MAKNode::string_message(string msgstr)
{
	Message msg;
	string type = msgstr.substr(0, msgstr.find_first_of(" "));
	msg.type = (MessageType) atoi(type.c_str());
	msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);
	switch (msg.type)
	{
	case REQUEST:
		msg.from = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		msgstr = msgstr.substr(msgstr.find_first_of(" ") + 1);

		msg.seq = atoi(msgstr.c_str());
		break;
	default:
		msg.from = atoi(msgstr.substr(0, msgstr.find_first_of(" ")).c_str());
		break;
	}
	return msg;
}

void MAKNode::printqueue()
{

	set<MAKNode::Message>::iterator iter;
	cout<<"Waiting queue:";
	for (iter = this->waiting_queue.begin(); iter != this->waiting_queue.end(); iter++)
	{
		printf("(%d,%d);", (*iter).seq, (*iter).from);
	}

	cout<<endl;
}

void MAKNode::printresponse()
{
	map<int, MessageType>::iterator iter;
	cout<<"response map:";
	for (iter = this->response_map.begin(); iter != this->response_map.end(); iter++)
	{
		printf("(%d,%d);", (*iter).first, (*iter).second);
	}

	cout<<endl;
}
