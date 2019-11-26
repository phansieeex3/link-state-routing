
#include <stdio.h>
#include <stdlib.h>//link state node that contains information of chain


//my current node or all nodes
typedef struct _neighbor_node {
  int next_node; // vertex
  int weight; //edge
} neighbor_node;

//my neighbor node list
typedef struct _neighbor_list {
    struct _neighbor_list* next;
  struct _neighbor_list* prev;
  neighbor_node* neighbor_weight; 
} neighbor_list;


//link state node graph
typedef struct _link_state_node {
  int destination_ID; //vertex
 // int next_node;
 //int prev_node;
 // int weight; //edge
  //size_t pos;
  neighbor_list* neighbor_nodes;
} link_state_node;





//link state announcement
typedef struct _LSA {
  int node_ID;
  int neighbor_size;
  int sequence_number;
  neighbor_node* neighbor;
} LSA;

/*
_neighbor_node* setNode(int next_node, int weight) {
  neighbor_node* new_weight = (neighbor_node*)malloc(sizeof(node));
  (*new_weight).next_hop = next_node;
  (*new_weight).weight = weight;
  return new_weight;
}
neighbor_node* setneighbor(int next_node, int weight) {
  neighbor_node* new_node = setNode(next_node, weight);
  neighbor_node* new_node = (neighbor_node*)malloc(sizeof(neighbor_node));
  new_node->next = NULL;
  //new_node->prev = NULL;
  new_node->neighbor_weight = new_node;
  return new_node;
}*/

