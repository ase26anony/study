Edge* create_edge(Node* src, Node* dest, EdgeType t, DataType dt, 
                  int latency, int distance) {
    // Validate parameters
    if (!src || !dest || latency < 0 || distance < 0) {
        return NULL;
    }
    
    Edge* e = calloc(1, sizeof(Edge));  // Zero-initialize
    if (!e) return NULL;
    
    e->src = src;
    e->dest = dest;
    e->type = t;
    e->data_type = dt;
    e->latency = latency;
    e->distance = distance;
    // next_in, next_out, in_scc already NULL/false from calloc
    
    return e;
}
