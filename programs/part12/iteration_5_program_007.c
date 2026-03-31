struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
    int *edge_list;           // Array of edges
    int *reverse_edge;        // Reverse edge indices
    int *capacity;            // Edge capacities
    int *flow;                // Current flow values
    int *first_edge;          // First edge index for each vertex
    int *next_edge;           // Next edge in adjacency list
    /* ... possibly more ... */
};
