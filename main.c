#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "monitor_neighbors.h"
#include "message_objects.h"


const int MAX = 256;  
int globalMyID = 0;
//last time you heard from each neighbor_node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
struct timeval globalLastHeartbeat[256];
//struct timeval globalLastHeartbeat[256];
//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
struct sockaddr_in globalNodeAddrs[256];


//my link list root
neighbor_list* first_neighbor;
neighbor_list* first_next_neighbor;


char* filename;

int main(int argc, char** argv)

{
	
	if(argc != 4)
	{
		
		fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
		exit(1);
	}
	


//0 ./run, 1 mynodeid, 2 initialcostsfile, 3 logfile
    filename = argv[3];

	//initialization: get this process's neighbor_node ID, record what time it is, 
	//and set up our sockaddr_in's for sending to the other nodes.
	globalMyID = atoi(argv[1]);
	int i = 0;
	for( ; i<MAX; i++ )
	{
		char ipaddress[MAX];
		gettimeofday(&globalLastHeartbeat[i], 0);
		sprintf(ipaddress, "10.1.1.%d", i);
		memset(&globalNodeAddrs[i], 0, sizeof(globalNodeAddrs[i]));
		globalNodeAddrs[i].sin_family = AF_INET; //ipv4 can connect
		globalNodeAddrs[i].sin_port = htons(7777);
		inet_pton(AF_INET, ipaddress, &globalNodeAddrs[i].sin_addr);
		//converts ipv4 address in binary form, source, destination 
	}
	
	


 
	
//TODO: read and parse initial costs file. default to cost 1 if no entry for a neighbor_node. file may be empty.
     first_neighbor = NULL;

	

	char buff[1024];

	first_next_neighbor = NULL;

	FILE* initialcostsfile = fopen(argv[2], "r");
	 

	 //get my neighbors...
	do
	{
		int weight;
		int uuid;

		printf("inputing my weights...");
		//end of file, should break from forever loop
		if(fgets(buff, 1024, initialcostsfile) == NULL) 
		{
			break;
		}
		
		sscanf(buff, "%d %d", &uuid, &weight);
		first_next_neighbor = NULL;
		//save my weights and neighbors in a linklist
		first_next_neighbor=insert(first_next_neighbor, setNeighbor(uuid, weight));
	} while(fgets(buff, 1024, initialcostsfile) != NULL);


	//socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
	if ( (globalSocketUDP=socket(AF_INET, SOCK_DGRAM, 0))  < 0)
	{
		perror("socket");
		exit(1);
	}
	char myAddr[100];
	struct sockaddr_in bindAddr;


	sprintf(myAddr, "10.1.1.%d", globalMyID);	


	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(7777);
	inet_pton(AF_INET, myAddr, &bindAddr.sin_addr);
	if(bind(globalSocketUDP, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind");
		close(globalSocketUDP);
		exit(1);
	}
	
	

	pthread_t announcerThread;
//pthread_t type address, aurgument attribute that we want new thread to contain, function pointer 
  	//pthread_create(&announcerThread,0, announceToNeighbors, (void*)0);
	pthread_create(&announcerThread,NULL, announceToNeighbors, (void *)0);
  	pthread_t monitorThread;
 	pthread_create(&monitorThread,NULL, monitorNeighbors, (void *)0);
		
  	//good luck, have fun! 
 	 listenForNeighbors();
	
	
	
}
