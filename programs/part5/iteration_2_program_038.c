e->src = src;           // Source node of the edge
e->dest = dest;         // Destination node of the edge  
e->type = t;            // Type of edge/connection
e->data_type = dt;      // Data type being transmitted
e->latency = l;         // Time delay for transmission
e->distance = d;        // Physical/logical distance
e->next_in = e->next_out = NULL;  // Initialize linked list pointers
e->in_scc = false;      // Mark as not in a strongly connected component
return e;               // Return the initialized edge
