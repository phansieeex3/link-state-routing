#include "pqueue.h"

typedef struct _vector {
  int next_hop;
  int cost;
} vector;
//linked-list data structure for storing neighbor list
typedef struct _neighbor_node {
  struct _neighbor_node* next;
  struct _neighbor_node* previous;
  vector* neighbor_cost;
} neighbor_node;
typedef struct _LSP {
  int node_id;
  int sequence_num;
  int neighbor_num;
  neighbor_node* first_neighbor;
} LSP;
typedef struct _link_state_vector {
  int dest_ID;
  int next_hop;
  int previous_node;
  int cost;
  size_t pos;
} link_state_vector;

vector* setVector(int next_hop, int cost);
neighbor_node* setneighbor(int next_hop, int cost);

