#pragma omp target teams distribute
for (int j = 0; j < N; j++) {
    #pragma omp simd safelen(16)
    for (int i = 0; i < M; i++) {
        if (a[i] > threshold) {
            b[i] = b[i-1] + 1;  // Loop-carried dependency!
        }
    }
}
