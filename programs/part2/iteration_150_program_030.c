struct fixup_graph fg;
fg.num_edges = calculate_num_edges();
fg.edge_list = malloc(fg.num_edges * sizeof(struct edge));
// ... use it ...
free(fg.edge_list);
