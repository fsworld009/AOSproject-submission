#ifndef MAKNODE_H
#define MAKNODE_H
#include "Node.h"
#include <map>
using namespace std;

class MAKNode : public Node
{
	struct Message
	{
		MessageType type;
		int from;
		int to;
		int seq;

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
	MAKNode(int node_id);
	virtual ~MAKNode();
	int virtual receive_message(string message);

protected:
private:
	
	int seq;
	bool is_locked;
	bool is_inCS;
	pair<int, int> locked_for; 
	map<int, MessageType> response_map;
	set<MAKNode::Message> waiting_queue;
	set<MAKNode::Message> inquiry_list;
	
	int virtual run();
			
	//mutual exclusion
	void send_request();
	void receive_request(MAKNode::Message msg);
	void receive_locked(MAKNode::Message msg);
	void receive_failed(MAKNode::Message msg);
	void receive_inquiry(MAKNode::Message msg);
	void receive_relinquish(MAKNode::Message msg);
	void receive_release(MAKNode::Message msg);
	
	//actions
	void send_locked(int to);
	void send_inquiry(int to);
	void send_failed(int to);
	void send_relinquish(int to);
	void send_release(int to);
	void accessCS();
	void finishCS();

	string message_string(MAKNode::Message msg);
	MAKNode::Message string_message(string msgstr);

	//debug
	void printqueue();
	void printresponse();
};

#endif // MAKNode_H
