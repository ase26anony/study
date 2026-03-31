// Creating a fixup graph for max flow
struct fixup_graph create_residual_graph(struct graph *original) {
    struct fixup_graph fg;
    fg.new_entry_index = original->num_vertices;     // Add super-source
    fg.new_exit_index = original->num_vertices + 1;  // Add super-sink
    fg.num_edges = 2 * original->num_edges + 2;      // Forward + backward edges
    fg.edge_list = malloc(fg.num_edges * sizeof(struct edge));
    
    // Build residual edges...
    return fg;
}
