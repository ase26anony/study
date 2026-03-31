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
    // Use volatile and command-line args to prevent constant folding
    volatile int base_size = 1000;
    int n = base_size;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_size;
    }
    
    // Dynamic scaling factor
    volatile float scale_factor = 2.5f;
    float scale = scale_factor;
    
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
        // Use collapse(2) with 2D loop to increase complexity
        int m = (n > 100) ? 100 : n;
        #pragma omp target teams distribute parallel for simd collapse(2) \
                num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i += m) {
            for (int j = 0; j < m && (i + j) < n; ++j) {
                int idx = i + j;
                // Data-dependent computation with non-constant scale
                c[idx] = a[idx] + b[idx] * scale;
            }
        }
        
        // Second target region - different pattern to potentially trigger again
        #pragma omp target teams distribute parallel for simd \
                num_teams(2) thread_limit(64)
        for (int i = 0; i < n; ++i) {
            // Conditional computation to create more complex control flow
            if (c[i] > 0.0f) {
                d[i] = c[i] * 0.5f;
            } else {
                d[i] = -c[i];
            }
        }
    }
    
    // Third target region with declare target function
    #pragma omp target data map(tofrom: d[0:n])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < n; i += 2) {
            if (i + 1 < n) {
                // More complex access pattern
                d[i] = device_init ? d[i] + 1.0f : d[i];  // Reference function pointer
                d[i+1] = d[i] * scale;
            }
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    #pragma omp parallel for simd reduction(+:checksum)
    for (int i = 0; i < n; ++i) {
        checksum += (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array values at indices 0, %d/2, %d-1: %f, %f, %f\n", 
           n, n, c[0], c[n/2], c[n-1]);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
