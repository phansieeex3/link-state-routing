#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "data_structure.h"
#include "pqueue.h"

extern int globalMyID;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];

extern neighbor_node* first_neighbor;
extern int neighbor_num;
extern neighbor_node* first_down_neighbor;

extern vector** cost_table;

extern char* filename;

LSP** LSP_list;
int LSP_num = 0;
int sequence_num = 0;

pthread_mutex_t list_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t list_down_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sequence_num_m = PTHREAD_MUTEX_INITIALIZER;

vector* setVector(int next_hop, int cost);
neighbor_node* setneighbor(int next_hop, int cost);

neighbor_node* linkedlist_insert(neighbor_node* head, neighbor_node* new_node) {
  new_node->next = NULL;
  new_node->previous = NULL;
  if(head == NULL) {
    head = new_node;
  }
  else {
    new_node->next = head;
    head->previous = new_node;
    head = new_node;
  }
  return head;
}

neighbor_node* linkedlist_getNeighbor(neighbor_node* head, int neighbor_id) {
  neighbor_node* current = head;
  while(current != NULL) {
    if(current->neighbor_cost->next_hop == neighbor_id) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

neighbor_node* linkedlist_deleteNeighbor(neighbor_node* head, int neighbor_id) {
  neighbor_node* current = head;
  while(current != NULL) {
    if(current->neighbor_cost->next_hop == neighbor_id) {
      //printf("find correspding neighbor %d\n", neighbor_id);
      //printf("current->previous %d\n", current->previous == NULL); 
      if(current->previous == NULL) {
	head = current->next;
      }
      else if(current->next == NULL) {
	current->previous->next = NULL;
      }
      else {
	current->previous->next = current->next;
	current->next->previous = current->previous;
      }
      return head;
    }
    current = current->next;
  }
  //printf("head == NULL %d\n", head == NULL);
  return head;
}

neighbor_node* linkedlist_update(neighbor_node* head, int neighbor_id, neighbor_node* new_node) {
  head = linkedlist_deleteNeighbor(head, neighbor_id);
  head = linkedlist_insert(head, new_node);
  return head;
}

int linkedlist_contains(neighbor_node* head, int neighbor_id) {
  neighbor_node* current = head;
  while(current != NULL) {
    if(current->neighbor_cost->next_hop == neighbor_id) {
      return 1;
    }
    current = current->next;
  }
  return 0;
}
link_state_vector* createLinkStateVector(int dest_ID, int next_hop, int previous_node, int cost) {
  link_state_vector* new_vector = (link_state_vector*)malloc(sizeof(link_state_vector));
  new_vector->dest_ID = dest_ID;
  new_vector->next_hop = next_hop;
  new_vector->previous_node = previous_node;
  new_vector->cost = cost;
  return new_vector;
}
//Yes, this is terrible. It's also terrible that, in Linux, a socket
//can't receive broadcast packets unless it's bound to INADDR_ANY,
//which we can't do in this assignment.
void hackyBroadcast(const char* buf, int length)
{
  int i;
  for(i=0;i<256;i++)
    if(i != globalMyID) //(although with a real broadcast you would also get the packet yourself)
      sendto(globalSocketUDP, buf, length, 0,
	     (struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
}

char* parseLSP() {
  //LSP format: "LSP" + myID + sequence_num + neighbor_num + vector
  int bufLen = 3 + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + neighbor_num*sizeof(vector);
  //printf("parseLSP neighbor_num %d\n", neighbor_num);
  //printf("Buflen %d\n", bufLen);
  char* sendBuf = (char*)malloc(bufLen);
  int myID = htonl(globalMyID);
  int new_sequence_num = htonl(sequence_num);
  int new_neighbor_num = htonl(neighbor_num);
  strcpy(sendBuf, "LSP");
  memcpy(sendBuf + 3, &myID, sizeof(int));
  memcpy(sendBuf + 3 + sizeof(int), &new_sequence_num, sizeof(int));
  memcpy(sendBuf + 3 + 2 * sizeof(int), &new_neighbor_num, sizeof(int));
  int i = 0;
  neighbor_node* current = first_neighbor;
  while(current != NULL) {
    //   printf("%d  %lu  %lu\n", bufLen, 3 + 3 * sizeof(int), sizeof(vector));
    memcpy(sendBuf + 3 + 3 * sizeof(int) + i * sizeof(vector), current->neighbor_cost, sizeof(vector));
    i++;
    current = current->next;
  }
  return sendBuf;
}

void* updateLSP2neighbors(int source_neighbor) {
  char* sendBuf = parseLSP();
  int bufLen = 3 + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + neighbor_num * sizeof(vector);
  neighbor_node* current = first_neighbor;
  //printf("sequence %d\n", sequence_num);
  while(current != NULL) {
    int neighbor_id = current->neighbor_cost->next_hop;
    //printf("neighbor id%d\n", neighbor_id);
    if(neighbor_id != source_neighbor) {
      //printf("neighbor %d  %d\n", neighbor_id, source_neighbor);
      if(sendto(globalSocketUDP, sendBuf, bufLen, 0, (struct sockaddr*)&globalNodeAddrs[neighbor_id], sizeof(globalNodeAddrs[neighbor_id])) < 0) {
	perror("sendto()");
      }
    }
    current = current->next;
  }
  pthread_mutex_lock(&sequence_num_m);
  sequence_num++;
  pthread_mutex_unlock(&sequence_num_m);
}
int getSequenceNum(void* LSPbuf) {
  int sn = ntohl(*((int*)(LSPbuf + 3 + sizeof(int))));
  return sn;
}

void forwardLSP(char* recvBuf, int bytesRecvd, int heardFrom) {
  int node_id = ntohl(*((int*)(recvBuf + 3)));
  neighbor_node* current = first_neighbor;
  while(current != NULL) {
    int neighbor_id = current->neighbor_cost->next_hop;
    //printf("neighbor_id %d node_id %d\n", neighbor_id, node_id);
    if(neighbor_id != node_id && neighbor_id != heardFrom) {
      if(sendto(globalSocketUDP, recvBuf, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[neighbor_id], sizeof(globalNodeAddrs[neighbor_id])) < 0) {
	perror("sendto()");
      } 
    }
    current = current->next;
  }
}
LSP* getLSP(void* LSPbuf) {
  LSP* lsp = (LSP*)malloc(sizeof(LSP));
  lsp->node_id = ntohl(*((int*)(LSPbuf + 3)));
  lsp->sequence_num = getSequenceNum(LSPbuf);
  lsp->neighbor_num = ntohl(*((int*)(LSPbuf + 3 + 2 * sizeof(int))));
  neighbor_node* new_neighbor_list = (neighbor_node*)malloc(sizeof(neighbor_node));
  lsp->first_neighbor = NULL;
  int i;
  for(i = 0; i < lsp->neighbor_num; i++) {
    //printf("insert new_neighbor into lsp");
    int nv_next_hop = ((vector*)(LSPbuf + 3 + 3 * sizeof(int) + i * sizeof(vector)))->next_hop;
    int nv_cost = ((vector*)(LSPbuf + 3 + 3 * sizeof(int) + i * sizeof(vector)))->cost;
    lsp->first_neighbor = linkedlist_insert(lsp->first_neighbor, setneighbor(nv_next_hop, nv_cost));
  }
  return lsp;
}

//------------callback functions for pqueue-------------

/**< callback to compare nodes */
int
cmp_pri(pqueue_pri_t next, pqueue_pri_t curr)
{
  link_state_vector* new_next = (link_state_vector*)next;
  link_state_vector* new_curr = (link_state_vector*)curr;
  if(new_next->cost > new_curr->cost) {
    return 1;
  }
  else if(new_next->cost == new_curr->cost) {
    if(new_next->dest_ID > new_curr->dest_ID) {
      return 1;
    }
  }
  return 0;
}

/**< callback to get priority of a node */
pqueue_pri_t get_pri(void *a)
{
  return (link_state_vector *)a;
}

/**< callback to set priority of a node */
void set_pri(void *a, pqueue_pri_t pri)
{
  a = pri;
}

/**< callback to get position of a node */
size_t get_pos(void *a)
{
  return ((link_state_vector *) a)->pos;
}

/**< callback to set position of a node */
void set_pos(void *a, size_t pos)
{
  ((link_state_vector *) a)->pos = pos;
}

int equals(void *a, void *b)
{
  return ((link_state_vector*)a)->dest_ID == ((link_state_vector*)b)->dest_ID;
}
//----------------------------------------------------


void run_dfs(pqueue_t* tentative_table) {
  //printf("tentative_table size %d\n", pqueue_size(tentative_table));
  link_state_vector* new_confirmed = (link_state_vector*)pqueue_pop(tentative_table);
  if(new_confirmed == NULL) {
    return;
  }
  //printf("new_confirmed%d\n", new_confirmed->dest_ID);
  cost_table[new_confirmed->dest_ID] = setVector(new_confirmed->next_hop, new_confirmed->cost);
  //printf("%d newfirmed == NULL\n", new_confirmed == NULL);
  vector* new_vector = setVector(new_confirmed->next_hop, new_confirmed->cost);
  int destID = new_confirmed->dest_ID;
  LSP* correspondingLSP = LSP_list[destID];
  if(correspondingLSP == NULL) {
    // printf("%d not finding LSP for %d\n", globalMyID, destID);
    return;
  }
  neighbor_node* new_neighbor = correspondingLSP->first_neighbor;
  neighbor_node* current = new_neighbor;
  while(current != NULL) {
    //printf("running dfs\n");
    if(cost_table[current->neighbor_cost->next_hop] != NULL) {
      current = current->next;
      continue;
    }
    link_state_vector* new_vector = createLinkStateVector(current->neighbor_cost->next_hop, new_confirmed->next_hop, new_confirmed->dest_ID, current->neighbor_cost->cost + new_confirmed->cost);
    link_state_vector* existing_vector;
    if((existing_vector = (link_state_vector*)pqueue_get(tentative_table, new_vector)) != NULL) {
      if(existing_vector->cost > new_vector->cost) {
	pqueue_remove(tentative_table,existing_vector);
	pqueue_insert(tentative_table, new_vector);
      }
      else if(existing_vector->cost == new_vector->cost) {
	if(existing_vector->previous_node > new_vector->previous_node) {
	  pqueue_remove(tentative_table,existing_vector);
	  pqueue_insert(tentative_table, new_vector);
	}
      }
    }
    else {
      pqueue_insert(tentative_table, new_vector);
    }
    current = current->next;
  }
  run_dfs(tentative_table);
}
 
void run_dijkstra() {
  neighbor_node* current = first_neighbor;
  if(first_neighbor == NULL) {
    return;
  }
  int i;
  pqueue_t* tentative_table = pqueue_init(256, cmp_pri, get_pri, set_pri, get_pos, set_pos,equals);
  for(i = 0; i < 256; i++) {
    free(cost_table[i]);
    cost_table[i] = NULL;
  }
  cost_table[globalMyID] = setVector(globalMyID, 0);
  //printf("%d\n", first_neighbor == NULL);
  while(current != NULL) {
    link_state_vector* new_vector = createLinkStateVector(current->neighbor_cost->next_hop, current->neighbor_cost->next_hop, globalMyID, current->neighbor_cost->cost); 
    pqueue_insert(tentative_table, new_vector);
    current = current->next;
  }
  run_dfs(tentative_table);
}

void* announceToNeighbors(void* unusedParam)
{ 
  struct timespec sleepFor;
  sleepFor.tv_sec = 0;
  sleepFor.tv_nsec = 300 * 1000 * 1000; //300 ms
  while(1)
    {
      char* sendBuf = parseLSP(-1);
      int bufLen = 3 + sizeof(int) + sizeof(int) + sizeof(int) + neighbor_num * sizeof(vector);
      hackyBroadcast(sendBuf, bufLen);
      pthread_mutex_lock(&sequence_num_m);
      sequence_num++;
      pthread_mutex_unlock(&sequence_num_m);
      nanosleep(&sleepFor, 0);
    }
}

void teardownNode(int neighbor_id) {
  printf("tear down wait for mutex\n");
  pthread_mutex_lock(&list_m);
  //printf("mutex locked by teardown\n");
  neighbor_node* broken_neighbor = linkedlist_getNeighbor(first_neighbor, neighbor_id);
  if(broken_neighbor == NULL) {
    pthread_mutex_unlock(&list_m);
    return;
  }
  else {
    first_neighbor = linkedlist_deleteNeighbor(first_neighbor, neighbor_id);
    neighbor_num--;
  }
  first_down_neighbor = linkedlist_insert(first_down_neighbor, broken_neighbor);
  //printf("teardone %d\n", neighbor_id);
  updateLSP2neighbors(neighbor_id);
  pthread_mutex_unlock(&list_m);

}

void deliverHistoryLSP(int neighbor_id) {
  int i;
  for(i = 0; i < 256; i++) {
    if(LSP_list[i] != NULL) {
       //LSP format: "LSP" + myID + sequence_num + neighbor_num + vector
      int lsp_node_id = LSP_list[i]->node_id;
      int lsp_sequence_num = LSP_list[i]->sequence_num;
      int lsp_neighbor_num = LSP_list[i]->neighbor_num;
      neighbor_node* lsp_first_neighbor = LSP_list[i]->first_neighbor;
      int bufLen = 3 + 3 * sizeof(int) + lsp_neighbor_num * sizeof(vector);
      char* sendBuf = (char*)malloc(bufLen);
      strcpy(sendBuf, "LSP");
      memcpy(sendBuf + 3, &lsp_node_id, sizeof(int));
      memcpy(sendBuf + 3 + sizeof(int), &lsp_sequence_num, sizeof(int));
      memcpy(sendBuf + 3 + 2 * sizeof(int), &lsp_neighbor_num, sizeof(int));
      int i = 0;
      neighbor_node* current = lsp_first_neighbor;
      while(current != NULL) {
	//   printf("%d  %lu  %lu\n", bufLen, 3 + 3 * sizeof(int), sizeof(vector));
	memcpy(sendBuf + 3 + 3 * sizeof(int) + i * sizeof(vector), current->neighbor_cost, sizeof(vector));
	i++;
	current = current->next;
      }
      if(sendto(globalSocketUDP, sendBuf, bufLen, 0, (struct sockaddr*)&globalNodeAddrs[neighbor_id], sizeof(globalNodeAddrs[neighbor_id])) < 0) {
	perror("sendto()");
      }
    }
  }
}
void setupNode(int neighbor_id) {  
  pthread_mutex_lock(&list_down_m);
  neighbor_node* recover_neighbor = linkedlist_getNeighbor(first_down_neighbor, neighbor_id);
  first_down_neighbor = linkedlist_deleteNeighbor(first_down_neighbor, neighbor_id);

  if(recover_neighbor == NULL) {
    first_neighbor = linkedlist_insert(first_neighbor, setneighbor(neighbor_id, 1));
    neighbor_num++;
  }
  else {
    first_neighbor = linkedlist_insert(first_neighbor, recover_neighbor);
    neighbor_num++;
  }
  //deliverHistoryLSP(neighbor_id);
  updateLSP2neighbors(-1);
  pthread_mutex_unlock(&list_down_m);
}


void* monitorNeighborsAlive(void* unusedParam) {
  struct timespec sleepFor;
  sleepFor.tv_sec = 0;
  sleepFor.tv_nsec = 650 * 1000 * 1000; //650 ms
  while(1)
    {
      neighbor_node* current = first_neighbor;
      struct timeval tempTime;
      while(current != NULL) {
	int neighbor_id = current->neighbor_cost->next_hop;
	gettimeofday(&tempTime, 0);
	if(tempTime.tv_sec - globalLastHeartbeat[neighbor_id].tv_sec > 1) {
	  teardownNode(neighbor_id);
	}
	current = current->next;
      }
      nanosleep(&sleepFor, 0);
    }
}   

void listenForNeighbors()
{
  char fromAddr[100];
  struct sockaddr_in theirAddr;
  socklen_t theirAddrLen;
  unsigned char recvBuf[1000];
  int i;
  LSP_list = (LSP**)malloc(256*sizeof(LSP*));
  for(i = 0; i < 256; i++) {
    LSP_list[i] = NULL;
  }
  int bytesRecvd;
 
 char *ext = ".txt";
 char *result = NULL;
 asprintf(&result, "%s%s", filename, ext);
printf("FILENAMEEE  %s\n", result);
  FILE* fp = fopen(result, "w+");
  char logLine[1024];
  while(1)
    {
      theirAddrLen = sizeof(theirAddr);
      if ((bytesRecvd = recvfrom(globalSocketUDP, recvBuf, 1000 , 0, 
				 (struct sockaddr*)&theirAddr, &theirAddrLen)) == -1)
	{
	  perror("connectivity listener: recvfrom failed");
	  exit(1);
	}
		
      inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);
      short int heardFrom = -1;
      if(strstr(fromAddr, "10.1.1."))
	{
	  heardFrom = atoi(
			   strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
	  //TODO: this node can consider heardFrom to be directly connected to it; do any such logic now.
	  pthread_mutex_lock(&list_m);
	  if(!linkedlist_contains(first_neighbor, heardFrom)) {
	    //printf("setup node %d\n", heardFrom);
	    setupNode(heardFrom);
	  }
	  pthread_mutex_unlock(&list_m);
	  //record that we heard from heardFrom just now.
	  gettimeofday(&globalLastHeartbeat[heardFrom], 0);
	}
      //printf("heardFrom %d\n", heardFrom);	
      //Is it a packet from the manager? (see mp2 specification for more details)
      //send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
      void* data_in = (void*)recvBuf;
      short int destID = ntohs(*((short int*)(data_in + 4)));
      if(!strncmp(recvBuf, "send", 4))
	{
	  //TODO send the requested message to the requested destination node
	  char*msg = (char*)malloc(bytesRecvd - 6 + 1);
	  memcpy(msg, data_in + 4 + sizeof(short int), bytesRecvd - 6);
	  msg[bytesRecvd - 6] = '\0';
	  if(destID == globalMyID) {
	    sprintf(logLine, "receive packet message %s\n", msg);
	  }
	  else {
	    if(first_neighbor == NULL) {
	      continue;
	    }
	    run_dijkstra();
	    char* data_out = (char*)malloc(bytesRecvd);
	    strcpy(data_out, "dest");
	    short int new_destID = htons(destID);
	    memcpy(data_out + 4, &new_destID, sizeof(short int));
	    memcpy(data_out + 4 + sizeof(short int), msg, strlen(msg));
	    //printf("destID %d\n", destID);
	    if(cost_table[destID] == NULL) {
	      sprintf(logLine, "unreachable dest %d\n", destID);
	      printf("%s", logLine);
	      fwrite(logLine, 1, strlen(logLine), fp);
	      continue;
	    }
	    short int next_hop = cost_table[destID]->next_hop;
	    if(sendto(globalSocketUDP, data_out, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[next_hop], sizeof(globalNodeAddrs[next_hop])) < 0)
	      perror("sendto()");
	    sprintf(logLine, "sending packet dest %d nexthop %d message %s\n", destID, next_hop, msg);
	  }
	  printf("%s", logLine);
	  fwrite(logLine, 1, strlen(logLine), fp);
	  // ...
	}
      //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
      else if(!strncmp(recvBuf, "cost", 4))
	{
	  //TODO record the cost change (remember, the link might currently be down! in that case,
	  //this is the new cost you should treat it as having once it comes back up.)
	  // ...
	  int new_cost = ntohl(*(int*)(data_in + 4 + sizeof(short int)));
	  neighbor_node* curNeighbor;
	  if((curNeighbor = (neighbor_node*)linkedlist_getNeighbor(first_neighbor, destID)) != NULL) {
	    curNeighbor->neighbor_cost->cost = new_cost;
	    updateLSP2neighbors(-1);
	  }
	  else {
	    curNeighbor = setneighbor(destID, new_cost);
	    first_down_neighbor = linkedlist_update(first_down_neighbor, destID, curNeighbor);
	  }
	}
		
      //TODO now check for the various types of packets you use in your own protocol
      else if(!strncmp(recvBuf, "dest", 4)) {
	char*msg = (char*)malloc(bytesRecvd - 6 + 1);
	memcpy(msg, data_in + 4 + sizeof(short int), bytesRecvd - 6);
	msg[bytesRecvd - 6] = '\0';
	if(destID == globalMyID) {
	  sprintf(logLine, "receive packet message %s\n", msg);
	}
	else {
	  run_dijkstra();
	  int next_hop = cost_table[destID]->next_hop; 
	  if(sendto(globalSocketUDP, recvBuf, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[next_hop], sizeof(globalNodeAddrs[next_hop])) < 0)
	    perror("sendto()");
	  sprintf(logLine, "forward packet dest %d nexthop %d message %s\n", destID, next_hop, msg);
	}
	printf("%s", logLine);
	fwrite(logLine, 1, strlen(logLine), fp);
		
      }

      else if(!strncmp(recvBuf, "LSP", 3)) {
	pthread_mutex_lock(&list_m);
	LSP* new_LSP = getLSP(data_in);
	int LSP_source = new_LSP->node_id;
	if(LSP_list[LSP_source] == NULL) {
	  LSP_list[LSP_source] = new_LSP;
	  forwardLSP(recvBuf, bytesRecvd, heardFrom);
	  // printf("LSP null\n");
	}
	else if(LSP_list[LSP_source]->sequence_num < getSequenceNum(data_in)) {
	  LSP_list[LSP_source] = new_LSP;
	  //printf("LSP new version\n"
	  forwardLSP(recvBuf, bytesRecvd, heardFrom);
	}
	pthread_mutex_unlock(&list_m);
      }
      fflush(fp);
    }
  // ... 
  //(should never reach here)
  close(globalSocketUDP);
}

