struct fixup_graph *fixup_graph_create(int num_vertices);
void fixup_graph_add_edge(struct fixup_graph *g, int src, int dest, int cost);
void fixup_graph_compute_minimum_cost_flow(struct fixup_graph *g);
