// Set the source node of the edge
e->src = src;

// Set the destination node of the edge  
e->dest = dest;

// Set the type of the edge (e.g., directed/undirected, communication type, etc.)
e->type = t;

// Set the data type associated with the edge (what kind of data flows through it)
e->data_type = dt;

// Set the latency (time delay) for traversing this edge
e->latency = l;

// Set the distance (could be physical distance or logical distance)
e->distance = d;

// Initialize the linked list pointers for adjacency lists
// next_in: pointer to next edge coming INTO the destination node
// next_out: pointer to next edge going OUT of the source node
e->next_in = e->next_out = NULL;

// Mark that this edge is not part of a strongly connected component (SCC)
// Used for graph algorithms like Tarjan's algorithm
e->in_scc = false;

// Return the initialized edge structure
return e;
