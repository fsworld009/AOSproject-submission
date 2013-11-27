#include <stdio.h>
#include <string.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include "../socket/Socket.h"
#define LSERVER_BUFFER_SIZE 1024

enum ROLE{NODE, SWITCH} ;

int get_netid(){
    int netid=0;
    FILE *cmd=popen("uname -a | cut -d \" \" -f2","r"); //execute Linux command to get "netXX.utdallas.edu"
    char result[50];
    fgets(result, sizeof(result), cmd);
    char num[3];
    num[0]=result[3];
    num[1]=result[4];
    num[2]='\0';        //num="XX", XX is the net id
    netid = atoi(num);  //convert the num string into int
    pclose(cmd);
    return netid;
}

ROLE get_role(int netid){
    if(netid%5==0){
        return SWITCH;
    }else{
        return NODE;
    }
}

int main(int argc, char *argv[])
{
	printf("Start listening server\n");
    int netid = get_netid();
    printf("My netid is %d\n",netid);
    
    ROLE role = get_role(netid);
    
    struct sockaddr_in server_addr/*, client_addr*/;
	unsigned int server_sock, client_sock, ids;
	int addr_len, port, net_status;
    int s_port;

	/*if (argc != 2)
	{
		printf("\nusage: %s <network status>\n\n", argv[0]);
		exit(-1);
	}*/
	
	//net_status = strtol( argv[1], NULL, 10 );
	//port = LSERVER_PORT;
    FILE* fp = fopen("ports.txt","r");

    fscanf(fp,"%d",&port);
    fscanf(fp,"%d",&s_port);
    fclose(fp);

	pthread_attr_t attr;
	pthread_t threads;

    
    //int option=1;
    
    
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    int option=1;
    setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,(char*)&option,sizeof(option));
    
	 
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
 
 
    sockaddr client_addr;
    socklen_t client_len = sizeof(client_addr);
 
	if (listen(server_sock, 100) != 0)
	{
		printf("Failed to listen to port %d", port);
		return 1;
	}else{
        printf("ListenServer:: start to listen to port %d\n",port);
    }

	pthread_attr_init(&attr);

	pid_t pid;

	while(1)
	{
		client_sock = accept(server_sock, &client_addr, &client_len);
        if(client_sock<0){
            exit(0);
        }
		pid = fork();
        //printf("%d\n",pid);
		if (pid > 0)
		{
            //printf("pid>0\n");
            char buff[LSERVER_BUFFER_SIZE];
            int result;
            while(1){
                bzero(buff,LSERVER_BUFFER_SIZE);
                //result = read(client_sock, buff, LSERVER_BUFFER_SIZE);
                recv(client_sock, buff, LSERVER_BUFFER_SIZE, 0);
                //if(result ==-1 || result == 0){
                //    printf("socket error\n");
                //    close(client_sock);
                //    close(server_sock);
                //    return 0;
                //}else{
                    //printf("RECV %s\n",buff);
                    
                    
                    if(strcmp(buff,"0")==0 || strcmp(buff,"1")==0){
                        int algorithm = atoi(buff);
                        if(role==NODE){
                            char buff[30];
                            sprintf(buff,"./node.out %d %d",netid, algorithm);
                            system(buff);
                        }else{
                            system("./switch");
                        }
                    }else if(strcmp(buff,"EXIT")==0){
                        //send an invalid msg to switch
                        /*int switch_netid = 5*((netid/5)+1);
                        Socket socket;
                        char sad[100];
                        if(switch_netid<-9){
                            sprintf(sad,"net0%d.utdallas.edu",switch_netid);
                        }else{
                            sprintf(sad,"net%d.utdallas.edu",switch_netid);
                        }
                        socket.connectHost(sad,s_port);
                        char msg[10];
                        msg[0] = 46;
                        socket.send(msg);
                        
                        socket.disconnect();*/
                        
                        shutdown(client_sock,SHUT_RDWR);
                        close(client_sock);
                        shutdown(server_sock,SHUT_RDWR);
                        close(server_sock);
                        return 0;
                    }
                //}
            }
			return 0;
		}else{
            //printf("pid=0\n");
        }

	}


	return 0;
}
