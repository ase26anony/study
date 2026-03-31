struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
    fixup_edge *edge_list;
    int *first_edge; // adjacency list start index
    // ... 
};
