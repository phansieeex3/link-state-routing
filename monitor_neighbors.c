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
#include <limits.h>
#include "monitor_neighbors.h"



pthread_mutex_t list_operation_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sequence_number_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t list_thread = PTHREAD_MUTEX_INITIALIZER;

neighbor_node* setNeighbor(int id, int weight) {
    neighbor_node* new_node = (neighbor_node*) malloc(sizeof(neighbor_node));
    new_node->id = id;
    new_node->weight = weight;

    return new_node;

}

char* getMyLSA() {

    int i = 0;
  int bufLen = 3 + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + neighbor_size*sizeof(neighbor_node);
  char* sendBuf = (char*)malloc(bufLen);


  int id = htonl(globalMyID);
  int new_neighbor_num = htonl(neighbor_size);
  int new_sequence_num = htonl(sequence_numbers);
  strcpy(sendBuf, "LSA");
  memcpy(sendBuf + 3, &id, sizeof(int));

  memcpy(sendBuf + 3 + sizeof(int), &new_sequence_num, sizeof(int));

  memcpy(sendBuf + 3 + 2 * sizeof(int), &new_neighbor_num, sizeof(int));

  neighbor_list* current = first_neighbor;
  
  while(current != NULL) 
  {
    memcpy(sendBuf + 3 + 3 * sizeof(int) + i * sizeof(neighbor_node), current->neighbor_node, sizeof(neighbor_node));
    current = current->next;
    i++;

  }
  return sendBuf;


}

//update LASs
void updateLSAtoNeighbors(int neighbor) {
  char* sendBuf = getMyLSA();

  int buf = neighbor * sizeof(neighbor_node)+ 3 + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int);
  neighbor_list* current = first_neighbor; //first neighbor in our list

  while(current != NULL) {
	  int destination = current->neighbor_node->id;

	  if (destination != neighbor) 
	  {
		   if(sendto(globalSocketUDP, sendBuf, buf, 0, (struct sockaddr*)&globalNodeAddrs[destination], sizeof(globalNodeAddrs[destination])) > 0) 
			{
				printf("forwarded LSA\n");
			}
			//else {
				//perror("ForwardLSA error");
			//}

	  }

	  //incurrement my current neighbor_node
	  current = current->next;
  }

pthread_mutex_lock(&sequence_number_thread);
  sequence_numbers +=1;
pthread_mutex_unlock(&sequence_number_thread);


}

/*
neighbor_list* setUpNeighbors(int id, int weight) {

    //neighbor_node* new_node = (neighbor_node*) malloc(sizeof(neighbor_node));
    neighbor_list* list = (neighbor_list*) malloc(sizeof(neighbor_list));
    list->next = NULL;
    list->prev = NULL;
   
    neighbor_node* neighbor = setNeighbor(id, weight);

    //adding my neighbor
    list->neighbor_node = neighbor;

    return list;

}*/

//inserting my new neighbors in my link list
neighbor_list* insert(neighbor_list* root, neighbor_node* new_neighbor_node) {
	
	neighbor_list * new_node = (neighbor_list*) malloc(256*sizeof(neighbor_node));
   	new_node->next = NULL;
    	new_node->prev = NULL;
	new_node->neighbor_node = new_neighbor_node;

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



	//return root;


}






int contains(neighbor_list* root, int neighbor_id) {
  neighbor_list* current = root;
  while(current != NULL) {
    if(current->neighbor_node->id == neighbor_id) {
      return 1;
    }
    current = current->next;
  }
  return 0;
}

//for sending out my LSA





//get my neighbor 
neighbor_list* getNeighbor(neighbor_list* root, int neighbor_add) {
	//dummy pointer
  neighbor_list* current = root;

	//looping through my list
  while(current != NULL) { //while not the end of my linklist

    if(current->neighbor_node->id == neighbor_add) {
      return current;
    }
    current = current->next;
  }

	  

return NULL;
}


