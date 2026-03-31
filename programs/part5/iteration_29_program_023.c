#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
float device_compute(float a, float b) {
    return a * b + (a - b);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float result = 0.0f;
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i % 10) * 0.1f;
        b[i] = (i % 7) * 0.2f;
    }
    
    // Use argc to create runtime condition for offloading
    int use_offload = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (use_offload) {
        // This should trigger the SIMT transformation for GPU offloading
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) reduction(+:result)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            float temp = device_compute(a[i], b[i]);
            result += temp * (i % 5 + 1); // Data-dependent computation
        }
    } else {
        // Host fallback version
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            float temp = a[i] * b[i] + (a[i] - b[i]);
            result += temp * (i % 5 + 1);
        }
    }
    
    // Additional nested OpenMP construct to increase complexity
    float verify = 0.0f;
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:verify)
        for (int i = 0; i < N; i++) {
            verify += a[i] + b[i];
        }
    }
    
    printf("Result: %f\n", result);
    printf("Checksum: %f\n", verify);
    
    free(a);
    free(b);
    
    return 0;
}
