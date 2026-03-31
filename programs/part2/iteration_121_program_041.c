#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_init(float *arr, int n, float val) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        arr[i] = val;
    }
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    // Use volatile and command-line args to prevent constant propagation
    volatile int base_n = 1000;
    int n = base_n;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_n;
    }
    
    // Dynamic scaling factor
    float scale = (argc > 2) ? atof(argv[2]) : 2.5f;
    
    // Allocate arrays
    float *a = (float*)malloc(n * sizeof(float));
    float *b = (float*)malloc(n * sizeof(float));
    float *c = (float*)malloc(n * sizeof(float));
    float *d = (float*)malloc(n * sizeof(float));
    
    // Initialize on host
    for (int i = 0; i < n; ++i) {
        a[i] = (float)i;
        b[i] = (float)(n - i);
    }
    
    // First target region with teams distribute parallel for simd
    #pragma omp target data map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        // This should trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < n/2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int idx = i * 2 + j;
                if (idx < n) {
                    c[idx] = a[idx] + b[idx] * scale;
                }
            }
        }
        
        // Second target region with different pattern
        // Uses declare target function and another SIMD loop
        #pragma omp target teams distribute parallel for simd \
                num_teams(2) thread_limit(64)
        for (int i = 0; i < n; ++i) {
            d[i] = 0.0f;
        }
        
        #pragma omp target teams distribute parallel for simd \
                num_teams(3) thread_limit(256)
        for (int i = 0; i < n; ++i) {
            // Conditional computation to prevent over-optimization
            if (c[i] > 0.0f) {
                d[i] = c[i] * 0.5f - a[i];
            } else {
                d[i] = c[i] * 2.0f + b[i];
            }
        }
    }
    
    // Third target region with reduction (different pattern)
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
            reduction(+:sum) map(tofrom: sum) map(to: c[0:n])
    for (int i = 0; i < n; ++i) {
        sum += c[i];
        // Additional computation to make loop body non-trivial
        c[i] = c[i] / (scale + 1.0f);
    }
    
    // Verify results
    printf("Array size: %d\n", n);
    printf("Scale factor: %.2f\n", scale);
    printf("Checksum c: %.2f\n", sum);
    
    // Compute checksum for d
    float sum_d = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum_d += d[i];
    }
    printf("Checksum d: %.2f\n", sum_d);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
