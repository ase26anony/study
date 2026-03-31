#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_init(float *a, float *b, int n, float scale) {
    // Simple initialization function for device
    for (int i = 0; i < n; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
    }
}
#pragma omp end declare target

int main(int argc, char **argv) {
    // Use volatile and command-line args to prevent constant folding
    volatile int base_n = 10000;
    int n = base_n;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_n;
    }
    
    // Dynamic scale factor
    float scale = 2.5f;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // Allocate arrays
    float *a = (float*)malloc(n * sizeof(float));
    float *b = (float*)malloc(n * sizeof(float));
    float *c = (float*)malloc(n * sizeof(float));
    float *d = (float*)malloc(n * sizeof(float));
    
    // Initialize host arrays
    for (int i = 0; i < n; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 0.5f;
    }
    
    // First target region with teams distribute parallel for simd
    // Using collapse(2) to create 2D loop structure
    #pragma omp target data map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        // Initialize on device
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < n; i++) {
            c[i] = 0.0f;
        }
        
        // First SIMT-transformed loop: vector addition with scaling
        // Using dynamic bounds and non-constant scale
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < n) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Second target region with different computation pattern
        // This may trigger the transformation again
        #pragma omp target data map(tofrom: d[0:n])
        {
            // Conditional update with SIMD - increases transformation likelihood
            #pragma omp target teams distribute parallel for simd \
                    num_teams(8) thread_limit(64)
            for (int i = 0; i < n; i++) {
                if (c[i] > 100.0f) {
                    d[i] = c[i] * 0.5f;
                } else {
                    d[i] = c[i] * 2.0f;
                }
            }
            
            // Third loop: reduction-like pattern
            float sum = 0.0f;
            #pragma omp target teams distribute parallel for simd \
                    reduction(+:sum) map(tofrom: sum)
            for (int i = 0; i < n; i++) {
                sum += d[i];
            }
            
            printf("Intermediate sum: %f\n", sum);
        }
    }
    
    // Another similar offloaded region with different parameters
    // Using volatile variable for loop bound
    volatile int m = n / 2;
    float *e = (float*)malloc(n * sizeof(float));
    
    #pragma omp target data map(to: c[0:n]) map(from: e[0:n])
    {
        // This should also trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
                num_teams(2) num_threads(256)
        for (int i = 0; i < m; i++) {
            // Complex addressing pattern
            int idx = (i * 7) % n;
            e[idx] = c[idx] * c[(idx + 1) % n];
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < n; i++) {
        checksum += c[i] + d[i] + e[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Array sizes used: n=%d, m=%d, scale=%.2f\n", n, m, scale);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    return 0;
}
