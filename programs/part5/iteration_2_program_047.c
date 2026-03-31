// Set source node of the edge
e->src = src;

// Set destination node of the edge  
e->dest = dest;

// Set the type of edge/connection
e->type = t;

// Set the data type being transferred
e->data_type = dt;

// Set the latency (time delay) of this edge
e->latency = l;

// Set the distance (physical or logical) between nodes
e->distance = d;

// Initialize linked list pointers for incoming/outgoing edges
e->next_in = e->next_out = NULL;

// Mark edge as not part of a strongly connected component (SCC) initially
e->in_scc = false;

// Return the initialized edge
return e;
