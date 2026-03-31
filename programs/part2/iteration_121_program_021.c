#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
float compute_scale(float x) {
    return x * 2.0f;
}
#pragma omp end declare target

int main(int argc, char **argv) {
    // Use command-line argument to prevent constant folding
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 10000;
    }
    
    // Use volatile to prevent optimization
    volatile int dynamic_N = N;
    
    // Allocate arrays
    float *a = (float*)malloc(dynamic_N * sizeof(float));
    float *b = (float*)malloc(dynamic_N * sizeof(float));
    float *c = (float*)malloc(dynamic_N * sizeof(float));
    float *d = (float*)malloc(dynamic_N * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < dynamic_N; i++) {
        a[i] = (float)i;
        b[i] = (float)(dynamic_N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Non-constant scale factor
    float scale = 1.5f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // First target region with teams distribute parallel for simd
    // Using collapse(2) for 2D loop to increase complexity
    #pragma omp target data map(to: a[0:dynamic_N], b[0:dynamic_N]) \
                            map(from: c[0:dynamic_N])
    {
        // This should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < dynamic_N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < dynamic_N) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Second similar offloaded loop with different pattern
        // to potentially trigger transformation again
        #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(64)
        for (int i = 0; i < dynamic_N; i++) {
            // Use function call inside declare target region
            float local_scale = compute_scale(scale);
            c[i] += a[i] * local_scale - b[i];
        }
    }
    
    // Another target region with different data mapping
    // to increase chance of hitting the transformation
    #pragma omp target data map(to: c[0:dynamic_N]) map(from: d[0:dynamic_N])
    {
        // Third offloaded loop with conditional computation
        #pragma omp target teams distribute parallel for simd \
                num_teams(16) thread_limit(256)
        for (int i = 0; i < dynamic_N; i++) {
            if (i % 2 == 0) {
                d[i] = c[i] * 2.0f;
            } else {
                d[i] = c[i] / 2.0f;
            }
        }
        
        // Fourth loop with reduction-like pattern
        float sum = 0.0f;
        #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) num_teams(4)
        for (int i = 0; i < dynamic_N; i++) {
            sum += d[i];
        }
        
        printf("Final sum: %f\n", sum);
    }
    
    // Verify computation
    float host_sum = 0.0f;
    for (int i = 0; i < dynamic_N; i++) {
        host_sum += d[i];
    }
    printf("Host sum: %f\n", host_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
