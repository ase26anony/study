Edge* e = malloc(sizeof(Edge));
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
