e->src = src;           // Sets the source node of the edge
e->dest = dest;         // Sets the destination node of the edge  
e->type = t;            // Sets the type of the edge (e.g., directed/undirected)
e->data_type = dt;      // Sets the data type associated with the edge
e->latency = l;         // Sets the latency/cost of traversing the edge
e->distance = d;        // Sets the distance metric for the edge
e->next_in = e->next_out = NULL;  // Initializes linked list pointers to NULL
e->in_scc = false;      // Marks the edge as not in a strongly connected component
return e;               // Returns the initialized edge
