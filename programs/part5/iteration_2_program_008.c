e->src = src;           // Set source node/vertex
e->dest = dest;         // Set destination node/vertex  
e->type = t;            // Set edge type
e->data_type = dt;      // Set data type associated with the edge
e->latency = l;         // Set latency/cost value
e->distance = d;        // Set distance value
e->next_in = e->next_out = NULL;  // Initialize linked list pointers to NULL
e->in_scc = false;      // Mark as not in a strongly connected component
return e;               // Return the initialized edge
