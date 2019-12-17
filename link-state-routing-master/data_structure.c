#include "data_structure.h"
vector* setVector(int next_hop, int cost) {
  vector* new_cost = (vector*)malloc(sizeof(vector));
  (*new_cost).next_hop = next_hop;
  (*new_cost).cost = cost;
  return new_cost;
}
neighbor_node* setneighbor(int next_hop, int cost) {
  vector* new_vector = setVector(next_hop, cost);
  neighbor_node* new_neighbor = (neighbor_node*)malloc(sizeof(neighbor_node));
  new_neighbor->next = NULL;
  new_neighbor->previous = NULL;
  new_neighbor->neighbor_cost = new_vector;
  return new_neighbor;
}
