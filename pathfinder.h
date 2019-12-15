// The first step on the shortest path from the local node to the destination node
typedef struct {
    int destination_id; // The id of the destination node
    int neighbor_id; // The first node on the shortest path to destination node
} path;

// Linked List of paths
typedef struct _pathList {
    path *path;
    struct _pathList *next; // Next path in the list
} pathList;