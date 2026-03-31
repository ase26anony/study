#pragma omp target teams distribute
for (int i = 0; i < M; i++) {
    #pragma omp simd safelen(16)
    for (int j = 0; j < N; j++) {
        /* loop body */
    }
}
