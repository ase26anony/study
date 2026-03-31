Edge* create_edge(Node* src, Node* dest, EdgeType t, DataType dt, 
                  int latency, int distance) {
  if (!src || !dest) {
    fprintf(stderr, "Error: Source or destination node is NULL\n");
    return NULL;
  }
  
  Edge* e = malloc(sizeof(Edge));
  if (!e) {
    fprintf(stderr, "Error: Memory allocation failed for edge\n");
    return NULL;
  }
  
  e->src = src;
  e->dest = dest;
  e->type = t;
  e->data_type = dt;
  e->latency = latency;
  e->distance = distance;
  e->next_in = NULL;
  e->next_out = NULL;
  e->in_scc = false;
  
  // Optional: Add edge to source and destination node lists
  // e->next_out = src->out_edges;
  // src->out_edges = e;
  // etc.
  
  return e;
}
