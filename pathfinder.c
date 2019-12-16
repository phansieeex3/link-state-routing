#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "message_objects.h"
#include "pathfinder.h"
 
void calculateShortestPaths(link_state_node *localNode) {
    // Write network data to file
    FILE *fp;
    fp = fopen("./network_data.txt", "w");
        // Iterate through linked_state_node's
    link_state_node *current = localNode;
    while (current != NULL) {
        fprintf(fp, "%d,", current->destination_ID);
        neighbor_list *neighbors = current->neighbor_nodes;
        while (neighbors != NULL) {
            int id = neighbors->neighbor_node->id;
            int weight = neighbors->neighbor_node->weight;
            fprintf(fp, "%d,%d,", id, weight);
            neighbors = neighbors->next;
        }  
        fprintf(fp, "\n");
        current = current->next;
    }
    fclose(fp);

    // Run python code on file
    int py_response = system("python pathfinder.py");
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


int main () {
    link_state_node *localNode = calloc(1, sizeof (link_state_node));
    link_state_node *itr = localNode;

    // Node 1
    itr->destination_ID = 1;
    itr->neighbor_nodes = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->neighbor_node->id = 2;
    itr->neighbor_nodes->neighbor_node->weight = 3;
    itr->neighbor_nodes->prev = NULL;
    itr->neighbor_nodes->next = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->next->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->next->neighbor_node->id = 3;
    itr->neighbor_nodes->next->neighbor_node->weight = 1;
    itr->neighbor_nodes->next->prev = itr->neighbor_nodes;
    itr->neighbor_nodes->next->next = NULL;
    itr->next = malloc(sizeof(link_state_node));
    itr = itr->next;
    
    // Node 2
    itr->destination_ID = 2;
    itr->neighbor_nodes = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->neighbor_node->id = 1;
    itr->neighbor_nodes->neighbor_node->weight = 3;
    itr->neighbor_nodes->prev = NULL;
    itr->neighbor_nodes->next = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->next->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->next->neighbor_node->id = 3;
    itr->neighbor_nodes->next->neighbor_node->weight = 1;
    itr->neighbor_nodes->next->prev = itr->neighbor_nodes;
    itr->neighbor_nodes->next->next = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->next->next->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->next->next->neighbor_node->id = 4;
    itr->neighbor_nodes->next->next->neighbor_node->weight = 2;
    itr->neighbor_nodes->next->next->prev = itr->neighbor_nodes->next;
    itr->neighbor_nodes->next->next->next = NULL;
    itr->next = malloc(sizeof(link_state_node));
    itr = itr->next;

    // Node 3
    itr->destination_ID = 3;
    itr->neighbor_nodes = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->neighbor_node->id = 1;
    itr->neighbor_nodes->neighbor_node->weight = 1;
    itr->neighbor_nodes->prev = NULL;
    itr->neighbor_nodes->next = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->next->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->next->neighbor_node->id = 2;
    itr->neighbor_nodes->next->neighbor_node->weight = 1;
    itr->neighbor_nodes->next->prev = itr->neighbor_nodes;
    itr->neighbor_nodes->next->next = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->next->next->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->next->next->neighbor_node->id = 4;
    itr->neighbor_nodes->next->next->neighbor_node->weight = 7;
    itr->neighbor_nodes->next->next->prev = itr->neighbor_nodes->next;
    itr->neighbor_nodes->next->next->next = NULL;
    itr->next = malloc(sizeof(link_state_node));
    itr = itr->next;

    // Node 4
    itr->destination_ID = 4;
    itr->neighbor_nodes = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->neighbor_node->id = 2;
    itr->neighbor_nodes->neighbor_node->weight = 2;
    itr->neighbor_nodes->prev = NULL;
    itr->neighbor_nodes->next = malloc(sizeof(neighbor_list));
    itr->neighbor_nodes->next->neighbor_node = malloc(sizeof(neighbor_node));
    itr->neighbor_nodes->next->neighbor_node->id = 3;
    itr->neighbor_nodes->next->neighbor_node->weight = 7;
    itr->neighbor_nodes->next->prev = itr->neighbor_nodes;
    itr->neighbor_nodes->next->next = NULL;
    itr->next = NULL;

    calculateShortestPaths(localNode);

    return 0;
}