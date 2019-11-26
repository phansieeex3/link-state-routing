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
#include "monitor_neighbors.h"



//my link list root
neighbor_list* first_neighbor;
neighbor_list* first_next_neighbor;

int contains(neighbor_list* root, int neighbor_id) {
  neighbor_list* current = root;
  while(current != NULL) {
    if(current->neighbor_weight->next_node == neighbor_id) {
      return 1;
    }
    current = current->next;
  }
  return 0;
}

//inserting my new neighbors in my link list
neighbor_node* insert(neighbor_list* root, neighbor_list* new_node) {

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
//deleting a neighbor_node
neighbor_list* delete(neighbor_list* root, int neighbor_id) {
  neighbor_list* current = root;
  while(current != NULL) {
    if(current->neighbor_weight->next_node == neighbor_id) {

      if(current->prev == NULL) {
		root = current->next;
      }
      if(current->next == NULL) {
			current->next = NULL;
      }
      else {
		current->next = current->next;
		current->next->prev = current->prev;
      }
    	current = current->next;

  
    }
  }
     return root;
}


//update
neighbor_list* update(neighbor_list* head, int neighbor_id, neighbor_node* new_node) {
  head = linkedlist_deleteNeighbor(head, neighbor_id);
  head = linkedlist_insert(head, new_node);
  return head;
}


  //printf("root == NULL %d\n", root == NULL);
   

//get my neighbor 
neighbor_list* getNeighbor(neighbor_list* root, int neighbor_add) {
	//dummy pointer
  neighbor_list* current = root;

	//looping through my list
  while(current != NULL) { //while not the end of my linklist

    if(current->neighbor_weight->next_node == neighbor_add) {
      return current;
    }
    current = current->next;
  }

	return root;
}




link_state_node* createNode(int destination, int neighbor_node, int prev_node, int weight) {
  link_state_node* lsn = (link_state_node*)malloc(sizeof(link_state_node));
    lsn->neighbor_nodes->prev = prev_node;
 //weight
  lsn->neighbor_nodes->neighbor_weight->weight = weight;
//destination id
  lsn->destination_ID = destination;
  //next
  lsn->neighbor_nodes->neighbor_weight->next_node = neighbor_node;
  
 
  return lsn;
}


//forward LSAs
void forwardLSA(char* recvBuf, int bytesRecvd, int heardFrom) {

if (recvBuf !=NULL) printf("resume, recvbuf %s", recvBuf);
  int node_id = ntohl(* ( (int*) (recvBuf + 3) ) );
  neighbor_list* current = root_neighbor; //first neighbor in our list
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

  int buf = neighbor * sizeof(neighbor_node)+ 3 + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int);
  neighbor_list* current = root_neighbor; //first neighbor in our list

  while(current != NULL) {
	  int destination = current->neighbor_weight->next_node;

	  if (destination != neighbor) 
	  {
		   if(sendto(globalSocketUDP, sendBuf, buf, 0, (struct sockaddr*)&globalNodeAddrs[destination], sizeof(globalNodeAddrs[destination])) > 0) 
			{
				printf("forwarded LSA\n");
			}
			else {
				perror("ForwardLSA error");
			}

	  }

	  //incurrement my current neighbor_node
	  current = current->next;
  }
  sequence_number +=1;



}

  
// message format LSA : id, sequence, neigh, neighbor_node (next_node, weight) format

//convert LSAs
LSA* convertLSA(void* buff) {
	LSA* lsa = (LSA*) malloc(sizeof(LSA)); //casting 

	int destination = ntohl( * ((int*) (buff+3) ) );
	lsa->node_ID = destination;
	
	int sequence = ntohl(* ( ( int* )(buff + 3 + sizeof(int))));
	lsa->sequence_number = sequence;
	
	int neighbor_size_new = ntohl(*((int*) (buff + 3 + 2 * sizeof(int))));
	lsa->neighbor_size= neighbor_size_new ;

	//create neighbor neighbor_node list
	neighbor_list* neighbor_list =  malloc(neighbor_size * sizeof(neighbor_node));
	lsa->neighbor = NULL; 

	int i = 0;

	int a; //java = 0, c= NULL

	for(i; i< neighbor_size; i++) {
	int next_node = ((neighbor_node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(neighbor_node)))->next_node;
    int neigh_weight = ((neighbor_node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(neighbor_node)))->weight;
    lsa->neighbor = insert(lsa->neighbor, setneighbor(next_node, neigh_weight));

	}

 return lsa;



}

