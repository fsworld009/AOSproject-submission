#include <stdio.h>
#include <string.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

typedef struct req
{
	req *prev;
	req *next;
	unsigned long time;
}req;

typedef struct config
{
	int num_nodes;
	int reliable;
	int congested;
	unsigned long alloc_time;
	req reqlist[46]; 
}config;

config read_config()
{
	ifstream config_file;
	config_file.open("config.txt");
	string buffer;
	config c;
	memset(&c, 0x00, sizeof(c));
	
	int trans[37];
	memset(trans, 0x00, sizeof(trans));
	int count = 0;
	int next = 0;
	while(count < 45)
	{
		count ++;
		
		if( count %5 == 0)
		{
			continue;
		}
		
		next ++;
		trans[next] = count;
		
	}
	
	getline(config_file, buffer); 
	while( ! config_file.eof())
	{
		
		if( buffer[0] == '#' || strlen(buffer.c_str()) == 0 )
		{
			getline(config_file, buffer);
			continue;
		}
		
		//cout << buffer << endl;
		
		if( strcmp(buffer.c_str(), "request_start") == 0)
		{
			getline(config_file, buffer);
			while (strcmp(buffer.c_str(), "request_end") != 0)
			{
				if( buffer[0] == '#'  || strlen(buffer.c_str()) == 0)
				{
					getline(config_file, buffer);
					continue;
				}
				
				istringstream iss(buffer);
				string result;
				getline(iss, result, '('); //Finish parsing this
				getline(iss, result, ',');
				//cout << "Result: " << result << endl;
				
				int node = trans[strtol(result.c_str(), NULL, 10)];
				req* current = &(c.reqlist[node]);
				while (current->next != NULL)
				{
					current = current->next;
				}
				current->next = (req*) malloc(sizeof(req));
				memset(current->next, 0x00, sizeof(req));
				current->next->prev = current;
				
				unsigned long milliseconds;
				string time;
				string temp;
			
				getline(iss, time, ')');
				istringstream t_stream(time);
			
				getline(t_stream, temp, ':'); //minutes
				milliseconds = strtol(temp.c_str(), NULL, 10);
				milliseconds *= 60;
			
				getline(t_stream, temp, ':'); //seconds
				milliseconds += strtol(temp.c_str(), NULL, 10);
				milliseconds *= 1000;
			
				getline(t_stream, temp, ')'); //milliseconds
				milliseconds += strtol(temp.c_str(), NULL, 10);
				
				current->time = milliseconds;
				
				getline(config_file, buffer);
				
			}
			continue;
		}
		
		istringstream iss(buffer);
		string result;
		getline(iss, result, '=');
		
		string test;
		
		if( result == "num_nodes")
		{
			iss >> c.num_nodes;
			
		}
		else if( result == "net_reliable")
		{
			iss >> c.reliable;
		}
		else if( result == "net_congested")
		{
			iss >> c.congested;
		}
		else if (result == "res_alloc_time")
		{
			unsigned long milliseconds;
			string time;
			string current;
			
			getline(iss, time);
			istringstream t_stream(time);
			
			getline(t_stream, current, ':'); //minutes
			milliseconds = strtol(current.c_str(), NULL, 10);
			milliseconds *= 60;
			
			getline(t_stream, current, ':'); //seconds
			milliseconds += strtol(current.c_str(), NULL, 10);
			milliseconds *= 1000;
			
			getline(t_stream, current); //milliseconds
			milliseconds += strtol(current.c_str(), NULL, 10);
			
			c.alloc_time = milliseconds;
			
		}
		
		getline(config_file, buffer);
	}
	
	return c;
}

