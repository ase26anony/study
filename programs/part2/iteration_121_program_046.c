#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_compute(float* a, float* b, float* c, int n, float scale) {
    // Empty - just to mark function for offloading
}
#pragma omp end declare target

int main(int argc, char** argv) {
    // Use command-line input to prevent constant propagation
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 10000;
    }
    
    // Use volatile to prevent optimization
    volatile int dynamic_N = N;
    int actual_N = dynamic_N;
    
    // Allocate arrays
    float* a = (float*)malloc(actual_N * sizeof(float));
    float* b = (float*)malloc(actual_N * sizeof(float));
    float* c = (float*)malloc(actual_N * sizeof(float));
    float* d = (float*)malloc(actual_N * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < actual_N; i++) {
        a[i] = (float)i;
        b[i] = (float)(actual_N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Non-constant scale to prevent optimization
    float scale = (argc > 2) ? atof(argv[2]) : 2.5f;
    volatile float volatile_scale = scale;
    float actual_scale = volatile_scale;
    
    // First target region with teams distribute parallel for simd
    // This should trigger the SIMT transformation
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) \
                            map(from: c[0:actual_N])
    {
        // Use collapse(2) to create 2D loop structure
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < actual_N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < actual_N) {
                    c[idx] = a[idx] + b[idx] * actual_scale;
                }
            }
        }
        
        // Second similar offloaded loop with different computation
        // Increases chance of hitting the transformation again
        #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(64)
        for (int i = 0; i < actual_N; i++) {
            // Conditional update to create more complex control flow
            if (c[i] > 100.0f) {
                c[i] = c[i] * 0.5f;
            } else {
                c[i] = c[i] * 2.0f;
            }
        }
    }
    
    // Another target region with different loop structure
    #pragma omp target data map(to: a[0:actual_N]) map(tofrom: d[0:actual_N])
    {
        // Use dynamic scheduling to create more complex code generation
        #pragma omp target teams distribute parallel for simd \
                schedule(static, 256) num_teams(2)
        for (int i = 0; i < actual_N; i++) {
            // Reduction-like pattern
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                if (i + k < actual_N) {
                    sum += a[i + k] * k;
                }
            }
            d[i] = sum;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < actual_N; i++) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("First few values: c[0]=%f, c[1]=%f, d[0]=%f\n", 
           c[0], c[1], d[0]);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
