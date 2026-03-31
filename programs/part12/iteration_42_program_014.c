// Initialize edge fields
e->src = src;           // Source node of the edge
e->dest = dest;         // Destination node of the edge  
e->type = t;            // Type of edge/connection
e->data_type = dt;      // Data type being transferred
e->latency = l;         // Time delay for traversal
e->distance = d;        // Physical/logical distance
e->next_in = e->next_out = NULL;  // Linked list pointers for adjacency lists
e->in_scc = false;      // Flag for Strongly Connected Component membership
return e;               // Return the initialized edge