int forward (char* msg, char* to_addr, int port)
{
	int num = port;
	int sockfd;
	struct addrinfo *result;
	struct sockaddr_in *server_addr;
    	
	int err = getaddrinfo(to_addr, NULL, NULL, &result);
	if( err != 0)
	{
		return 5;
	}
	if( result == NULL)
	{
		return 4;
	}
	
	sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sockfd == -1)
	{
		return 1;
	}
	
	server_addr = (struct sockaddr_in *) result->ai_addr;
	(*server_addr).sin_port = htons(num);
	if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1)
	{
		return 2;
	}
	
	if (write(sockfd, msg, strlen(msg)) != strlen(msg))
	{
		return 3;
	}
	
	
	close(sockfd);
	return 0;
}

void contact_servers(int num_nodes, char* msg, int port)
{
	int count = num_nodes;
	int current = 1;
	char to_addr[1024];
	while (count != 0)
	{
		if (current % 5 != 0)
		{
			count --;
		}
		
		memset(to_addr, 0x00, sizeof(to_addr));
		if (current < 10)
		{
			sprintf(to_addr, "net0%d.utdallas.edu", current);
		}
		else
		{
			sprintf(to_addr, "net%d.utdallas.edu", current);
		}
		
		int result = forward(msg, to_addr, port);
		if (result != 0)
		{
			cout << "Contacting listening server " << current << " failed: " << result << endl;
		}
		
		current ++;
	}
	
	if( current % 5 != 0)
	{
		current = current + 5 - (current %5);
	}
	
	memset(to_addr, 0x00, sizeof(to_addr));
	if (current < 10)
	{
		sprintf(to_addr, "net0%d.utdallas.edu", current);
	}
	else
	{
		sprintf(to_addr, "net%d.utdallas.edu", current);
	}
	
	int result = forward(msg, to_addr, port);
	if (result != 0)
	{
		cout << "Contacting listening server " << current << " failed: " << result << endl;
	}
	
	return;
}

void write_config(config c)
{
	for (int x = 1; x <= 45; x++)
	{
		char fname[32];
		
		if (x < 10)
		{
			sprintf(fname, "config0%d", x);
		}
		else
		{
			sprintf(fname, "config%d", x);
		}
		
		ofstream ofile(fname);
		
		if( x % 5 == 0)
		{
			if( c.reliable == 2)
			{
				ofile << 3 << endl;
			}
			else if( c.congested == 1)
			{
				ofile << 2 << endl;
			}
			else
			{
				ofile << 1 << endl;
			}
			ofile.close();
			continue;
		}
		
		ofile << c.alloc_time << endl;
		req* current = &(c.reqlist[x]);
		while (current->next != NULL)
		{
			ofile << current->time << endl;
			current = current->next;
		}
		ofile.close();
	}
}

//define quorum structure
struct Quorum
{
	int id;
	vector<int> quorum_members;
};

vector<Quorum> QuorumAssignment(int n)
{
	//define a vector to store all quorums
	vector<Quorum> quorum_list;

	/*determining size of the matrix for implementing square grid quorum assignment method*/
	int K = 1;
	while ((K*K) < n)
		K++;
	
	// create a K*K matrix and enter the node numbers
	int count = 1;
	int** myMatrix = new int*[K];							// initializing 2D matrix     
	for (int i = 0; i < K; i++)
	{
		myMatrix[i] = new int[K];
	}

	for (int i = 0; i < K; i++)
		for (int j = 0; j < K; j++)
			myMatrix[i][j] = -1;


	for (int row = 0; row < K; row++)						// iterate through rows 
		for (int col = 0; col < K; col++)					// iterate through columns
		{
			if (count <= n)
			{
				// count for all nodes 
				//entering the node number in its correct location in the matrix
				myMatrix[row][col] = count;
				count++;
			}
		}

	// Each quorum is an array of node numbers in that quorum
		for (int row = 0; row < K; row++)						// iterate through rows 
		{
			for (int col = 0; col < K; col++)					// iterate through columns
			{
				if (quorum_list.size() == n)
					break;

				Quorum q;

				//get quorum id
				q.id = myMatrix[row][col];

				// first k elements of the quorum array
				for (int  idx = 0; idx < K; idx++)
				{
					if (myMatrix[row][idx] != -1)
						q.quorum_members.push_back(myMatrix[row][idx]);
				}

				/* stores all the values in the row addressed by variable row into the first k indices of the quorum array*/
				for (int temp = 0; temp < K; temp++)
				{
					if (temp == row)
						continue;
					else
					{
						if (myMatrix[temp][col] != -1)
							q.quorum_members.push_back(myMatrix[temp][col]);
					}
				}

				//insert the quorum into the quorum list
				quorum_list.push_back(q);
			}
		}

		//delete 
		for (int idx = 0; idx < K; idx++)
			delete [] myMatrix[idx];
		delete [] myMatrix;


		return quorum_list;
}

