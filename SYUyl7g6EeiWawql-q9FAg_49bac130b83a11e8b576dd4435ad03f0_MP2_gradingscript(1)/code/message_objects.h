#ifndef MESSAGE_OBJECTS_H
#define MESSAGE_OBJECTS_H
#include <stdio.h>
#include <stdlib.h>//link state node that contains information of chain


//my current node or all nodes
typedef struct _neighbor_node {
  int id; // vertex
  int weight; //edge

} neighbor_node;

//my neighbor node list
typedef struct _neighbor_list {
 
  neighbor_node* neighbor_node; 
   struct _neighbor_list* next;
  struct _neighbor_list* prev;
} neighbor_list;


//link state node graph
typedef struct _link_state_node  {
  int destination_ID; //vertex
  neighbor_list* neighbor_nodes;
  struct _link_state_node* next;
} link_state_node;


//link state announcement
typedef struct _LSA {
  int node_ID;
  int neighbor_size;
  int sequence_number;
  neighbor_list* neighbors;
} LSA;



#endif
