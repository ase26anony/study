struct fixup_graph *create_fixup_graph(int num_vertices);
void add_fixup_edge(struct fixup_graph *g, int from, int to);
void compute_dominators(struct fixup_graph *g);
