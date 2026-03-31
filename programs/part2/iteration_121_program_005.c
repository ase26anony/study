#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
float compute_scale(float x) {
    return x * 0.5f + 1.0f;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
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
    float *a = (float*)malloc(actual_N * sizeof(float));
    float *b = (float*)malloc(actual_N * sizeof(float));
    float *c = (float*)malloc(actual_N * sizeof(float));
    float *d = (float*)malloc(actual_N * sizeof(float));
    
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
    // This should trigger SIMT transformation
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) \
                            map(from: c[0:actual_N])
    {
        // Use collapse(2) with 2D loop structure
        int M = (actual_N > 100) ? 100 : actual_N;
        int K = actual_N / M;
        
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < K; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < actual_N) {
                    float s = compute_scale((float)idx);
                    c[idx] = a[idx] + b[idx] * scale * s;
                }
            }
        }
        
        // Second similar offloaded loop with different computation
        // Increases chance of hitting the transformation block again
        #pragma omp target teams distribute parallel for simd \
                num_teams(2) thread_limit(64)
        for (int i = 0; i < actual_N; i++) {
            // Conditional update to prevent simple vectorization
            if (c[i] > 100.0f) {
                c[i] = c[i] * 0.9f;
            } else {
                c[i] = c[i] * 1.1f;
            }
        }
    }
    
    // Third target region with different loop structure
    // Uses distribute parallel for (without simd clause explicitly)
    #pragma omp target data map(to: c[0:actual_N]) map(from: d[0:actual_N])
    {
        #pragma omp target teams distribute parallel for \
                simd num_teams(8) thread_limit(256)
        for (int i = 0; i < actual_N; i++) {
            // Complex computation to prevent optimization
            float val = c[i];
            for (int k = 0; k < 3; k++) {
                val = val * 0.8f + (float)k;
            }
            d[i] = val;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < actual_N; i++) {
        checksum += (double)d[i];
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
