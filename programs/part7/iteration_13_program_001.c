Edge* create_edge(Node* src, Node* dest, EdgeType t, DataType dt, 
                  int l, int d) {
  if (!e) return NULL;  // Safety check
  
  e->src = src;
  e->dest = dest;
  e->type = t;
  e->data_type = dt;
  e->latency = l;
  e->distance = d;
  e->next_in = e->next_out = NULL;
  e->in_scc = false;
  
  // Optional: Initialize other fields to defaults
  // e->weight = 1.0;
  // e->capacity = 0;
  
  return e;
}
