// A link state node
typedef struct _node {
    int nodeId;
    int weight;
    nodeList* neighbors;
} node;

// Map of node id to a path
typedef struct _pathMap {
    int id;
    nodeList* path;
} pathMap;

// List of nodes
typedef struct _nodeList {
    node* node;
    struct nodeList * next;
} nodeList;

// Given a node (representing the local node), generate a list of paths from that node to everyother node
pathMap* buildPaths(node* localNode) {
    // Generate array of all nodes in network
    // Build 2d graph from array

    // for each node in network
        node* goalNode = ???;
        computeShortestPath(localNode, goalNode);
        // Save path to local node

    
    pathMap* paths = 0;
    return paths;
}

nodeList* computeShortestPath(node* startNode, node* goalNode) {

}