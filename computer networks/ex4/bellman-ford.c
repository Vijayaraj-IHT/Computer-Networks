#include <stdio.h>
#include <stdbool.h>

#define MAX_VERTICES 1000
#define MAX_EDGES 10000
#define INF 1000000000 // 10^9 to handle larger edge weights without overflow

typedef struct {
    int src;
    int dest;
    int weight;
} Edge;

Edge edgeList[MAX_EDGES];
int dist[MAX_VERTICES];
int parent[MAX_VERTICES];

int numVertices = 0;
int numEdges = 0;
int sourceVertex = 0;

void inputGraph(void) {
    printf("Enter number of vertices: ");
    scanf("%d", &numVertices);

    printf("Enter number of edges: ");
    scanf("%d", &numEdges);
    int i;
    printf("Enter each edge as: source destination weight\n");
    for (i = 0; i < numEdges; i++) {
        printf("Edge %d: ", i + 1);
        scanf("%d %d %d", &edgeList[i].src, &edgeList[i].dest, &edgeList[i].weight);
    }

    printf("Enter the source vertex: ");
    scanf("%d", &sourceVertex);
}

void initDistArray(void) {
    int i;
    for (i = 0; i < numVertices; i++) {
        dist[i] = (i == sourceVertex) ? 0 : INF;
        parent[i] = -1;
    }
}

/* Relax every edge up to (V - 1) times with early exit optimization */
void relaxEdges(void) {
    int i,j;
    for (i = 1; i <= numVertices - 1; i++) {
        bool updated = false;

        for (j = 0; j < numEdges; j++) {
            int u = edgeList[j].src;
            int v = edgeList[j].dest;
            int w = edgeList[j].weight;

            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                updated = true;
            }
        }

        /* Stop early if no distances changed in this pass */
        if (!updated) {
            break;
        }
    }
}

/* V-th pass: if any edge can still be relaxed, a negative-weight cycle exists */
int detectNegativeCycle(void) {
    int j;
    for (j = 0; j < numEdges; j++) {
        int u = edgeList[j].src;
        int v = edgeList[j].dest;
        int w = edgeList[j].weight;

        if (dist[u] != INF && dist[u] + w < dist[v]) {
            return 1; // Negative cycle detected
        }
    }
    return 0;
}

void displayShortestPaths(void) {
    int path[MAX_VERTICES];
    int i,k;
    printf("\n%-10s %-25s %-30s\n", "Vertex", "Distance from Source", "Path");
    printf("-------------------------------------------------------------\n");

    for (i = 0; i < numVertices; i++) {
        printf("%-10d ", i);

        if (dist[i] == INF) {
            printf("%-25s %-30s\n", "INF", "Unreachable");
            continue;
        }

        printf("%-25d ", dist[i]);

        int count = 0;
        int curr = i;
        while (curr != -1) {
            path[count++] = curr;
            curr = parent[curr];
        }

        for (k = count - 1; k >= 0; k--) {
            printf("%d", path[k]);
            if (k > 0) printf(" -> ");
        }
        printf("\n");
    }
}

int main(void) {
    printf("=== Bellman-Ford Shortest Path Algorithm ===\n\n");

    inputGraph();
    initDistArray();
    relaxEdges();

    if (detectNegativeCycle()) {
        printf("\nGraph contains a negative weight cycle reachable from source %d. Shortest paths are undefined.\n", sourceVertex);
        return 0;
    }

    displayShortestPaths();

    return 0;
}
