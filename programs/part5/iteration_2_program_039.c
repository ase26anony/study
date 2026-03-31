e->src = src;           // Set source node of the edge
e->dest = dest;         // Set destination node of the edge  
e->type = t;            // Set edge type (e.g., directed/undirected, weight type)
e->data_type = dt;      // Set data type associated with the edge
e->latency = l;         // Set latency/cost of traversing the edge
e->distance = d;        // Set physical/logical distance
e->next_in = e->next_out = NULL;  // Initialize adjacency list pointers
e->in_scc = false;      // Mark edge as not in a strongly connected component (yet)
return e;               // Return the initialized edge
