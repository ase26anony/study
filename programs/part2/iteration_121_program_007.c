#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_compute(float* a, float* b, float* c, int n, float scale) {
    // Empty - just to test declare target
}
#pragma omp end declare target

int main(int argc, char** argv) {
    // Use volatile and command-line args to prevent constant propagation
    volatile int base_n = 1000;
    int n = base_n;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_n;
    }
    
    // Dynamic scale factor
    float scale = (argc > 2) ? atof(argv[2]) : 2.5f;
    
    // Allocate arrays
    float* a = (float*)malloc(n * n * sizeof(float));
    float* b = (float*)malloc(n * n * sizeof(float));
    float* c = (float*)malloc(n * n * sizeof(float));
    float* d = (float*)malloc(n * n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < n * n; i++) {
        a[i] = (float)(i % 100);
        b[i] = (float)((i + 1) % 100);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // First target region with teams distribute parallel for simd
    // Using collapse(2) for 2D loop to increase complexity
    #pragma omp target data map(to: a[0:n*n], b[0:n*n]) map(from: c[0:n*n])
    {
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int idx = i * n + j;
                // Data-dependent computation with non-constant scale
                c[idx] = a[idx] + b[idx] * scale;
            }
        }
        
        // Second similar offloaded loop with different computation
        // This may trigger the transformation again
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(2) thread_limit(64)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int idx = i * n + j;
                // Different computation pattern
                c[idx] = c[idx] + (a[idx] - b[idx]) / (scale + 1.0f);
            }
        }
    }
    
    // Third target region with different array
    #pragma omp target data map(to: a[0:n*n]) map(from: d[0:n*n])
    {
        // Using simdlen clause to hint at SIMD/SIMT transformation
        #pragma omp target teams distribute parallel for simd \
                simdlen(8) num_teams(3)
        for (int i = 0; i < n * n; i++) {
            // Conditional computation to prevent optimization
            if (a[i] > 50.0f) {
                d[i] = a[i] * scale;
            } else {
                d[i] = a[i] / scale;
            }
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < n * n; i++) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Scale used: %f\n", scale);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
