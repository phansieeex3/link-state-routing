import Queue  
from collections import namedtuple

Edge = namedtuple('Edge', ['vertex', 'weight'])

# Graph used to compute Dijkstra's
class GraphUndirectedWeighted(object):  
    def __init__(self, vertex_count):
        self.vertex_count = vertex_count
        self.adjacency_list = [[] for _ in range(vertex_count)]

    def add_edge(self, source, dest, weight):
        # Error handling
        assert source < self.vertex_count
        assert dest < self.vertex_count
        self.adjacency_list[source].append(Edge(dest, weight))
        self.adjacency_list[dest].append(Edge(source, weight))

    def get_edge(self, vertex):
        for e in self.adjacency_list[vertex]:
            yield e

    def get_vertex(self):
        for v in range(self.vertex_count):
            yield v

# Calculates shortest path from source to destination and can return distance
def dijkstra(graph, source, dest):  
    q = Queue.PriorityQueue()
    parents = []
    distances = []
    start_weight = float("inf")

    for i in graph.get_vertex():
        weight = start_weight
        if source == i:
            weight = 0
        distances.append(weight)
        parents.append(None)

    q.put(([0, source]))

    # Find shortest path 
    while not q.empty():
        v_tuple = q.get()
        v = v_tuple[1]

        # Iterate through neighbors
        for e in graph.get_edge(v):
            candidate_distance = distances[v] + e.weight
            if distances[e.vertex] > candidate_distance:
                distances[e.vertex] = candidate_distance
                parents[e.vertex] = v
                
                # Used to alert for negative cyclops
                if candidate_distance < -1000:
                    raise Exception("Negative cycle detected!")
                q.put(([distances[e.vertex], e.vertex]))

    shortest_path = []
    end = dest

    # Build shortest path
    while end is not None:
        shortest_path.append(end)
        end = parents[end]

    shortest_path.reverse()

    return shortest_path #, distances[dest]


def main():  
    print("Running Dijkstra's in Python...")

    file_name = "./network_data.txt"
    f = open(file_name)

    nodes = []

    # Get count of edges and nodes
    edgeCount = 0
    for line in f:
        data = line.split(',')
        nodes.append(data[0])
        edgeCount += 1
    f.close()

    g = GraphUndirectedWeighted(edgeCount)

    # Add edges to graph
    f = open(file_name)
    for line in f:
        edgeCount += 1
        data = line.split(',')
        i = 1
        while (data[i] != '\n'):
            #print("Adding Edge: " + str(nodes.index(data[0]))+ ", " + str(nodes.index(data[i])) + ", " + str(data[i+1]))
            g.add_edge(nodes.index(data[0]), nodes.index(data[i]), int(data[i+1]))
            i += 2
    f.close()


    # Write data to file
    f = open(file_name, 'w')

    # Local Node
    f.write(str(nodes[0]) + ":" + str(nodes[0]) + "\n")

    for i in range(len(nodes)):
        shortest_path = dijkstra(g, 0, i)
        
        if (i != 0):
            f.write(str(nodes[i]) + ":" + str(nodes[shortest_path[1]]))
            if (i != len(nodes) - 1):
                f.write("\n")
        
    print("Dijkstra's Finished!")



if __name__ == "__main__":  
    main()
