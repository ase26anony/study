// Creating a fixup graph for a circulation problem
struct fixup_graph create_fixup_graph(int original_vertices) {
    struct fixup_graph fg;
    
    // Add artificial source and sink
    fg.new_entry_index = original_vertices;     // Artificial source
    fg.new_exit_index = original_vertices + 1;  // Artificial sink
    
    // Allocate edges (original edges + artificial edges)
    fg.num_edges = calculate_total_edges(original_vertices);
    fg.edge_list = malloc(fg.num_edges * sizeof(struct edge));
    
    return fg;
}
