e->src = src;          // Source node/vertex of the edge
e->dest = dest;        // Destination node/vertex of the edge
e->type = t;           // Type of edge (e.g., directed/undirected, different edge categories)
e->data_type = dt;     // Data type associated with the edge (if applicable)
e->latency = l;        // Latency or weight/cost of traversing this edge
e->distance = d;       // Physical or logical distance between nodes
e->next_in = e->next_out = NULL;  // Pointers for adjacency list (incoming/outgoing edges)
e->in_scc = false;     // Flag indicating if edge is in a Strongly Connected Component
return e;              // Return the initialized edge
