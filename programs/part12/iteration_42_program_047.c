// Initialize edge fields
e->src = src;           // Source node/vertex
e->dest = dest;         // Destination node/vertex  
e->type = t;            // Edge type/category
e->data_type = dt;      // Data type associated with the edge
e->latency = l;         // Communication/transfer latency
e->distance = d;        // Distance metric (could be physical or logical)
e->next_in = e->next_out = NULL;  // Linked list pointers for adjacency lists
e->in_scc = false;      // Flag indicating if edge is in a Strongly Connected Component
return e;               // Return the initialized edge
