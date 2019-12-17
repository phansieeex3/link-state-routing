#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "monitor_neighbors.h"
#include "pqueue.h"
#include "data_structure.h"

void listenForNeighbors();
void* announceToNeighbors(void* unusedParam);
void* monitorNeighborsAlive(void* unusedParam);
neighbor_node* setneighbor(int next_hop, int cost);
vector* setVector(int next_hop, int cost);
neighbor_node* linkedlist_insert(neighbor_node* head, neighbor_node* new_node);

int globalMyID = 0;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
struct sockaddr_in globalNodeAddrs[256];

//linked-list head
neighbor_node* first_neighbor;
neighbor_node* first_down_neighbor;

int neighbor_num;
vector** cost_table;


char* filename;

int main(int argc, char** argv)
{
  if(argc != 4)
    {
      fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
      exit(1);
    }
	
  filename = argv[3];
  //initialization: get this process's node ID, record what time it is, 
  //and set up our sockaddr_in's for sending to the other nodes.
  globalMyID = atoi(argv[1]);
  int i;
  for(i=0;i<256;i++)
    {
      gettimeofday(&globalLastHeartbeat[i], 0);	
      char tempaddr[100];
      sprintf(tempaddr, "10.1.1.%d", i);
      memset(&globalNodeAddrs[i], 0, sizeof(globalNodeAddrs[i]));
      globalNodeAddrs[i].sin_family = AF_INET;
      globalNodeAddrs[i].sin_port = htons(7777);
      inet_pton(AF_INET, tempaddr, &globalNodeAddrs[i].sin_addr);
    }
	
	
  //TODO: read and parse initial costs file. default to cost 1 if no entry for a node. file may be empty.
  first_neighbor = NULL;
  cost_table = (vector**)malloc(256*sizeof(vector*));
  for(i = 0; i < 256; i++) {
    cost_table[i] = NULL;
  }
  FILE* fp = fopen(argv[2], "r");
  char line[1024];
  neighbor_num = 0;
  while(1) {
    if(fgets(line, 1024, fp) == NULL) {
      break;
    }
    int node_id;
    int node_cost;
    sscanf(line, "%d %d", &node_id, &node_cost);
    first_down_neighbor = linkedlist_insert(first_down_neighbor, setneighbor(node_id, node_cost));
  }
	  
  //socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
  if((globalSocketUDP=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("socket");
      exit(1);
    }
  int optval = 1;
  setsockopt(globalSocketUDP, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
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
  //start threads... feel free to add your own, and to remove the provided ones.
  pthread_t announcerThread;
  pthread_create(&announcerThread, 0, announceToNeighbors, (void*)0);
  pthread_t monitorThread;
  pthread_create(&monitorThread, 0, monitorNeighborsAlive, (void*)0);
		
  //good luck, have fun!
  listenForNeighbors();
		
}
