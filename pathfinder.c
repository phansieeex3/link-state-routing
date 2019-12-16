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