// message format LSA : id, sequence, neigh, neighbor_node (next_node, weight) format
/*
LSA* parseLSAforReceving(void* buff) {

  LSA* lsp = (LSA*)malloc(sizeof(LSA));
 lsa->neighbor_size = ntohl(*((int*)(buff + 3 + 2 * sizeof(int))));
 //id, sequence, neighbor number, neighbor_node
  lsp->node_id = ntohl(*((int*)(buff + 3)));
  lsa->sequence_num = getSequenceNum(buff);

  neighbor_node* new_neighbor_list = (neighbor_node*)malloc(sizeof(neighbor_node));
  lsa->first_neighbor = NULL;
  	int i = 0;

	for(i; i< neighbor_size; i++) {
	int next_node = ((neighbor_node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(neighbor_node)))->next_node;
    int neigh_weight = ((neighbor_node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(neighbor_node)))->weight;
    lsa->first_neighbor = insert(lsa->first_neighbor, setneighbor(next_node, neigh_weight));

	}
  return lsa;
}*/



neighbor_node* uppdate(neighbor_node* root, int neigbor_add, neighbor_node* new_node) {
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

int getSequenceNum(void* LSPbuf) {
  int sn = ntohl(*((int*)(LSPbuf + 3 + sizeof(int))));
  return sn;
}


void* announceToNeighbors(void* unusedParam)
{
	struct timespec sleepFor;
	sleepFor.tv_sec = 500*1000;

   while(1)
    {
      char* sendBuf = convertLSA("");
      int bufLen = 3 + sizeof(int) + sizeof(int) + sizeof(int) + neighbor_size * sizeof(neighbor_node);
      hackyBroadcast(sendBuf, bufLen);
      pthread_mutex_lock(&sequence_number_thread);
      sequence_number++;
      pthread_mutex_unlock(&sequence_number_thread);
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
      neighbor_list* current = first_neighbor;
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
	char log[1024];

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
			
			pthread_mutex_lock(&list_operation_thread);
			if(!contains(first_neighbor, heardFrom)) { // or sequence is higher.....
				//printf("setup neighbor_node %d\n", heardFrom);
				setupNode(heardFrom); 
			}
			pthread_mutex_unlock(&list_operation_thread);
					//TODO: this neighbor_node can consider heardFrom to be directly connected to it; do any such logic now.
			
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
				memcpy(message, "dest", 4);
				short int destID_forward = htons(destID);
	   			 memcpy(sending_data + 4, &destID_forward, sizeof(short int));
	   		 	memcpy(sending_data + 4 + sizeof(short int), message, strlen(message));

				//if(contains(topology, destID) < 0) "destination unreachable"
				if(sendto(globalSocketUDP, sending_data, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[next_neighbor->next_node], sizeof(globalNodeAddrs[next_neighbor->next_node])) < 0) perror("cannot send message in send");
	   			 sprintf(log, "send dest %d nexthop %d message %s\n", destID_forward, next_neighbor, message);
	 			 }


		}
		else if(!strncmp(recvBuf, "dest", 4)) {
			//TODO send the requested message to the requested destination neighbor_node
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
				memcpy(message, "dest", 4);
				short int destID_forward = htons(destID);
	   			 memcpy(sending_data + 4, &destID_forward, sizeof(short int));
	   		 	memcpy(sending_data + 4 + sizeof(short int), message, strlen(message));

				//if(contains(topology, destID) < 0) "destination unreachable"
				if(sendto(globalSocketUDP, sending_data, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[next_neighbor->next_node], sizeof(globalNodeAddrs[next_neighbor->next_node])) < 0) perror("cannot send message in send");
	   			 sprintf(log, "send dest %d nexthop %d message %s\n", destID, next_neighbor, message);
	 			 }
			//convert ip address 
			//check if in table and part of my toplogy
			//if not then it is untreachable then log it

			//else
			//try to sendto destination and log it


			}

			

		
		//'weight'<4 ASCII bytes>, destID<net order 2 byte signed> newweight<net order 4 byte signed>
		//new weight to your neighbor
		else if(!strncmp(recvBuf, "cost", 4)) {
			//update my neighbors with new cost
			int new_weight = ntohl(*(int*)(data_pt + 4 + sizeof(short int)));
			neighbor_node* curNeighbor;

			if((curNeighbor = (neighbor_node*) getNeighbor(first_neighbor, destID)) != NULL) {
				
				curNeighbor->weight = new_weight;
				updateLSAtoNeighbors(-1);
			} 
			else {
				curNeighbor = setNeighbor(destID, new_weight);
				first_next_neighbor = update(first_next_neighbor, destID, curNeighbor);
			}

			//if neighbor_node cost contains in list then replace with new neighbor_node cost
			//delete neighbor, insert new neighbor with it's new cost

			//if not contains then add new neighbor and neighbor_node cost

		}

		else if(!strncmp(recvBuf, "LSA", 3)) {
			//get lsa information
			//if new LSA or sequence is greater than my current sequence
			//forward my LSA
		
			LSA* new_LSA = convertLSA(data_pt);

			int LSA_id = new_LSA->node_ID;

			int LSA_neigh_size = new_LSA->neighbor_size;
			int LSA_sequence_number = new_LSA->sequence_number;
			if(LSA_list[LSA_id] == NULL || LSA_sequence_number < getSequenceNum(data_pt)) 
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
		close(globalSocketUDP);
	}
	//(should never reach here)
	
