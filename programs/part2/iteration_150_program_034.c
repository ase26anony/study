// Creating a fixup graph
struct fixup_graph create_fixup_graph(int original_vertices) {
    struct fixup_graph fg;
    fg.new_exit_index = original_vertices;     // New sink index
    fg.new_entry_index = original_vertices + 1; // New source index
    fg.num_edges = 0;
    fg.edge_list = malloc(MAX_EDGES * sizeof(struct edge));
    return fg;
}

// Adding an edge to the fixup graph
void add_fixup_edge(struct fixup_graph *fg, int src, int dest, int cost) {
    fg->edge_list[fg->num_edges].src = src;
    fg->edge_list[fg->num_edges].dest = dest;
    fg->edge_list[fg->num_edges].cost = cost;
    fg->num_edges++;
}
