

//link state node that contains information of chain
typedef struct _link_state_node {
  int destination_ID;
  int next_node;
  int previous_node;
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
  node* neighbor_cost;
} neighbor_node;

//link state announcement
typedef struct _LSA {
  int node_ID;
  int sequence_number;
  int neighbor_number;
  neighbor_node* neighbor;
} LSA;




node* setNode(int next_node, int weight);
neighbor_node* setneighbor(int next_node, int weight);
