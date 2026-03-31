// Consider adding error checking
if (!e) return NULL;

// Or using designated initializers for clarity
Edge* create_edge(Node* src, Node* dest, EdgeType t, DataType dt, 
                  int latency, int distance) {
    return &(Edge){
        .src = src,
        .dest = dest,
        .type = t,
        .data_type = dt,
        .latency = latency,
        .distance = distance,
        .next_in = NULL,
        .next_out = NULL,
        .in_scc = false
    };
}
