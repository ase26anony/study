e->src = src;           // Source node/vertex of the edge
e->dest = dest;         // Destination node/vertex of the edge  
e->type = t;            // Type of edge/connection
e->data_type = dt;      // Data type associated with the edge
e->latency = l;         // Latency or weight of the edge
e->distance = d;        // Distance metric (could be physical or logical)
e->next_in = e->next_out = NULL;  // Linked list pointers for adjacency lists
e->in_scc = false;      // Flag indicating if edge is in a Strongly Connected Component
return e;               // Return the initialized edge
