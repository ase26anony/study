#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_init(float *a, float *b, int n, float scale) {
    // Simple initialization function for device
    for (int i = 0; i < n; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 0.5f;
    }
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
    volatile int dynamic_N = N;
    int actual_N = dynamic_N;
    
    // Allocate arrays
    float *a = (float*)malloc(actual_N * sizeof(float));
    float *b = (float*)malloc(actual_N * sizeof(float));
    float *c = (float*)malloc(actual_N * sizeof(float));
    float *d = (float*)malloc(actual_N * sizeof(float));
    
    // Initialize on host
    for (int i = 0; i < actual_N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 0.5f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Non-constant scale factor
    float scale = 2.0f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // First target region with teams distribute parallel for simd
    #pragma omp target data map(to: a[0:actual_N], b[0:actual_N]) \
                            map(from: c[0:actual_N])
    {
        // Complex loop structure to trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
                    collapse(2) num_teams(8) thread_limit(128)
        for (int i = 0; i < actual_N/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < actual_N) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Second similar offloaded loop with different pattern
        // Using dynamic schedule to increase transformation complexity
        #pragma omp target teams distribute parallel for simd \
                    schedule(static, 256) num_teams(4)
        for (int i = 0; i < actual_N; i++) {
            // Conditional computation to prevent simplification
            if (i % 3 == 0) {
                c[i] = c[i] * 2.0f - b[i];
            } else {
                c[i] = c[i] + a[i] * 0.5f;
            }
        }
    }
    
    // Another target region with different data mapping
    #pragma omp target data map(to: a[0:actual_N]) \
                            map(tofrom: d[0:actual_N])
    {
        // Loop with reduction-like pattern
        float local_sum = 0.0f;
        #pragma omp target teams distribute parallel for simd \
                    reduction(+:local_sum) collapse(2)
        for (int i = 0; i < actual_N/4; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < actual_N) {
                    d[idx] = a[idx] * c[idx];
                    local_sum += d[idx];
                }
            }
        }
        
        // Additional computation to use the reduction result
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < actual_N; i++) {
            d[i] = d[i] / (local_sum + 1.0f);
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < actual_N; i++) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array sizes: c[%d], d[%d]\n", actual_N, actual_N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
