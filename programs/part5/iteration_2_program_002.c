Edge* create_edge(Node* src, Node* dest, EdgeType t, DataType dt, 
                  int latency, int distance) {
  if (!src || !dest) return NULL;
  
  Edge* e = malloc(sizeof(Edge));
  if (!e) return NULL;
  
  e->src = src;
  e->dest = dest;
  e->type = t;
  e->data_type = dt;
  e->latency = latency;
  e->distance = distance;
  e->next_in = e->next_out = NULL;
  e->in_scc = false;
  
  return e;
}
