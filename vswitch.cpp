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

#define PEND_CONNECTIONS (100)
using namespace std;

int forward (char* msg, char* to_addr, int aport)
{
	int num = aport;	
	int sockfd;
	struct addrinfo *result;
	struct sockaddr_in *server_addr;
    	
	int err = getaddrinfo(to_addr, NULL, NULL, &result);
	if( err != 0)
	{
		fprintf(stderr, "\ngetaddrinfo: %s\n", gai_strerror(err));
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
	
	if (write(sockfd, msg, 1024) != 1024)
	{
		return 3;
	}
	
	
	close(sockfd);
	return 0;
}

void handle( unsigned int client_sock, int net_status, int aport)
{
	char buffer[1024];
	char to_addr[18];
	char log_name[9];
	unsigned int node_num;
	ofstream log_file;
	
	memset(buffer, 0x00, sizeof(buffer));
	recv(client_sock, buffer, 1024, 0);
	node_num = strtol(buffer, NULL, 10);
	
	if (node_num > 45 || node_num == 0)
	{
		send(client_sock, "1", 1, 0); //Invalid Node Number
		return;
	}
	send(client_sock, "0", 1, 0); //respond that node number is valid
	
	if( node_num < 10)
	{
		sprintf(log_name, "log0%d", node_num);
	}
	else
	{
		sprintf(log_name, "log%d", node_num);
	}
	
	log_file.open(log_name, ios::out | ios::binary);
	
	
	while (1)
	{
		memset(buffer, 0x00, 1024);
		int msg_length = recv(client_sock, buffer, 1024, 0);
		if( msg_length == 0 )
		{
			printf("Peer Shutdown %d\n", node_num);
			break;
		}
		if( msg_length == -1)
		{
			printf("Error receiving message %d\n", node_num);
			break;
		}
		
		char b = buffer[msg_length]; //cut out length of padded 0x00s at the end
		while( b == 0x00 && msg_length > 0)
		{
			msg_length --;
			b = buffer[msg_length];
		}
		
		unsigned int t = buffer[0] & 0xff;
		printf("Message from %d to %d\n", node_num, t);
		log_file << buffer << endl;
		log_file << msg_length << endl;
		
		if (t == 255) //Control message
		{
			send(client_sock, "0", 1, 0);
			continue; //all messages are logged. These are only logged
		}
		
		else if( net_status == 3 && (rand() % 10) == 0  && t != 44)
		{
			//log_file << "dropped" << endl;
			send(client_sock, "0", 1, 0);
			continue;
		}
		
		else if ( net_status == 2 )
		{
			sleep( rand() % 11 + 5); //sleep between 5 and 15 seconds
		}
		
		
		if (t == 46) //If "To" is invalid, exit
		{
			send(client_sock, "0", 1, 0); // 0 means no problem, there's never a problem exiting
			break;
		}
		else if( t > 45 || t== 0 )
		{
			send(client_sock, "0", 1, 0);
			continue;
		}
		else if (t < 10) //format "To" Hostname
		{
			sprintf(to_addr, "net0%d.utdallas.edu", buffer[0]);
		}
		else
		{
			sprintf(to_addr, "net%d.utdallas.edu", buffer[0]);
		}
		
		int temp = forward(buffer, to_addr, aport);
		printf("Returned: %d\n", temp);
		send(client_sock, &temp, 4, 0);
	}

	close(client_sock);
	log_file.close();

	return;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr, client_addr;
	unsigned int server_sock, client_sock, ids;
	int addr_len, port, net_status;
	
	char hostname[1024];
	gethostname(hostname, sizeof(hostname));
	
	char char_stat[3];
	char_stat[0] = hostname[3];
	char_stat[1] = hostname[4];
	char_stat[2] = 0x00;
	int node_num = strtol( char_stat, NULL, 10);
	
	cout << "node_num = " << node_num << endl;
	
	char cfile[32];
	if(node_num < 10)
	{
		sprintf(cfile, "config0%d", node_num);
	}
	else
	{
		sprintf(cfile, "config%d", node_num);
	}
	
	ifstream ifile(cfile);
	ifile >> net_status;
	ifile.close();
		//1 for reliable uncongested
		//2 for reliable congested
		//3 for unreliable
		
	cout << "net_status = " << net_status << endl;
	
	int lport, aport;
	ifstream iports("ports.txt");
	iports >> lport >> aport;
	iports.close();
	
	port = aport;

	pthread_attr_t attr;
	pthread_t threads;

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	 
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
 
	if (listen(server_sock, PEND_CONNECTIONS) != 0)
	{
		printf("Failed to listen to port %d", port);
		return 1;
	}

	pthread_attr_init(&attr);

	pid_t pid;

	while(1)
	{
		client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t*) &addr_len);
		pid = fork();

		if (pid > 0)
		{
			handle(client_sock, net_status, aport);
			return 0;
		}

	}


	return 0;
}