//deleting a neighbor_node
neighbor_list* delete(neighbor_list* root, int neighbor_id) {
  neighbor_list* current = root, *prev;

//delete my root
if(current != NULL && current->neighbor_node->id == neighbor_id) {
  root = current->next;
 free(current);
 return root;
}

  while(current != NULL && current->neighbor_node->id != neighbor_id) 
  {
    prev = current;
    current = current->next;

  }

  if(current == NULL ) return root; //did not find 
  
   prev->next = current->next;

	free(current);

return root;

}
  
    



//update
neighbor_list* update(neighbor_list* head, int neighbor_id, neighbor_node* new_node) {
  head = delete(head, neighbor_id); //delete and insert new info
  head = insert(head, new_node);
  return head;
}


  //printf("root == NULL %d\n", root == NULL);
   
void setUpNeighbors(int neighbor_id) {  

  pthread_mutex_lock(&list_operation_thread);

//nearest neigh
  neighbor_list* neigh = getNeighbor(first_next_neighbor, neighbor_id);

//delete
  first_next_neighbor = delete(first_next_neighbor, neighbor_id);

  if(neigh == NULL) //contains! 
  {
    first_neighbor = insert(first_neighbor, setNeighbor(neighbor_id, 1));
	printf("INSERTING SET UP NEIGHBORS NO NEIGHBOR");
    neighbor_size++;
  }
  else 
  {
    first_neighbor = insert(first_neighbor, neigh->neighbor_node);
	printf("INSERTING SET UP NEIGHBORS yes NEIGHBOR");
    neighbor_size++;
  }

  printf("NEIGHBOR SIZE    ======================== %d \n", neighbor_size);


  updateLSAtoNeighbors(0);
 
  pthread_mutex_unlock(&list_operation_thread);
}





link_state_node* create_link_state_node(int destination, neighbor_list* neighbor_list) {
   link_state_node* lsn =  malloc(sizeof(link_state_node));
   lsn->next = NULL;
   lsn->destination_ID = destination;

   if(neighbor_list == NULL) printf("CREATELINK STATE NODE CANNOT, NEIGHBORLIST IS NULL");
	while(neighbor_list != NULL) {
        int id = neighbor_list->neighbor_node->id;
        int neigh_weight =neighbor_list->neighbor_node->weight;
	printf(" 	CREATE LINK STATE NODE inserting to my topology id %d weight %d \n", id, neigh_weight); 
        lsn->neighbor_nodes = insert(lsn->neighbor_nodes, setNeighbor(id, neigh_weight));

        neighbor_list = neighbor_list->next;


    }
	    
	
  //  lsn->next = NULL;
  
 
  return lsn;
}


link_state_node* add_link_state_node(int id, neighbor_list* list, link_state_node* graph) {
	

	//int id = list->neighbor_node->id;
	//int weight = list->neighbor_node->weight;

	//neighbor_list* new_data = insert(graph->neighbor_nodes, setNeighbor(id, weight));

	link_state_node* new_link = create_link_state_node(id, list);
	printf(" 	ADDING LINK STATE NODE inserting to my topology id %d  \n", id); 

	//make new node as top of list
	new_link->next = graph;
	graph = new_link;


return graph;
	
	
		
    /*
link_state_node* head = graph;
    if(head->neighbor_nodes == NULL && head->next == NULL) {
        while(list != NULL) {
            int id = list->neighbor_node->id;
            int weight = list->neighbor_node->weight;
            head->neighbor_nodes = insert(head->neighbor_nodes, setNeighbor(id, weight));
            list->neighbor_node = list->next->neighbor_node;
        }
    } else {
          while(head->next != NULL) 
             head = head->next; // loop to the end, append the next link
          head->next = create_link_state_node(id, list);
        }
        return graph; */

	
   }    






