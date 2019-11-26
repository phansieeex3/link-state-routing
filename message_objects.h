
#include <stdio.h>
#include <stdlib.h>//link state node that contains information of chain


//my current node or all nodes
typedef struct _neighbor_node {
  int id; // vertex
  int weight; //edge

} neighbor_node;

//my neighbor node list
typedef struct _neighbor_list {
  struct _neighbor_list* next;
  struct _neighbor_list* prev;
  neighbor_node* neighbor_node; 
} neighbor_list;


//link state node graph
typedef struct _link_state_node {
  int destination_ID; //vertex
 // int next_node;
 //int prev_node;
 // int weight; //edge
  //size_t pos;
  neighbor_list* neighbor_nodes;
  struct _link_state_node* next;
} link_state_node;





//link state announcement
typedef struct _LSA {
  int node_ID;
  int neighbor_size;
  int sequence_number;
  neighbor_node* neighbor;
} LSA;




