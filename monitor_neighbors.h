#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>


extern int globalMyID;
//last time you heard from each neighbor_node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];


//neighbor neighbor_node 
extern neighbor_node* root_neighbor;
extern int neighbor_number;
extern neighbor_node* next_neighbor;
extern int neighbor_size;
extern int sequence_number;


extern neighbor_node** weight_table;
extern char* file_path;

LSA** LSA_link_list; 

//feelin cute, might delete later 
int LSA_number = 0;
//keeping track of my sequence number
int sequence_number = 0;


//threads to wait while I do operations :D
pthread_mutex_t list_operation_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t list_next_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sequence_number_thread = PTHREAD_MUTEX_INITIALIZER;

LSA** LSA_list;

//set my neighbor if received

neighbor_node* setNeighbor(int next_node, int weight);
//setting my neighbor_node
neighbor_node* setNode(int next_node, int weight);