//forward LSAs
void forwardLSA(char* recvBuf, int bytesRecvd, int heardFrom) {

if (recvBuf !=NULL) printf("resume, recvbuf %s", recvBuf);
  int node_id = ntohl(* ( (int*) (recvBuf + 3) ) );
  neighbor_list* current = first_neighbor; //first neighbor in our list
  while(current != NULL) 
  {
    int neighbor_id = current->neighbor_node->id;

    if(neighbor_id != heardFrom && neighbor_id != node_id && heardFrom != 0) 
	{
      if(sendto(globalSocketUDP, recvBuf, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[neighbor_id], sizeof(globalNodeAddrs[neighbor_id])) > 0) 
	  {
		printf("forwarded LSA\n");
      }
	  else {
		  perror("ForwardLSA error inside FORWARd lsa");
	  }
    }
    current = current->next;
  }
}







  
// message format LSA : id, sequence, neigh, neighbor_node (id, weight) format

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
	lsa->neighbors = NULL; 

	int i;

	 //java = 0, c= NULL

	for(i=0; i< neighbor_size; i++) {
	int id = ((neighbor_node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(neighbor_node)))->id;
    int neigh_weight = ((neighbor_node*)(buff + 3 + 3 * sizeof(int) + i * sizeof(neighbor_node)))->weight;
    lsa->neighbors = insert(lsa->neighbors, setNeighbor(id, neigh_weight));

	}

 return lsa;



}



//Yes, this is terrible. It's also terrible that, in Linux, a socket
//can't receive broadcast packets unless it's bound to INADDR_ANY,
//which we can't do in this assignment.
void hackyBroadcast(const char* buf, int length)
{
	//printf("hackybroadcast %s", buf);

	int i;
	for(i=0;i<256;i++)
		if(i != globalMyID) { //(although with a real broadcast you would also get the packet yourself)
			//sendto(globalSocketUDP, buf, length, 0,(struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]) );
		if(sendto(globalSocketUDP, buf, length, 0,(struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]) ) > 0) 
			  {
				printf("forwarded LSA\n");
		      }
			  //else {
				 // perror("ForwardLSA error");
			  //}
			}
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
      int bufLen = 3 + sizeof(int) + sizeof(int) + sizeof(int) + neighbor_size * sizeof(neighbor_node);
      
      char* sendBuf = getMyLSA();
      hackyBroadcast(sendBuf, bufLen);
      pthread_mutex_lock(&sequence_number_thread);
      sequence_numbers++;
      pthread_mutex_unlock(&sequence_number_thread);
      nanosleep(&sleepFor, 0);
    }
}

void teardownNode(int id) {
	printf("id");
}



