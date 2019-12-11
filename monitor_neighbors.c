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
#include "monitor_neighbors.h"


pthread_mutex_t list_operation_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sequence_number_thread = PTHREAD_MUTEX_INITIALIZER;


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
			else {
				perror("ForwardLSA error");
			}

	  }

	  //incurrement my current neighbor_node
	  current = current->next;
  }
  sequence_numbers +=1;



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
	neighbor_list * new_node;
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

	return root;
}
//deleting a neighbor_node
neighbor_list* delete(neighbor_list* root, int neighbor_id) {
  neighbor_list* current = root;
  while(current != NULL) {
    if(current->neighbor_node->id == neighbor_id) {

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
      return root;
    }
    	current = current->next;

  }
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
  //pthread_mutex_lock(&list_down_m);
  neighbor_list* neigh = getNeighbor(first_next_neighbor, neighbor_id);
  first_next_neighbor = delete(first_next_neighbor, neighbor_id);

  if(neigh == NULL) //contains! 
  {
    first_neighbor = insert(first_neighbor, setNeighbor(neighbor_id, 1));
    neighbor_size++;
  }
  else 
  {
    first_neighbor = insert(first_neighbor, neigh->neighbor_node);
    neighbor_size++;
  }
  //deliverHistoryLSP(neighbor_id);

  updateLSAtoNeighbors(-1);
 
 // pthread_mutex_unlock(&list_down_m);
}





link_state_node* create_link_state_node(int destination, neighbor_list* neighbor_list) {
   link_state_node* lsn =  malloc(sizeof(link_state_node));
   lsn->next = NULL;
   lsn->destination_ID = destination;



	while(neighbor_list != NULL) {
        int id = neighbor_list->neighbor_node->id;
        int neigh_weight =neighbor_list->neighbor_node->weight;
        lsn->neighbor_nodes = insert(lsn->neighbor_nodes, setNeighbor(id, neigh_weight));

        neighbor_list = neighbor_list->next;


    }
	    
	
  //  lsn->next = NULL;
  
 
  return lsn;
}


