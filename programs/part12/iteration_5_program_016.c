// Used in algorithms like Edmonds-Karp or Dinic's algorithm
// where you need to add super source/sink nodes
struct fixup_graph {
    int new_exit_index;  // Super sink index
    int new_entry_index; // Super source index
    int num_vertices;    // Original vertices + 2 (for super nodes)
    vertex_t *vertices;
    edge_t *edges;
};
