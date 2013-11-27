#include "Node.h"
#include <iostream>
#include <unistd.h>
#include "string.h"
#include <stdlib.h>
#include <fstream>
using namespace std;

Node::Node(int node_id): node_id(node_id), m_node_network(this, this->node_id)
{
    	//ctor
    	m_start_signaled=false;
    	m_disconnect_signaled=false;
    	CS_time=0;
    	timer=0;
    	CS_timer=0;
	num_req = 0;
	m_done = false;
}

//called by NodeNetwork when it received "START" signal
int Node::start_signal(){
    //m_wait_lock.lock();
    m_start_signaled=true;
    //m_wait_lock.unlock();
    return 0;
}
/*
int Node::waitForSignal(){
    //m_wait_lock.lock();
    cout << "Waiting for signal..." << endl;
    while(m_wait){
        //m_wait_lock.unlock();
        usleep(100);
        //m_wait_lock.lock();
    }
    //m_wait_lock.unlock();
    return 0;
}*/

int Node::init(){
    parse_quorum();
    parse_schedule();


    //print set
    /*cout << "CS TIME=" << CS_time << endl;
    set<int>::iterator iter;
    for (iter = quorum_set.begin(); iter != quorum_set.end(); iter++)
    {
        cout << *iter << " ";
    }
    cout << endl;
    for (iter = time_schedule.begin(); iter != time_schedule.end(); iter++)
    {
        cout << *iter << " ";
    }
    cout << endl;*/

    m_node_network.init();
    return 0;
}

int Node::start(){
    sleep(5);
    m_node_network.start();
    cout << "Waiting for signal..." << endl;
    while(!m_start_signaled){
        usleep(100);
    }
    run();
    this->send_end_signal();

    while(!m_disconnect_signaled){
        usleep(100);
    }
    m_node_network.close_me();
    return 0;
}

int Node::send(unsigned int from,unsigned int to,unsigned int timestamp, string message){
    m_node_network.send(from,to,timestamp,(char*)message.c_str());
    return 0;
}

int Node::parse_quorum(){
    char filepath[30];
    if(node_id<=9){
        sprintf(filepath,"quorum0%d",node_id);
    }else{
        sprintf(filepath,"quorum%d",node_id);
    }
	ifstream ifs(filepath, ios::in);
	string str;
	while (getline(ifs, str))
	{
		quorum_set.insert(atoi(str.c_str()));
	}
    ifs.close();
	return 0;
}


int Node::parse_schedule(){
    char filepath[30];
    if(node_id<=9){
        sprintf(filepath,"config0%d",node_id);
    }else{
        sprintf(filepath,"config%d",node_id);
    }
	ifstream ifs(filepath, ios::in);
	string str;
    getline(ifs, str);
    CS_time = atol(str.c_str());
	while (getline(ifs, str))
	{
		time_schedule.insert(atol(str.c_str()));
	}
    ifs.close();
	return 0;
}

int Node::receive(string message){
    m_queue_lock.lock();
    m_message_queue.push(message);
    m_queue_lock.unlock();
    return 0;
}

bool Node::get_message(string* message){
    m_queue_lock.lock();
    if(!m_message_queue.empty()){
        *message =  m_message_queue.front();
        m_message_queue.pop();
        m_queue_lock.unlock();
        return true;
    }else{
        m_queue_lock.unlock();
        return false;
    }

}

int Node::disconnect_signal(){
    m_disconnect_signaled=true;
    return 0;
}

bool Node::recv_end_signal(){
    return m_disconnect_signaled;
}

bool Node::done_all_request(){
    return m_done;
}

int Node::send_end_signal(){
    m_done=true;
    m_node_network.send_end_signal();
    return 0;
}

int Node::send_access_cs_msg(){

    m_node_network.send(node_id,255,timer,(char*)"1");
    return 0;
}


int Node::send_finish_cs_msg(){
    m_node_network.send(node_id,255,timer,(char*)"2");
    return 0;
}

int Node::send_request_cs_msg(){
    m_node_network.send(node_id,255,timer,(char*)"3");
    return 0;
}

/*
bool Node::end(){
    return m_close;
}*/

Node::~Node()
{
    //dtor

}