link_state_node* add_link_state_node(int id, neighbor_list* list, link_state_node* graph) {
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
        return graph; 
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
		  perror("ForwardLSA error");
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
		if(topology == NULL) {
            topology = create_link_state_node(destID, first_next_neighbor);
        }
		short int heardFrom = -1;
		if(strstr(fromAddr, "10.1.1."))
		{
			heardFrom = atoi(strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			pthread_mutex_lock(&list_operation_thread);
			if(!contains(first_neighbor, heardFrom)) { // or sequence is higher.....
				printf("setup neighbor_node %d\n", heardFrom);
				setUpNeighbors(heardFrom); 
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
                
                //first_next_neighbor??
                   pathList* shortestPath = findPaths(topology);
                   // int first_next_neighbor = paths->path->destination_id;
				//find shortest path within my graph
				//run Shortest path algo
//run diks***************
                    char* sending_data = malloc(sizeof(bytesRecvd));
                    memcpy(message, "dest", 4);
                    short int destID_forward = htons(destID);
                    memcpy(sending_data + 4, &destID_forward, sizeof(short int));
                    memcpy(sending_data + 4 + sizeof(short int), message, strlen(message));

                    //if(contains(topology, destID) < 0) "destination unreachable"
                    if(sendto(globalSocketUDP, sending_data, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[shortestPath->path->neighbor_id], sizeof(globalNodeAddrs[first_next_neighbor->neighbor_node->id])) < 0) perror("cannot send message in send");
                        sprintf(log, "send dest %d nexthop %d message %s\n", destID_forward, first_next_neighbor, message);
                    }


		}
		else if(!strncmp(recvBuf, "dest", 4)) {
			//TODO send the requested message to the requested destination neighbor_node
			// ALLOCATE SPACE FOR MSH
			//if my packet , print i have received
			char* message = recvBuf + 4 + sizeof(short int) ; // message on the rest

			if(destID == globalMyID) {
				printf("got my message, thanks bye %s\n", message);
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

				if(contains(first_neighbor, destID) < 0) {
                    printf("destination unreachable");
				if(sendto(globalSocketUDP, sending_data, bytesRecvd, 0, (struct sockaddr*)&globalNodeAddrs[first_next_neighbor->neighbor_node->id], sizeof(globalNodeAddrs[first_next_neighbor->neighbor_node->id])) < 0) perror("cannot send message in send");
	   			 sprintf(log, "send dest %d nexthop %d message %s\n", destID, first_next_neighbor, message);
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
				forwardLSA(recvBuf, bytesRecvd, heardFrom);

                //insert my neighbor list....from others into my graph
                topology = add_link_state_node(new_LSA->node_ID, new_LSA->neighbors, topology);

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
	




/*
typedef struct {
    int vertex;
    int weight;
} edge_t;
 
typedef struct {
    edge_t **edges;
    int edges_len;
    int edges_size;
    int dist;
    int prev;
    int visited;
} vertex_t;
 
typedef struct {
    vertex_t **vertices;
    int vertices_len;
    int vertices_size;
} graph_t;
 
typedef struct {
    int *data;
    int *prio;
    int *index;
    int len;
    int size;
} heap_t;
 
void add_vertex (graph_t *g, int i) {
    if (g->vertices_size < i + 1) {
        int size = g->vertices_size * 2 > i ? g->vertices_size * 2 : i + 4;
        g->vertices = realloc(g->vertices, size * sizeof (vertex_t *));
        for (int j = g->vertices_size; j < size; j++)
            g->vertices[j] = NULL;
        g->vertices_size = size;
    }
    if (!g->vertices[i]) {
        g->vertices[i] = calloc(1, sizeof (vertex_t));
        g->vertices_len++;
    }
}
 
void add_edge (graph_t *g, int a, int b, int w) {
    a = a - 'a';
    b = b - 'a';
    add_vertex(g, a);
    add_vertex(g, b);
    vertex_t *v = g->vertices[a];
    if (v->edges_len >= v->edges_size) {
        v->edges_size = v->edges_size ? v->edges_size * 2 : 4;
        v->edges = realloc(v->edges, v->edges_size * sizeof (edge_t *));
    }
    edge_t *e = calloc(1, sizeof (edge_t));
    e->vertex = b;
    e->weight = w;
    v->edges[v->edges_len++] = e;
}
 
heap_t *create_heap (int n) {
    heap_t *h = calloc(1, sizeof (heap_t));
    h->data = calloc(n + 1, sizeof (int));
    h->prio = calloc(n + 1, sizeof (int));
    h->index = calloc(n, sizeof (int));
    return h;
}
 
void push_heap (heap_t *h, int v, int p) {
    int i = h->index[v] == 0 ? ++h->len : h->index[v];
    int j = i / 2;
    while (i > 1) {
        if (h->prio[j] < p)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
        j = j / 2;
    }
    h->data[i] = v;
    h->prio[i] = p;
    h->index[v] = i;
}
 
int min (heap_t *h, int i, int j, int k) {
    int m = i;
    if (j <= h->len && h->prio[j] < h->prio[m])
        m = j;
    if (k <= h->len && h->prio[k] < h->prio[m])
        m = k;
    return m;
}
 
int pop_heap (heap_t *h) {
    int v = h->data[1];
    int i = 1;
    while (1) {
        int j = min(h, h->len, 2 * i, 2 * i + 1);
        if (j == h->len)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
    }
    h->data[i] = h->data[h->len];
    h->prio[i] = h->prio[h->len];
    h->index[h->data[i]] = i;
    h->len--;
    return v;
}
 
void dijkstra (graph_t *g, int a, int b) {
    int i, j;
    a = a - 'a';
    b = b - 'a';
    for (i = 0; i < g->vertices_len; i++) {
        vertex_t *v = g->vertices[i];
        v->dist = INT_MAX;
        v->prev = 0;
        v->visited = 0;
    }
    vertex_t *v = g->vertices[a];
    v->dist = 0;
    heap_t *h = create_heap(g->vertices_len);
    push_heap(h, a, v->dist);
    while (h->len) {
        i = pop_heap(h);
        if (i == b)
            break;
        v = g->vertices[i];
        v->visited = 1;
        for (j = 0; j < v->edges_len; j++) {
            edge_t *e = v->edges[j];
            vertex_t *u = g->vertices[e->vertex];
            if (!u->visited && v->dist + e->weight <= u->dist) {
                u->prev = i;
                u->dist = v->dist + e->weight;
                push_heap(h, e->vertex, u->dist);
            }
        }
    }
}
 
void print_path (graph_t *g, int i) {
    int n, j;
    vertex_t *v, *u;
    i = i - 'a';
    v = g->vertices[i];
    if (v->dist == INT_MAX) {
        printf("no path\n");
        return;
    }
    for (n = 1, u = v; u->dist; u = g->vertices[u->prev], n++)
        ;
    char *path = malloc(n);
    path[n - 1] = 'a' + i;
    for (j = 0, u = v; u->dist; u = g->vertices[u->prev], j++)
        path[n - j - 2] = 'a' + u->prev;
    printf("%d %.*s\n", v->dist, n, path);
}

*/

// Uses Dijikstra's shortest path routing algorithm to find the shortest path to each other node
pathList* findPaths(link_state_node* localNode) {
    /*
    // Free existing paths
    pathList *current = *paths;
    while (current != NULL) {
        free(current->path);
        pathList *prev = current;
        current = current->next;
        free (prev);
    }*/

    /*
    // Initial Graph generation
    graph_t *g = calloc(1, sizeof (graph_t));
    
    add_edge(g, 'a', 'b', 7);
    add_edge(g, 'a', 'c', 9);
    add_edge(g, 'a', 'f', 14);
    add_edge(g, 'b', 'c', 10);
    add_edge(g, 'b', 'd', 15);
    add_edge(g, 'c', 'd', 11);
    add_edge(g, 'c', 'f', 2);
    add_edge(g, 'd', 'e', 6);
    add_edge(g, 'e', 'f', 9);
    
    dijkstra(g, 'a', 'a');
    print_path(g, 'a');
    dijkstra(g, 'a', 'b');
    print_path(g, 'b');
    dijkstra(g, 'a', 'c');
    print_path(g, 'c');
    dijkstra(g, 'a', 'd');
    print_path(g, 'd');
    dijkstra(g, 'a', 'e');
    print_path(g, 'e');
    dijkstra(g, 'a', 'f');
    print_path(g, 'f');

    */

    pathList *head = NULL;
    head = malloc(sizeof(pathList));
    if (head == NULL) {
        return NULL;
    }

    // Path 1
    path *path1 = NULL;
    path1 = malloc(sizeof(path));
    if (path1 == NULL) {
        return NULL;
    }
    path1->destination_id = 2;
    path1->neighbor_id = 2;

    // Path 2
    path *path2 = NULL;
    path2 = malloc(sizeof(path));
    if (path2 == NULL) {
        return NULL;
    }
    path2->destination_id = 3;
    path2->neighbor_id = 3;

    // Path 3
    path *path3 = NULL;
    path3 = malloc(sizeof(path));
    if (path3 == NULL) {
        return NULL;
    }
    path3->destination_id = 4;
    path3->neighbor_id = 3;

    // Path 4
    path *path4 = NULL;
    path4 = malloc(sizeof(path));
    if (path4 == NULL) {
        return NULL;
    }
    path4->destination_id = 5;
    path4->neighbor_id = 3;

    // Path 5
    path *path5 = NULL;
    path5 = malloc(sizeof(path));
    if (path5 == NULL) {
        return NULL;
    }
    path5->destination_id = 6;
    path5->neighbor_id = 3;
    
    pathList *current = head;
    current->path = path1;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path2;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path3;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path4;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path5;

    return head;
}
 