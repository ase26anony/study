struct fixup_graph *create_fixup_graph(int entry, int exit, int max_edges) {
    struct fixup_graph *fg = malloc(sizeof(struct fixup_graph));
    fg->new_entry_index = entry;
    fg->new_exit_index = exit;
    fg->num_edges = 0;
    fg->edge_list = malloc(max_edges * sizeof(struct edge));
    return fg;
}
