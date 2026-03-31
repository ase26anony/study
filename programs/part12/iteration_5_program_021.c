struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
    fixup_vertex_t *vertices;  // Array of vertices
    fixup_edge_t *edges;       // Array of edges
    int *capacity;             // Edge capacities
    int *flow;                 // Current flow values
    /* ... */
};
