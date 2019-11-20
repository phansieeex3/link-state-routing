
//link state node that contains information of chain
typedef struct _link_state_node {
  int destination_ID;
  int next_node;
  int prev_node;
  int weight;
  size_t pos;
} link_state_node;

//my current node or all nodes
typedef struct _node {
  int next_node;
  int weight;
} node;


//my neighbor node
typedef struct _neighbor_node {
    struct _neighbor_node* next;
  struct _neighbor_node* prev;
  node* neighbor_weight;
} neighbor_node;

//link state announcement
typedef struct _LSA {
  int node_ID;
    int neighbor_size;

  int sequence_number;
  neighbor_node* neighbor;
} LSA;

node* setNode(int next_node, int weight) {
  node* new_weight = (node*)malloc(sizeof(node));
  (*new_weight).next_hop = next_node;
  (*new_weight).weight = weight;
  return new_weight;
}
neighbor_node* setneighbor(int next_node, int weight) {
  node* new_node = setNode(next_node, weight);
  neighbor_node* new_node = (neighbor_node*)malloc(sizeof(neighbor_node));
  new_node->next = NULL;
  new_node->prev = NULL;
  new_node->neighbor_weight = new_node;
  return new_node;
}

