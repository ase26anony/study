#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void compute_kernel(float *a, float *b, float *c, float scale, int n) {
    // Empty - just to demonstrate declare target
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use command-line argument to prevent constant propagation
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 10000;
    }
    
    // Use volatile to prevent optimization
    volatile int volatile_N = N;
    int actual_N = volatile_N;
    
    // Allocate arrays
    float *a = (float*)malloc(actual_N * sizeof(float));
    float *b = (float*)malloc(actual_N * sizeof(float));
    float *c = (float*)malloc(actual_N * sizeof(float));
    float *d = (float*)malloc(actual_N * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < actual_N; i++) {
        a[i] = (float)i;
        b[i] = (float)(actual_N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Non-constant scale factor
    float scale = (argc > 2) ? atof(argv[2]) : 2.5f;
    
    // First target region with teams distribute parallel for simd
    // Using collapse(2) to create 2D loop structure
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) \
                            map(from: c[0:actual_N])
    {
        // This should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(16) thread_limit(128)
        for (int i = 0; i < actual_N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < actual_N) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Second similar offloaded loop with different computation
        // Using different loop bounds and array access pattern
        #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(64)
        for (int i = 0; i < actual_N; i++) {
            // Conditional computation to create more complex control flow
            if (i % 3 == 0) {
                c[i] = c[i] * 0.5f;
            } else {
                c[i] = c[i] + 1.0f;
            }
        }
    }
    
    // Third target region with different data mapping
    // Using simdlen clause to hint at SIMD width
    #pragma omp target data map(to: a[0:actual_N]) \
                            map(tofrom: d[0:actual_N])
    {
        #pragma omp target teams distribute parallel for simd \
                simdlen(8) num_teams(4)
        for (int i = 0; i < actual_N; i++) {
            // Reduction-like pattern
            float temp = 0.0f;
            for (int k = 0; k < 4; k++) {
                if (k < 3) {
                    temp += a[(i + k) % actual_N];
                }
            }
            d[i] = temp * scale;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < actual_N; i++) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("First 5 values of c: ");
    for (int i = 0; i < 5 && i < actual_N; i++) {
        printf("%f ", c[i]);
    }
    printf("\n");
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