void write_quorums( vector<Quorum> quorums)
{
	int trans[37];
	memset(trans, 0x00, sizeof(trans));
	int count = 0;
	int next = 0;
	while(count < 45)
	{
		count ++;
		
		if( count %5 == 0)
		{
			continue;
		}
		
		next ++;
		trans[next] = count;
		
	}
	
	//for( int x = 0; x < 37; x++)
	//{
	//	cout << trans[x] << " ";
	//}
	//cout << endl;
	
	vector<Quorum>::iterator iter;
	for (iter = quorums.begin(); iter != quorums.end(); iter++)
	{
		Quorum q = (*iter);
		
		char fname[32];
		if ( trans[q.id] < 10)
		{
			sprintf(fname, "quorum0%d", trans[q.id]);
		}
		else
		{
			sprintf(fname, "quorum%d", trans[q.id]);
		}
		ofstream ofile(fname);
		vector<int>::iterator it;
		for (it = q.quorum_members.begin(); it != q.quorum_members.end(); it++)
		{
			ofile << trans[(*it)] << endl;
		}
		
		ofile.close();
	}
}

typedef struct crit_sec
{
	crit_sec *prev;
	crit_sec *next;
	int node;
	unsigned long requested;
	unsigned long started;
	unsigned long finished;
} crit_sec;

typedef struct data
{
	int msg_count;
	int avg_length;
	unsigned long avg_wait;
	unsigned long runtime;
	bool conflict;
} data;

bool get_conflict( crit_sec* head)
{
	bool confl = false;
	crit_sec* c1 = head;
	while( c1->next != NULL )
	{
		int n1 = c1->node;
		crit_sec* c2 = c1;
		
		if (c1->started == 0 || c1->finished == 0)
		{
			c1 = c1->next;
			continue;
		}
		
		while( c2->next != NULL )
		{
			c2 = c2->next;
			
			if (c2->started == 0 || c2->finished == 0)
			{
				continue;
			}
			
			if( (c1->started < c2->started && c1->finished > c2->started) 
				|| (c1->started < c2->finished && c1->finished > c2->finished))
			{
				confl = true;
				break;
			}

		}
		
		if( confl)
		{
			break;
		}
		
		c1 = c1->next;
	}
	
	return confl;
}

unsigned long get_runtime( crit_sec* head )
{
	crit_sec* current = head;
	unsigned long max = current->finished;
	
	while( current->next != NULL)
	{
		current = current->next;
		if (current->finished > max)
		{
			max = current->finished;
		}
		
	}
	
	return max;
}

unsigned long get_wait(crit_sec* head)
{
	unsigned long total_wait = 0;
	int num_req = 0;
	crit_sec* current = head;
	
	while( current != NULL)
	{
		if (current->started != 0 && current->requested != 0)
		{
			num_req ++;
			total_wait += current->started - current->requested;
		}
		current = current->next;
	}
	
	if (num_req == 0)
	{
		return -1;
	}
	
	return (total_wait / num_req); //loss of precision, we round to milliseconds
}

