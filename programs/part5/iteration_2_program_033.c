e->src = src;           // Source node of the edge
e->dest = dest;         // Destination node of the edge  
e->type = t;            // Type of edge/connection
e->data_type = dt;      // Data type transmitted through this edge
e->latency = l;         // Time delay for data to traverse this edge
e->distance = d;        // Physical or logical distance between nodes
e->next_in = e->next_out = NULL;  // Linked list pointers for adjacency lists
e->in_scc = false;      // Whether this edge is part of a strongly connected component
return e;               // Return the initialized edge
