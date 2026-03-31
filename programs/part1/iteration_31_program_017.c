// Consider adding validation
if (src == NULL || dest == NULL) {
    return NULL; // Or handle error
}

// Consider using designated initializers for clarity
Edge* e = malloc(sizeof(Edge));
if (!e) return NULL;

*e = (Edge){
    .src = src,
    .dest = dest,
    .type = t,
    .data_type = dt,
    .latency = l,
    .distance = d,
    .next_in = NULL,
    .next_out = NULL,
    .in_scc = false
};
