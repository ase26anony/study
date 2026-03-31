// Step 1: Run Dijkstra from source to target in original graph.
// Step 2: Transform graph: reverse edges on shortest path, negate costs.
// Step 3: Create fixup_graph with new_entry = source, new_exit = target
//         (or new artificial nodes if doing node-splitting).
// Step 4: Run shortest path again in fixup_graph to find disjoint path.
