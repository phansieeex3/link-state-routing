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
#include "message_objects.h"



extern int globalMyID;
//last time you heard from each neighbor_node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];




//my link list root
neighbor_list* first_neighbor;
neighbor_list* first_next_neighbor;
struct link_state_node* topology;

int neighbor_size ;


extern neighbor_node** weight_table;
extern char* file_path;

LSA** LSA_link_list; 


//keeping track of my sequence number
int sequence_numbers;


//threads to wait while I do operations :D
 pthread_mutex_t list_operation_thread;
// pthread_mutex_t list_next_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sequence_number_thread;

LSA** LSA_list;

//set my neighbor if received

neighbor_node* setNeighbor(int id, int weight);
//setting my neighbor_node
//neighbor_node* setNode(int next_node, int weight);

//function calls for different threads
void listenForNeighbors();
void* announceToNeighbors(void* unusedParam);
void* monitorNeighborsAlive(void* unusedParam);
void* monitorNeighbors(void* unusedParam);

void setUpNeighbors(int neighbor_id);
neighbor_list* insert(neighbor_list* root, neighbor_node* new_node);

// The first step on the shortest path from the local node to the destination node
typedef struct {
    int destination_id; // The id of the destination node
    int neighbor_id; // The first node on the shortest path to destination node
} path;

// Linked List of paths
typedef struct _pathList {
    path *path;
    struct _pathList *next; // Next path in the list
} pathList;

pathList* findPaths(link_state_node* localNode);
