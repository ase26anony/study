// Creating a simple flow network
struct fixup_graph create_flow_network() {
    struct fixup_graph g;
    g.new_entry_index = 0;  // Source vertex
    g.new_exit_index = 3;   // Sink vertex
    g.num_edges = 4;
    
    // Allocate edges: (src, dest, capacity)
    g.edge_list = malloc(g.num_edges * sizeof(struct edge));
    g.edge_list[0] = (struct edge){0, 1, 10};  // s->a, capacity 10
    g.edge_list[1] = (struct edge){0, 2, 5};   // s->b, capacity 5
    g.edge_list[2] = (struct edge){1, 3, 8};   // a->t, capacity 8
    g.edge_list[3] = (struct edge){2, 3, 7};   // b->t, capacity 7
    
    return g;
}
