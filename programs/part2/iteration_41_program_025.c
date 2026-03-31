// Pseudocode for understanding
for (team = 0; team < num_teams; team++) {
    // Each team processes a chunk of j iterations
    for (j_chunk = team_start; j_chunk < team_end; j_chunk++) {
        // Each thread uses SIMD for i-loop
        for (i = 0; i < M; i += vector_length) {
            // Process up to 16 elements simultaneously
            // Conditional must not have dependencies beyond 16 iterations
        }
    }
}