void* monitorNeighbors(void* unusedParam) {
  struct timespec sleepFor;
  sleepFor.tv_sec = 0;
  sleepFor.tv_nsec = 650 * 1000 * 1000; //650 ms
  while(1)
    {
      neighbor_list* current = first_neighbor;
      struct timeval tempTime;
      while(current != NULL) {
	int neighbor_id = current->neighbor_node->id;
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
LSA_list = (LSA**) malloc(256* sizeof(LSA*));

int x;
for(x = 0; x< 256; x++) {
 LSA_list[x] = NULL;

}

printf("FILENAMEEEEEEE %s \n", filename);
char str[] = ".txt";
strncat(filename, &str, 4);
FILE* fp = fopen(filename , "w+");



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

		//short int destID = getDestination(data_pt);
		short int destID = ntohs(*((short int*) (data_pt+4)));
		printf("command : %s \n" , recvBuf);
		printf("destination ID %d \n", destID);

		inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);
		if(topology == NULL) {
            topology = create_link_state_node(destID, first_neighbor);
        }
		short int heardFrom = -1;
		if(strstr(fromAddr, "10.1.1."))
		{
			heardFrom = atoi(strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			pthread_mutex_lock(&list_thread);
			if(!contains(first_neighbor, heardFrom)) { // or sequence is higher.....
				printf("setup neighbor_node %d\n", heardFrom);
				setUpNeighbors(heardFrom); 
			}
			pthread_mutex_unlock(&list_thread);
					//TODO: this neighbor_node can consider heardFrom to be directly connected to it; do any such logic now.
			
			//record that we heard from heardFrom just now.
			gettimeofday(&globalLastHeartbeat[heardFrom], 0);
		}
		
		//Is it a packet from the manager? (see mp2 specification for more details)
		//send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
		if(!strncmp(recvBuf, "send", 4))
		{
			printf("SENDDDDDDD****** \n");
			//send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
			

			char* message = (char *) malloc (bytesRecvd);
			memcpy(message, data_pt + 4 + sizeof(short int ) , bytesRecvd - 6);
int msgLen = 4+sizeof(short int)+strlen(message);
			char* sendBuf = malloc(msgLen);
			
			message[bytesRecvd - 6] = '\0';
			
printf("************MESSAGE snd RECEIVED********* : %s \n" , message);

			if(destID == globalMyID) {
				sprintf(log, "receive packet message %s \n", message);
				printf("got my message, thanks bye %s\n", message);
			}else {
			
   	topology = create_link_state_node(destID, first_neighbor);
			calculateShortestPaths(topology);
		          
			    //char* sending_data = malloc(sizeof(bytesRecvd));
				char sending_data[4+sizeof(short int)+strlen(message)];
				//strcpy(sending_data, "dest");
			   memcpy(sending_data, "dest", 4);
			    short int destID_forward = htons(destID);
			printf("destID_forward %d \n", destID_forward);
			    memcpy(sending_data + 4, &destID_forward, sizeof(short int));
		printf("msg after destID_forward %s \n", sending_data);
			    memcpy(sending_data + 4 + sizeof(short int), message, strlen(message));
printf("msg after message %s \n", sending_data);
			    int shortest_next_hop = (int) topology->first_hop_ID;

		            if(contains(topology->neighbor_nodes, shortest_next_hop) == 10) {
				sprintf(log, "unreachable dest %d\n", destID);
				printf("%s", log);
				fwrite(log, 1, strlen(log), fp);

				}

printf("*******sebd*********SENDING DATA:  %s \n", sending_data);

                    if(sendto(globalSocketUDP, sending_data, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[shortest_next_hop], sizeof(globalNodeAddrs[shortest_next_hop])) < 0)
perror("cannot send message in send");
                        
           
				sprintf(log, "send dest %d nexthop %d message %s\n", destID_forward, shortest_next_hop, message);  

}

			

			printf("%s", log);
			fwrite(log, 1, strlen(log), fp);

		}



		else if(!strncmp(recvBuf, "dest", 4)) {
printf("DESTTTTTTT****** \n");
			//TODO send the requested message to the requested destination neighbor_node
			// ALLOCATE SPACE FOR MSH
			//if my packet , print i have received
			//char* message = (char *) malloc (bytesRecvd - 5);
			//memcpy(message, data_pt + 4 + sizeof(short int ) , bytesRecvd - 6);

			char* message = (char *) malloc (bytesRecvd  );
			memcpy(message, data_pt + 4 + sizeof(short int ) , bytesRecvd - 6);
			
			message[bytesRecvd - 6] = '\0';

			printf("************MESSAGE dest RECEIVED********* : %s \n" , message);
			if(destID == globalMyID) {
				sprintf(log, "receive packet message %s \n", message);
				printf("got my message, thanks bye %s\n", message);
			}
			else {

/*****
				char sendBuf[4+sizeof(short int)+sizeof(int)];
				strcpy(sendBuf, "cost");
				memcpy(sendBuf+4, &destID, sizeof(short int));
				memcpy(sendBuf+4+sizeof(short int), &no_newCost, sizeof(int));
				int ss;
WORK ON THISSSS MESSG FUCKED UP				memcpy(&ss, sendBuf+6, 4);
*/

				topology = create_link_state_node(destID, first_neighbor);
				calculateShortestPaths(topology);
				int shortest_next_hop = (int) topology->first_hop_ID;
				//char* sending_data = malloc(sizeof(bytesRecvd));
				/*
			char sending_data[4+sizeof(short int)+sizeof(int)];
				strcpy(sending_data, "send");
  				//memcpy(sending_data, "dest", 4);				
				short int destID_forward = htons(destID);
				printf("destID_forward %d \n", destID_forward);
	   			 memcpy(sending_data + 4, &destID_forward, sizeof(short int));
				printf("msg after destID_forward %s \n", sending_data);
	   		 	memcpy(sending_data + 4 + sizeof(short int), message, strlen(message));
				printf("msg after message %s \n", sending_data);
				printf("************dest****SENDING DATA:  %s \n", sending_data); */
			
				if(sendto(globalSocketUDP, recvBuf, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[shortest_next_hop], sizeof(globalNodeAddrs[shortest_next_hop])) < 0) perror("cannot send message in dest");
	   			 
	 			

sprintf(log, "send dest %d nexthop %d message %s\n", destID, shortest_next_hop, message);

}
			printf("%s", log);
			fwrite(log, 1, strlen(log), fp);
			//convert ip address 
			//check if in table and part of my toplogy
			//if not then it is untreachable then log it

			//else
			//try to sendto destination and log it


			

        }

			

		
		//'weight'<4 ASCII bytes>, destID<net order 2 byte signed> newweight<net order 4 byte signed>
		//new weight to your neighbor
		else if(!strncmp(recvBuf, "cost", 4)) {

printf("COOOOOSSSTTTT****** \n");
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

printf("LSAAAAAAAAAA****** \n");
			//get lsa information
			//if new LSA or sequence is greater than my current sequence
			//forward my LSA
			pthread_mutex_lock(&list_thread);
			LSA* new_LSA = convertLSA(data_pt);

			int LSA_id = new_LSA->node_ID;

			int LSA_neigh_size = new_LSA->neighbor_size;
			int LSA_sequence_number = new_LSA->sequence_number;
			if(LSA_list[LSA_id] == NULL || LSA_sequence_number < getSequenceNum(data_pt)) 
			{
				LSA_list[LSA_id] = new_LSA;
				forwardLSA(recvBuf, bytesRecvd, heardFrom);

                //insert my neighbor list....from others into my graph
                //topology = add_link_state_node(new_LSA->node_ID, new_LSA->neighbors, topology);
//pthread_mutex_unlock(&list_thread);
			}
			
			//fflush(fp);
    	}

		}

		
		
		fclose(fp);
		close(globalSocketUDP);
	}
	


void calculateShortestPaths(link_state_node *localNode) {
    // Write network data to file
    FILE *fp;
    fp = fopen("./network_data.txt", "w");
        // Iterate through linked_state_node's
    link_state_node *current = localNode;
    while (current != NULL) {
        fprintf(fp, "%d,", current->destination_ID);
	printf("in calculate shortest path current destID : %d \n" , current->destination_ID);
        neighbor_list *neighbors = current->neighbor_nodes;
        while (neighbors != NULL) {
            int id = neighbors->neighbor_node->id;
            int weight = neighbors->neighbor_node->weight;
            fprintf(fp, "%d,%d,", id, weight);
		//printf("in calculate shortest path current destID : %d \n" , current->destination_ID);
            neighbors = neighbors->next;
        }  
        fprintf(fp, "\n");
        current = current->next;
    }
    fclose(fp);

    // Run python code on file
    int py_response = system("python3 pathfinder.py");
    printf("Python Response: %d\n", py_response);

    // Read data from file and create route from it
    fp = fopen("./network_data.txt", "r");

    while (!feof(fp)) {
        int node_id = 0;
        int first_hop_id = 0;
        fscanf(fp, "%d:", &node_id);
        fscanf(fp, "%d", &first_hop_id);

        // Add to link_state_node list
        current = localNode;
        while (current != NULL) {
            if (current->destination_ID == node_id) {
                current->first_hop_ID = first_hop_id;
                break;
            }
            current = current->next;
        }
    }

    fclose(fp);
}
 
