struct fixup_graph create_fixup_graph(int num_vertices, int num_edges) {
    struct fixup_graph fg;
    fg.new_exit_index = num_vertices;     // Artificial exit vertex
    fg.new_entry_index = num_vertices + 1; // Artificial entry vertex
    fg.edge_list = malloc(num_edges * sizeof(struct edge));
    fg.num_edges = num_edges;
    return fg;
}
