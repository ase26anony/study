// Common problems with this pattern:
// 1. Data transfer overhead if arrays aren't already on device
// 2. Nested loops might not be optimal for GPU if M is small
// 3. Conditional in SIMD loop can cause divergence

// Better approach for GPU often:
#pragma omp target teams distribute parallel for simd collapse(2)
for (int j = 0; j < N; j++) {
    for (int i = 0; i < M; i++) {
        /* loop body */
    }
}
