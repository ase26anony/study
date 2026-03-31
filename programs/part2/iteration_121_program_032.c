#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Declare target functions for offloading
#pragma omp declare target
float compute_value(float a, float b, float scale) {
    return a + b * scale;
}

int conditional_update(int val, int threshold) {
    return (val > threshold) ? val * 2 : val;
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use command-line arguments to prevent constant folding
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
    int *d = (int*)malloc(actual_N * sizeof(int));
    int *e = (int*)malloc(actual_N * sizeof(int));
    
    for (int i = 0; i < actual_N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(actual_N - i) * 0.05f;
        d[i] = i % 100;
    }
    
    // Non-constant scale to prevent optimization
    float scale = (argc > 2) ? atof(argv[2]) : 2.5f;
    volatile float volatile_scale = scale;
    float actual_scale = volatile_scale;
    
    // First target region with teams distribute parallel for simd
    // This should trigger SIMT transformation
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
                    c[idx] = compute_value(a[idx], b[idx], actual_scale);
                }
            }
        }
        
        // Second target region with different pattern
        // Increases chance of hitting the transformation again
        int threshold = (argc > 3) ? atoi(argv[3]) : 50;
        volatile int volatile_threshold = threshold;
        int actual_threshold = volatile_threshold;
        
        #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(64)
        for (int i = 0; i < actual_N; i++) {
            e[i] = conditional_update(d[i], actual_threshold);
        }
    }
    
    // Third target region with reduction pattern
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
            reduction(+:sum) map(to: c[0:actual_N]) map(tofrom: sum)
    for (int i = 0; i < actual_N; i++) {
        sum += c[i];
    }
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    #pragma omp parallel for simd reduction(+:checksum)
    for (int i = 0; i < actual_N; i++) {
        checksum += e[i];
    }
    
    printf("Results: sum(c) = %f, checksum(e) = %d\n", sum, checksum);
    printf("Array sizes: N = %d, scale = %f, threshold = %d\n", 
           actual_N, actual_scale, (argc > 3) ? atoi(argv[3]) : 50);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    return 0;
}
