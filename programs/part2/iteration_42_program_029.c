#pragma omp simd reduction(+:sum) aligned(a,b,c,d,out1,out2:64)
for (int i = 0; i < n; ++i) {
    // ... same operations ...
}
