// Creating a fixup graph
struct fixup_graph create_fixup_graph(int num_original_vertices) {
    struct fixup_graph fg;
    fg.new_entry_index = num_original_vertices;     // One past last original vertex
    fg.new_exit_index = num_original_vertices + 1;  // Two past last original vertex
    fg.num_edges = 0;
    fg.edge_list = malloc(MAX_EDGES * sizeof(struct edge));
    return fg;
}

// Adding an edge
void add_edge(struct fixup_graph *fg, int src, int dest, int cost) {
    fg->edge_list[fg->num_edges].src = src;
    fg->edge_list[fg->num_edges].dest = dest;
    fg->edge_list[fg->num_edges].cost = cost;
    fg->num_edges++;
}
