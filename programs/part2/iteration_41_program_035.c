for (int j = 0; j < N; j++) {           // Outer loop - distributed across teams
    for (int i = 0; i < M; i++) {       // Inner loop - vectorized with SIMD
        /* loop body with conditional */
    }
}
