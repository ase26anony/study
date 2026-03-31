// Creating a simple fixup graph
struct fixup_graph create_sample_graph() {
    struct fixup_graph graph;
    graph.new_entry_index = 0;  // Source vertex
    graph.new_exit_index = 3;   // Sink vertex
    graph.num_edges = 4;
    
    struct edge edges[4] = {
        {0, 1, 10},  // Edge from source to node 1 with capacity/cost 10
        {0, 2, 5},   // Edge from source to node 2 with capacity/cost 5
        {1, 3, 8},   // Edge from node 1 to sink with capacity/cost 8
        {2, 3, 7}    // Edge from node 2 to sink with capacity/cost 7
    };
    
    graph.edge_list = edges;
    return graph;
}