void write_results(crit_sec* head, int r)
{
	char fname[20];
	memset(fname, '\0', sizeof(fname));
	sprintf(fname, "output%d", r);
	
	ofstream ofile(fname);
	crit_sec* current = head;
	
	while(current != NULL)
	{
		if(current->finished != 0)
		{
			ofile << current->node << " entered critical section at: " << current->started
				<< " and exited at: " << current->finished << endl;
		}
		current = current->next;
	}
	ofile.close();
	return;
}

data* get_results(int num_nodes, int r)
{
	int trans[37];
	memset(trans, 0x00, sizeof(trans));
	int count = 0;
	int next = 0;
	data* d;
	d = (data*) malloc(sizeof(data));
	memset(d, 0x00, sizeof(data));
	
	while(count < 45)
	{
		count ++;
		
		if( count %5 == 0)
		{
			continue;
		}
		
		next ++;
		trans[next] = count;
		
	}
	
	d->msg_count = 0;
	int total_length = 0;
	int temp_length;
	
	crit_sec* head;
	head = (crit_sec*) malloc(sizeof(crit_sec));
	memset(head, 0x00, sizeof(crit_sec));
	crit_sec* current = head;
	
	for( int x = 1; x<= num_nodes; x++)
	{
		char fname[10];
		string buffer;
		
		sprintf(fname, "log%.2d", trans[x]);
		
		ifstream ifile;
		ifile.open(fname);
		
		if( ! ifile.is_open() )
		{
			cout << "Failed to open " << fname << endl;
			continue;
		}
		
		cout << "Opened " << fname << endl;
		getline(ifile, buffer);
		
		while( ! ifile.eof())
		{
			if(buffer[0] != 'c')
			{
				temp_length = strtol(buffer.c_str(), NULL, 10);
				total_length += temp_length;
				d->msg_count ++;
				getline(ifile, buffer);
				continue;
			}
			
			if(buffer[1] == '3')
			{
				getline(ifile, buffer);
				current->requested = strtol(buffer.c_str(), NULL, 10);
				current->node = x;
				
				getline(ifile, buffer);
				while( ! ifile.eof() && buffer[0] != 'c')
				{
					temp_length = strtol(buffer.c_str(), NULL, 10);
					total_length += temp_length;
					d->msg_count ++;
					getline(ifile, buffer);
				}
				
				if(ifile.eof())
				{
					continue;
				}
				
				if(buffer[1] == '1')
				{
					getline(ifile, buffer);
					current->started = strtol(buffer.c_str(), NULL, 10);
					
					getline(ifile, buffer);
					while( ! ifile.eof() && buffer[0] != 'c')
					{
						temp_length = strtol(buffer.c_str(), NULL, 10);
						total_length += temp_length;
						d->msg_count ++;
						getline(ifile, buffer);
					}
					
					if(ifile.eof())
					{
						continue;
					}
					
					if(buffer[1] == '2')
					{
						getline(ifile, buffer);
						current->finished = strtol(buffer.c_str(), NULL, 10);
						
						current->next = (crit_sec*) malloc(sizeof(crit_sec));
						memset(current->next, '\0', sizeof(crit_sec));
						current->next->prev = current;
						
						current = current->next;
						getline(ifile, buffer);
						continue;
					}
					
					continue;
				}
				
				continue;
			}
			
			getline(ifile, buffer);
		}
		//cout << "Reached EOF" << endl;
		
		if( current->requested != 0)
		{
			current->next = (crit_sec*) malloc(sizeof(crit_sec));
			memset(current->next, '\0', sizeof(crit_sec));
			current->next->prev = current;
			current = current->next;
		}
		ifile.close();
	}
	
	d->conflict = get_conflict(head);
	d->runtime = get_runtime(head);
	d->avg_wait = get_wait(head);
	
	if( d->msg_count == 0)
	{
		d->avg_length = 0;
		return d;
	}
	
	d->avg_length = total_length / d->msg_count;
	
	write_results(head, r);
	
	return d;
}

void clear_logs()
{
	system("rm -f log??");
}

