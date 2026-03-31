// Set the source node of the edge
e->src = src;

// Set the destination node of the edge  
e->dest = dest;

// Set the type of the edge (could be directed/undirected, etc.)
e->type = t;

// Set the data type associated with the edge
e->data_type = dt;

// Set the latency (delay) of the edge
e->latency = l;

// Set the distance or weight of the edge
e->distance = d;

// Initialize linked list pointers to NULL (edge is not yet connected)
e->next_in = e->next_out = NULL;

// Mark the edge as not being in a strongly connected component
e->in_scc = false;

// Return the initialized edge structure
return e;
