// Create and initialize a new edge
Edge* create_edge(Node* src, Node* dest, EdgeType t, DataType dt, 
                  int l, int d) {
  Edge* e = malloc(sizeof(Edge));
  
  // Set basic edge properties
  e->src = src;          // Source node
  e->dest = dest;        // Destination node
  e->type = t;           // Type of edge (e.g., directed/undirected, control/data flow)
  e->data_type = dt;     // Type of data flowing through the edge
  e->latency = l;        // Time delay or latency
  e->distance = d;       // Physical or logical distance
  
  // Initialize linked list pointers for adjacency lists
  e->next_in = e->next_out = NULL;  // For storing in adjacency lists
  
  // SCC (Strongly Connected Component) tracking
  e->in_scc = false;     // Whether this edge is part of an SCC
  
  return e;
}
