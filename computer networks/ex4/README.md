# Exercise 4 — Routing Algorithms (Bellman-Ford)

## Overview

This exercise implements the **Bellman-Ford shortest-path algorithm** for routing in computer networks, along with screenshots from virtual lab (VLab) simulations covering various network routing scenarios.

## Bellman-Ford Algorithm

### How It Works

The Bellman-Ford algorithm computes shortest paths from a single source vertex to all other vertices in a weighted directed graph. Unlike Dijkstra's algorithm, it handles **negative edge weights** and can **detect negative-weight cycles**.

1. **Initialize** — Set source distance to 0, all others to ∞
2. **Relax edges** — Repeat V-1 times: for each edge (u, v, w), if `dist[u] + w < dist[v]`, update `dist[v]` and record the path via `parent[v] = u`. Includes an early-exit optimization when no distances change in a pass.
3. **Negative cycle detection** — One additional pass: if any edge can still be relaxed, a negative-weight cycle exists and shortest paths are undefined.
4. **Path reconstruction** — Trace `parent[]` pointers from each vertex back to the source.

### Features

- Handles negative edge weights
- Detects negative-weight cycles
- Reconstructs and displays the full shortest path (not just the distance)
- Early termination optimization when no relaxation occurs in a pass

## Files

| File | Description |
|------|-------------|
| `bellman-ford.c` | Bellman-Ford shortest-path implementation with negative cycle detection |
| `VLAB/` | Virtual lab simulation screenshots |
| `VLAB/TOPICS.png` | Overview of VLab topics covered |
| `VLAB/V_Lab 1/` | VLab Experiment 1 screenshots |
| `VLAB/V_Lab 2/` | VLab Experiment 2 screenshots |
| `VLAB/V_Lab 3/` | VLab Experiment 3 screenshots |
| `VLAB/V_Lab 4/` | VLab Experiment 4 screenshots |
| `VLAB/V_Lab 5/` | VLab Experiment 5 screenshots |

## How to Run

```bash
gcc -o bellman-ford bellman-ford.c
./bellman-ford
```

You will be prompted to enter:
1. Number of vertices
2. Number of edges
3. Each edge as: `source destination weight`
4. Source vertex

## Sample Input/Output

```
=== Bellman-Ford Shortest Path Algorithm ===

Enter number of vertices: 5
Enter number of edges: 7
Enter each edge as: source destination weight
Edge 1: 0 1 6
Edge 2: 0 2 7
Edge 3: 1 2 8
Edge 4: 1 3 5
Edge 5: 1 4 -4
Edge 6: 2 3 -3
Edge 7: 3 4 7
Enter the source vertex: 0

Vertex    Distance from Source        Path
-------------------------------------------------------------
0          0                          0
1          6                          0 -> 1
2          7                          0 -> 2
3          4                          0 -> 2 -> 3
4          2                          0 -> 1 -> 4
```

## Key Concepts

- **Bellman-Ford algorithm** — O(V × E) time complexity
- **Edge relaxation** — Iteratively improving shortest-path estimates
- **Negative-weight edges** — Handled correctly (unlike Dijkstra's)
- **Negative-weight cycle detection** — V-th pass check
- **Path reconstruction** — Using parent/prev pointers
- **Routing in networks** — Distance-vector routing foundation
