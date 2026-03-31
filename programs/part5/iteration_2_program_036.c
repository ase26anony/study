// Initialize edge fields
e->src = src;           // Source node/vertex
e->dest = dest;         // Destination node/vertex  
e->type = t;            // Edge type (e.g., directed/undirected, weight type)
e->data_type = dt;      // Data type associated with the edge
e->latency = l;         // Latency or weight for timing/pathfinding
e->distance = d;        // Distance metric (could be physical or logical)
e->next_in = e->next_out = NULL;  // Linked list pointers for adjacency lists
e->in_scc = false;      // Flag for Strongly Connected Component detection
return e;               // Return the initialized edge
