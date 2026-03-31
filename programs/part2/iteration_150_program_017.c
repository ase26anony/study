struct fixup_graph g;
g.new_exit_index = 10;
g.new_entry_index = 11;
g.num_edges = 5;
g.edge_list = malloc(g.num_edges * sizeof(struct edge));

g.edge_list[0] = (struct edge){.src = 0, .dest = 1, .cost = 4};
// ... etc.
