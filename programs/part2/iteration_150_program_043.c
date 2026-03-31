struct fixup_graph graph;
graph.num_edges = calculate_num_edges();
graph.edge_list = malloc(graph.num_edges * sizeof(struct edge));
// ... use graph ...
free(graph.edge_list);
