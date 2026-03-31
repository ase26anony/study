#pragma omp simd safelen(16)
for (int i = 0; i < M; i++) {
    if (some_condition) {
        A[i] = A[i + k];  // Potential dependency!
    }
}
