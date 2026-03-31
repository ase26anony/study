e->src = src;           // Sets the source node of the edge
e->dest = dest;         // Sets the destination node of the edge  
e->type = t;            // Sets the edge type (e.g., directed/undirected, weight type)
e->data_type = dt;      // Sets the data type associated with the edge
e->latency = l;         // Sets the latency/cost of traversing the edge
e->distance = d;        // Sets the distance between nodes
e->next_in = e->next_out = NULL;  // Initializes linked list pointers for adjacency lists
e->in_scc = false;      // Marks the edge as not being in a strongly connected component
return e;               // Returns the initialized edge
