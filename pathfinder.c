#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "message_objects.h"
#include "pathfinder.h"

typedef struct {
    int vertex;
    int weight;
} edge_t;
 
typedef struct {
    edge_t **edges;
    int edges_len;
    int edges_size;
    int dist;
    int prev;
    int visited;
} vertex_t;
 
typedef struct {
    vertex_t **vertices;
    int vertices_len;
    int vertices_size;
} graph_t;
 
typedef struct {
    int *data;
    int *prio;
    int *index;
    int len;
    int size;
} heap_t;
 
void add_vertex (graph_t *g, int i) {
    if (g->vertices_size < i + 1) {
        int size = g->vertices_size * 2 > i ? g->vertices_size * 2 : i + 4;
        g->vertices = realloc(g->vertices, size * sizeof (vertex_t *));
        for (int j = g->vertices_size; j < size; j++)
            g->vertices[j] = NULL;
        g->vertices_size = size;
    }
    if (!g->vertices[i]) {
        g->vertices[i] = calloc(1, sizeof (vertex_t));
        g->vertices_len++;
    }
}
 
void add_edge (graph_t *g, int a, int b, int w) {
    a = a - 'a';
    b = b - 'a';
    add_vertex(g, a);
    add_vertex(g, b);
    vertex_t *v = g->vertices[a];
    if (v->edges_len >= v->edges_size) {
        v->edges_size = v->edges_size ? v->edges_size * 2 : 4;
        v->edges = realloc(v->edges, v->edges_size * sizeof (edge_t *));
    }
    edge_t *e = calloc(1, sizeof (edge_t));
    e->vertex = b;
    e->weight = w;
    v->edges[v->edges_len++] = e;
}
 
heap_t *create_heap (int n) {
    heap_t *h = calloc(1, sizeof (heap_t));
    h->data = calloc(n + 1, sizeof (int));
    h->prio = calloc(n + 1, sizeof (int));
    h->index = calloc(n, sizeof (int));
    return h;
}
 
void push_heap (heap_t *h, int v, int p) {
    int i = h->index[v] == 0 ? ++h->len : h->index[v];
    int j = i / 2;
    while (i > 1) {
        if (h->prio[j] < p)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
        j = j / 2;
    }
    h->data[i] = v;
    h->prio[i] = p;
    h->index[v] = i;
}
 
int min (heap_t *h, int i, int j, int k) {
    int m = i;
    if (j <= h->len && h->prio[j] < h->prio[m])
        m = j;
    if (k <= h->len && h->prio[k] < h->prio[m])
        m = k;
    return m;
}
 
int pop_heap (heap_t *h) {
    int v = h->data[1];
    int i = 1;
    while (1) {
        int j = min(h, h->len, 2 * i, 2 * i + 1);
        if (j == h->len)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
    }
    h->data[i] = h->data[h->len];
    h->prio[i] = h->prio[h->len];
    h->index[h->data[i]] = i;
    h->len--;
    return v;
}
 
void dijkstra (graph_t *g, int a, int b) {
    int i, j;
    a = a - 'a';
    b = b - 'a';
    for (i = 0; i < g->vertices_len; i++) {
        vertex_t *v = g->vertices[i];
        v->dist = INT_MAX;
        v->prev = 0;
        v->visited = 0;
    }
    vertex_t *v = g->vertices[a];
    v->dist = 0;
    heap_t *h = create_heap(g->vertices_len);
    push_heap(h, a, v->dist);
    while (h->len) {
        i = pop_heap(h);
        if (i == b)
            break;
        v = g->vertices[i];
        v->visited = 1;
        for (j = 0; j < v->edges_len; j++) {
            edge_t *e = v->edges[j];
            vertex_t *u = g->vertices[e->vertex];
            if (!u->visited && v->dist + e->weight <= u->dist) {
                u->prev = i;
                u->dist = v->dist + e->weight;
                push_heap(h, e->vertex, u->dist);
            }
        }
    }
}
 
void print_path (graph_t *g, int i) {
    int n, j;
    vertex_t *v, *u;
    i = i - 'a';
    v = g->vertices[i];
    if (v->dist == INT_MAX) {
        printf("no path\n");
        return;
    }
    for (n = 1, u = v; u->dist; u = g->vertices[u->prev], n++)
        ;
    char *path = malloc(n);
    path[n - 1] = 'a' + i;
    for (j = 0, u = v; u->dist; u = g->vertices[u->prev], j++)
        path[n - j - 2] = 'a' + u->prev;
    printf("%d %.*s\n", v->dist, n, path);
}

// Uses Dijikstra's shortest path routing algorithm to find the shortest path to each other node
pathList* findPaths(link_state_node* localNode) {
    /*
    // Free existing paths
    pathList *current = *paths;
    while (current != NULL) {
        free(current->path);
        pathList *prev = current;
        current = current->next;
        free (prev);
    }*/

    /*
    // Initial Graph generation
    graph_t *g = calloc(1, sizeof (graph_t));
    
    add_edge(g, 'a', 'b', 7);
    add_edge(g, 'a', 'c', 9);
    add_edge(g, 'a', 'f', 14);
    add_edge(g, 'b', 'c', 10);
    add_edge(g, 'b', 'd', 15);
    add_edge(g, 'c', 'd', 11);
    add_edge(g, 'c', 'f', 2);
    add_edge(g, 'd', 'e', 6);
    add_edge(g, 'e', 'f', 9);
    
    dijkstra(g, 'a', 'a');
    print_path(g, 'a');
    dijkstra(g, 'a', 'b');
    print_path(g, 'b');
    dijkstra(g, 'a', 'c');
    print_path(g, 'c');
    dijkstra(g, 'a', 'd');
    print_path(g, 'd');
    dijkstra(g, 'a', 'e');
    print_path(g, 'e');
    dijkstra(g, 'a', 'f');
    print_path(g, 'f');

    */

    pathList *head = NULL;
    head = malloc(sizeof(pathList));
    if (head == NULL) {
        return NULL;
    }

    // Path 1
    path *path1 = NULL;
    path1 = malloc(sizeof(path));
    if (path1 == NULL) {
        return NULL;
    }
    path1->destination_id = 2;
    path1->neighbor_id = 2;

    // Path 2
    path *path2 = NULL;
    path2 = malloc(sizeof(path));
    if (path2 == NULL) {
        return NULL;
    }
    path2->destination_id = 3;
    path2->neighbor_id = 3;

    // Path 3
    path *path3 = NULL;
    path3 = malloc(sizeof(path));
    if (path3 == NULL) {
        return NULL;
    }
    path3->destination_id = 4;
    path3->neighbor_id = 3;

    // Path 4
    path *path4 = NULL;
    path4 = malloc(sizeof(path));
    if (path4 == NULL) {
        return NULL;
    }
    path4->destination_id = 5;
    path4->neighbor_id = 3;

    // Path 5
    path *path5 = NULL;
    path5 = malloc(sizeof(path));
    if (path5 == NULL) {
        return NULL;
    }
    path5->destination_id = 6;
    path5->neighbor_id = 3;
    
    pathList *current = head;
    current->path = path1;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path2;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path3;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path4;
    current->next = malloc(sizeof(pathList));
    current = current->next;
    current->path = path5;

    return head;
}
 
int main () {
    link_state_node *localNode = calloc(1, sizeof (link_state_node));
    pathList *paths = findPaths(localNode);
    if (paths == NULL) {
        return -1;
    }
    
    printf("%d\n", paths->path->destination_id);
    printf("%d\n", paths->next->path->destination_id);

    return 0;
}