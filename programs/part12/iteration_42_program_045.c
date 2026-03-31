e->src = src;           // Set source node/vertex
e->dest = dest;         // Set destination node/vertex  
e->type = t;            // Set edge type (e.g., directed/undirected)
e->data_type = dt;      // Set data type associated with the edge
e->latency = l;         // Set latency/cost/weight
e->distance = d;        // Set distance metric
e->next_in = e->next_out = NULL;  // Initialize linked list pointers
e->in_scc = false;      // Mark as not in a strongly connected component
return e;               // Return the initialized edge
