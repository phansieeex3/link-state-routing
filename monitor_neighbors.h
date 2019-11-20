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
#include <message_objects.h>


extern int globalMyID;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];


//neighbor node 
extern neighbor_node* root_neighbor;
extern int neighbor_number;
extern neighbor_node* next_neighbor;
extern int neighbor_size;
extern int sequence_number;

extern node** weight_table;
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


//set my neighbor if received

node* setNeighbor(int next_node, int weight);
//setting my node
node* setNode(int next_node, int weight);


int contains(node* root, int neighbor_id) {
  node* current = root;
  while(current != NULL) {
    if(current->neighbor_weight->next_node == neighbor_id) {
      return 1;
    }
    current = current->next;
  }
  return 0;
}

//inserting my new neighbors in my link list
node* insert(node* root, node* new_node) {

	new_node->next = NULL;
	new_node->prev = NULL;
	//begin my link list
	if(root == NULL) {
		root = new_node;
	}

	//if not null, then insert
	else {
		new_node->next = root;
		root->prev = new_node;
		root = new_node;
	}

	//return where i am currently
	return root;


}
//deleting a node
node* delete(node* root, int neighbor_id) {
  node* current = root;
  while(current != NULL) {
    if(current->neighbor_weight->node == neighbor_id) {

      if(current->prev == NULL) {
		root = current->next;
      }
      else if(current->next == NULL) {
			current->prev->next = NULL;
      }
      else {
		current->prev->next = current->next;
		current->next->prev = current->prev;
      }
      return root;
    }
    current = current->next;
  }
  //printf("root == NULL %d\n", root == NULL);
  return 

//get my neighbor 
node* getNeighbor(node* root, int neighbor_add) {
	//dummy pointer
  node* current = root;

	//looping through my list
  while(current != NULL) { //while not the end of my linklist

    if(current->neighbor_weight->node == neighbor_add) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

root;
}

link_state_node* createNode(int destination, int node, int prev_node, int weight) {
  link_state_node* lsn = (link_state_node*)malloc(sizeof(link_state_node));
    lsn->prev_node = prev_node;
 //weight
  lsn->weight = weight;
//destination id
  lsn->destination_ID = destination;
  //next
  lsn->node = node;
  
 
  return lsn;
}


//forward LSAs
void forwardLSA(char* recvBuf, int bytesRecvd, int heardFrom) {

if (recvBuf !=NULL) printf("resume, recvbuf %s", recvBuf);
  int node_id = ntohl(* ( (int*) (recvBuf + 3) ) );
  neighbor_node* current = root_neighbor; //first neighbor in our list
  while(current != NULL) 
  {
    int neighbor_id = current->neighbor_weight->next_node;

    if(neighbor_id != heardFrom && neighbor_id != node_id && heardFrom != NULL) 
	{
      if(sendto(globalSocketUDP, recvBuf, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[neighbor_id], sizeof(globalNodeAddrs[neighbor_id])) > 0) 
	  {
		printf("forwarded LSA\n");
      }
	  else {
		  perror("ForwardLSA error");
	  }
    }
    current = current->next;
  }
}
//update LASs
void* updateLSAtoNeighbors(int neighbor) {
  char* sendBuf = parseLSA();

  int buf = neighbor * sizeof(node)+ 3 + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) 
  neighbor_node* current = root_neighbor; //first neighbor in our list

  while(current != NULL) {
	  int destination = current->neighbor_weight->next_node;

	  if (destination != neighbor) 
	  {
		   if(sendto(globalSocketUDP, recvBuf, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[neighbor_id], sizeof(globalNodeAddrs[neighbor_id])) > 0) 
			{
				printf("forwarded LSA\n");
			}
			else {
				perror("ForwardLSA error");
			}

	  }

	  //incurrement my current node
	  current = current->next;
  }
  sequence_number +=1;



}

  
// message format LSA : id, sequence, neigh, node (next_node, weight) format

//convert LSAs
LSA* convertLSA(void* buff) {
	LSA* lsa = (LSA*) malloc(sizeofLSA); //casting 

	int destination = ntohl( * ((int*) (buff+3) ) );
	lsa->node_ID = destination;
	
	int sequence = ntohl(* ( ( int* )(buff + 3 + sizeof(int))));
	lsa->sequence_number = sequence;
	
	int neighbor_size = ntohl(*((int*) (buff + 3 + 2 * sizeof(int))));
	lsa->neighbor_size(neighbor_size);

	//create neighbor node list
	neighbor_node* neighbor_list = neighbor_size * malloc(sizeof(_neighbor_node));
	lsa->neighbor = NULL; 

	int i = 0;

	for(i; i< neighbor_size; i++) {
	int next_node = ((node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(node)))->next_node;
    int neigh_weight = ((node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(node)))->weight;
    lsa->first_neighbor = insert(lsa->first_neighbor, setneighbor(next_node, neigh_weight));

	}

 return lsa;



}

// message format LSA : id, sequence, neigh, node (next_node, weight) format
/*
LSA* parseLSAforReceving(void* buff) {

  LSA* lsp = (LSA*)malloc(sizeof(LSA));
 lsa->neighbor_size = ntohl(*((int*)(buff + 3 + 2 * sizeof(int))));
 //id, sequence, neighbor number, node
  lsp->node_id = ntohl(*((int*)(buff + 3)));
  lsa->sequence_num = getSequenceNum(buff);

  neighbor_node* new_neighbor_list = (neighbor_node*)malloc(sizeof(neighbor_node));
  lsa->first_neighbor = NULL;
  	int i = 0;

	for(i; i< neighbor_size; i++) {
	int next_node = ((node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(node)))->next_node;
    int neigh_weight = ((node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(node)))->weight;
    lsa->first_neighbor = insert(lsa->first_neighbor, setneighbor(next_node, neigh_weight));

	}
  return lsa;
}*/



node* uppdate(node* root, int neigbor_add, node* new_node) {
  root = delete(root, neigbor_add);
  root = insert(root, new_node);
  return root;
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


void* announceToNeighbors(void* unusedParam)
{
	struct timespec sleepFor;
	sleepFor.tv_sec = 500*1000;

   while(1)
    {
      char* sendBuf = convertLSA(-1);
      int bufLen = 3 + sizeof(int) + sizeof(int) + sizeof(int) + neighbor_size * sizeof(node);
      hackyBroadcast(sendBuf, bufLen);
      pthread_mutex_lock(&sequence_num_m);
      sequence_num++;
      pthread_mutex_unlock(&sequence_num_m);
      nanosleep(&sleepFor, 0);
    }
}

//TODO change stuff here
void* monitorNeighbors(void* unusedParam) {
  struct timespec sleepFor;
  sleepFor.tv_sec = 0;
  sleepFor.tv_nsec = 650 * 1000 * 1000; //650 ms
  while(1)
    {
      node* current = first_neighbor;
      struct timeval tempTime;
      while(current != NULL) {
	int neighbor_id = current->neighbor_weight->next_node;
	gettimeofday(&tempTime, 0);
	if(tempTime.tv_sec - globalLastHeartbeat[neighbor_id].tv_sec > 1) {
	  teardownNode(neighbor_id);
	}
	current = current->next;
      }
      nanosleep(&sleepFor, 0);
    }
}


short int getDestination(char* buff) {
	short int ip = ntohs(*((short int*)(buff + 4)));
	return ip;
}

void listenForNeighbors()
{
	char fromAddr[100];
	struct sockaddr_in theirAddr;
	socklen_t theirAddrLen;
	unsigned char recvBuf[1000];

	int bytesRecvd;
	while(1)
	{
		theirAddrLen = sizeof(theirAddr);
		if ((bytesRecvd = recvfrom(globalSocketUDP, recvBuf, 1000 , 0, 
					(struct sockaddr*)&theirAddr, &theirAddrLen)) == -1)
		{
			perror("connectivity listener: recvfrom failed");
			exit(1);
		}
		
		void* data_pt = (void*) recvBuf;

		short int destID = getDestination(data_pt);

		inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);
		
		short int heardFrom = -1;
		if(strstr(fromAddr, "10.1.1."))
		{
			heardFrom = atoi(strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			pthread_mutex_lock(&list_m);
			if(!contains(first_neighbor, heardFrom)) { // or sequence is higher.....
				//printf("setup node %d\n", heardFrom);
				setupNode(heardFrom); 
			}
			pthread_mutex_unlock(&list_m);
					//TODO: this node can consider heardFrom to be directly connected to it; do any such logic now.
			
			//record that we heard from heardFrom just now.
			gettimeofday(&globalLastHeartbeat[heardFrom], 0);
		}
		
		//Is it a packet from the manager? (see mp2 specification for more details)
		//send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
		if(!strncmp(recvBuf, "send", 4))
		{
			//send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
			char* message = recvBuf + 4 + sizeof(short int) ; // message on the rest
//run diks***************
			if(destID == globalMyID) {
				printf("got my message, thanks bye %s\n", message);
			}
			else {
				//find shortest path within my graph
				//run Shortest path algo
//run diks***************
				char* sending_data = malloc(sizeof(bytesRecvd));
				memcpy(message, "dest");
				short int destID_forward = htons(destID);
	   			 memcpy(sending_data + 4, &destID_forward, sizeof(short int));
	   		 	memcpy(sending_data + 4 + sizeof(short int), message, strlen(message));

				//if(contains(topology, destID) < 0) "destination unreachable"
				if(sendto(globalSocketUDP, sending_data, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[next_neighbor], sizeof(globalNodeAddrs[next_neighbor])) < 0) perror("cannot send message in send");
	   			 sprintf(logLine, "send dest %d nexthop %d message %s\n", destID, next_neighbor, message);
	 			 }


		}
		else if(!strncmp(recvBuf, "dest", 4)) {
			//TODO send the requested message to the requested destination node
			// ALLOCATE SPACE FOR MSH
			//if my packet , print i have received
			char* message = recvBuf + 4 + sizeof(short int) ; // message on the rest

			if(destID == globalMyID) {
				printf("got my message, thanks bye %\n", message);
			}
			else {
				//else send to destination
			//caclulate path with dijstra
					//run diks***************
				char* sending_data = malloc(sizeof(bytesRecvd));
				memcpy(message, "dest");
				short int destID_forward = htons(destID);
	   			 memcpy(sending_data + 4, &destID_forward, sizeof(short int));
	   		 	memcpy(sending_data + 4 + sizeof(short int), message, strlen(message));

				//if(contains(topology, destID) < 0) "destination unreachable"
				if(sendto(globalSocketUDP, sending_data, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[next_neighbor], sizeof(globalNodeAddrs[next_neighbor])) < 0) perror("cannot send message in send");
	   			 sprintf(logLine, "send dest %d nexthop %d message %s\n", destID, next_neighbor, message);
	 			 }
			//convert ip address 
			//check if in table and part of my toplogy
			//if not then it is untreachable then log it

			//else
			//try to sendto destination and log it


			}

			

		}
		//'weight'<4 ASCII bytes>, destID<net order 2 byte signed> newweight<net order 4 byte signed>
		//new weight to your neighbor
		else if(!strncmp(recvBuf, "cost", 4)) {
			//update my neighbors with new cost
			int new_weight = ntohl(*(int*)(data_pt + 4 + sizeof(short int)));
			neighbor_node* curNeighbor;

			if((curNeighbor = (neighbor_node*) getNeighbor(first_neighbor, destID)) != NULL) {
				
				curNeighbor->neighbor_weight->weight = new_weight;
				updateLSAtoNeighbors(-1);
			}
			else {
				curNeighbor = setNeighbor(destID, new_weight);
				first_down_neighbor = update(first_down_neighbor, destID, curNeighbor);
			}

			//if node cost contains in list then replace with new node cost
			//delete neighbor, insert new neighbor with it's new cost

			//if not contains then add new neighbor and node cost

		}

		else if(!strncmp(recvBuf, "LSA", 4)) {
			//get lsa information
			//if new LSA or sequence is greater than my current sequence
			//forward my LSA
		
			LSA* new_LSA = convertLSA(data_pt);

			int LSA_id = new_LSA->node_ID;

			int LSA_neigh_size = new_LSA->neighbor_size;
			int LSA_sequence_number = new_LSA->sequence_number;
			if(LSA_list[LSA_id] == NULL or LSA_sequence_number < getSequenceNum(data_pt)) 
			{
				LSA_list[LSA_id] = new_LSA;
				forwardLSP(recvBuf, bytesRecvd, heardFrom);

			}
			
			//fflush(fp);
    }

		}
		
		//TODO now check for the various types of packets you use in your own protocol
		//else if(!strncmp(recvBuf, "your other message types", ))
		// ... 
	}
	//(should never reach here)
	close(globalSocketUDP);
}