void cleanup()
{
	system("rm -f quorum??");
	system("rm -f config??");
}

void term_wait(int num_nodes, unsigned int server_sock)
{
	struct sockaddr_in client_addr;
	unsigned int client_sock;
	int addr_len;
	int count = 0;
	
	while( count < num_nodes)
	{
		client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t*) &addr_len);
		char buffer[1024];
		recv(client_sock, buffer, 1024, 0);
		//send(client_sock, "\xff", 1, 0);
		
		count ++;
		cout << count << " / " << num_nodes << endl;
		
		close(client_sock);
		
	}
	
}

unsigned int term_listen(int aport)
{
	struct sockaddr_in server_addr;
	unsigned int server_sock;
	int port = aport;
	
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	 
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	
	if (listen(server_sock, 1024) != 0)
	{
		cout << "Failed to listen to port: " << port << endl;
	}
	
	return server_sock;
}

int main(int argc, char *argv[])
{
	//read in config
	config c = read_config();
	int lport, aport;
	ifstream iports("ports.txt");
	iports >> lport >> aport;
	iports.close();
	
	//quorums
	vector<Quorum> quorums = QuorumAssignment(c.num_nodes);
	write_quorums(quorums);
	
	//write config
	write_config(c);
	
	//create server to listen for termination
	unsigned int server_sock = term_listen(aport);
	
	//contact necessary servers to start
	char msg[10];
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "0");
	contact_servers(c.num_nodes, msg, lport);
	sleep(20);
	
	//run algorithm 1
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "START");
	contact_servers(c.num_nodes, msg, aport);
	term_wait(c.num_nodes, server_sock);
	
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "END");
	contact_servers(c.num_nodes, msg, aport);
	
	//collect results
	data* res1 = get_results(c.num_nodes, 1);
	clear_logs();
	
	cout << "Results 1 collected" << endl;
	
	//switch algorithm
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "1");
	cout << "Contacting for Round 2" << endl;
	contact_servers(c.num_nodes, msg, lport);
	cout << "Contact complete" << endl;
	sleep(20);
	
	//run algorithm 2
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "START");
	contact_servers(c.num_nodes, msg, aport);
	term_wait(c.num_nodes, server_sock);
	
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "END");
	contact_servers(c.num_nodes, msg, aport);
	sleep(20);
	
	//tell listening servers to exit
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "EXIT");
	contact_servers(c.num_nodes, msg, lport);
	
	//collect results
	data* res2 = get_results(c.num_nodes, 2);
	clear_logs();
	cleanup();
	close(server_sock);
	
	cout << "Message Count: " << endl;
	cout << "    Maekawa: " << res1->msg_count << endl;
	cout << "    Our Algorithm: " << res2->msg_count << endl;
	cout << "Average Message Length : " << endl;
	cout << "    Maekawa: " << res1->avg_length << endl;
	cout << "    Our Algorithm: " << res2->avg_length << endl;
	cout << "Average Wait: " << endl;
	cout << "    Maekawa: " << res1->avg_wait << endl;
	cout << "    Our Algorithm: " << res2->avg_wait << endl;
	cout << "Running Time: " << endl;
	cout << "    Maekawa: " << res1->runtime << endl;
	cout << "    Our Algorithm: " << res2->runtime << endl;
	cout << "Mutual Exlusion Maintained: " << endl;
	if( ! res1->conflict)
	{
		cout << "    Maekawa: True" << endl;
	}
	else
	{
		cout << "    Maekawa: False" << endl;
	}
	if( ! res2->conflict)
	{
		cout << "    Our Algorithm: True" << endl;
	}
	else
	{
		cout << "    Our Algorithm: False" << endl;
	}
	
	return 0;
}

// send to listening server --> send(whatev_sock, '0', 1); 'start'

//analyze collected data
//change all times to longs
//how to read message length, possibly include as a field?
//add control message for requesting critical section
