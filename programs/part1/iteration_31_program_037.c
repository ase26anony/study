e->src = src;           // Source node/vertex
e->dest = dest;         // Destination node/vertex  
e->type = t;            // Edge type (e.g., directed/undirected, weight type)
e->data_type = dt;      // Data type associated with the edge
e->latency = l;         // Time delay or cost to traverse
e->distance = d;        // Physical or logical distance
e->next_in = e->next_out = NULL;  // Linked list pointers for adjacency lists
e->in_scc = false;      // Whether edge is in a strongly connected component
return e;               // Return the initialized edge
