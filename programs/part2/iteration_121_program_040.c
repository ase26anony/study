#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_compute(float* a, float* b, float* c, int n, float scale) {
    // Empty - just to mark function for offloading
}
#pragma omp end declare target

int main(int argc, char** argv) {
    // Use command-line input to prevent constant folding
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 10000;
    }
    
    // Use volatile to prevent optimization
    volatile int dynamic_N = N;
    int actual_N = dynamic_N;
    
    // Allocate and initialize arrays
    float* a = (float*)malloc(actual_N * sizeof(float));
    float* b = (float*)malloc(actual_N * sizeof(float));
    float* c = (float*)malloc(actual_N * sizeof(float));
    float* d = (float*)malloc(actual_N * sizeof(float));
    
    for (int i = 0; i < actual_N; i++) {
        a[i] = (float)i;
        b[i] = (float)(actual_N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Non-constant scale factor
    float scale = 2.5f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // First target region with teams distribute parallel for simd
    // Using collapse(2) to create 2D loop structure
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) \
                            map(from: c[0:actual_N])
    {
        int outer = actual_N / 100;
        if (outer < 1) outer = 1;
        
        // This should trigger the SIMT transformation
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(outer) thread_limit(128)
        for (int i = 0; i < outer; i++) {
            for (int j = 0; j < 100; j++) {
                int idx = i * 100 + j;
                if (idx < actual_N) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Second similar region with different computation
        // to potentially trigger transformation again
        #pragma omp target teams distribute parallel for simd \
                num_teams(outer) thread_limit(128)
        for (int i = 0; i < actual_N; i++) {
            // Conditional computation to create data-dependent control flow
            if (c[i] > 100.0f) {
                c[i] = c[i] * 0.5f;
            } else {
                c[i] = c[i] * 2.0f;
            }
        }
    }
    
    // Another target region with different data mapping
    // to increase chance of hitting the transformation
    #pragma omp target data map(to: c[0:actual_N]) map(from: d[0:actual_N])
    {
        int chunk = actual_N / 50;
        if (chunk < 1) chunk = 1;
        
        // Third region with different loop structure
        #pragma omp target teams distribute parallel for simd \
                num_teams(50) thread_limit(64)
        for (int i = 0; i < actual_N; i++) {
            float val = c[i];
            // Use mathematical functions to create complex expressions
            d[i] = val * val + val / (scale + 1.0f);
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < actual_N; i++) {
        checksum += d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array size: %d\n", actual_N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
