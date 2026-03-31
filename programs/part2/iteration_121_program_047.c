#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_compute(float* a, float* b, float* c, int n, float scale) {
    // Empty - just to ensure declare target region exists
}
#pragma omp end declare target

int main(int argc, char** argv) {
    // Use volatile and command-line args to prevent constant folding
    volatile int base_n = 10000;
    int n = base_n;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_n;
    }
    
    // Non-constant scale to prevent optimization
    volatile float scale_base = 2.5f;
    float scale = scale_base;
    if (argc > 2) {
        scale = atof(argv[2]);
    }
    
    // Allocate arrays
    float* a = (float*)malloc(n * sizeof(float));
    float* b = (float*)malloc(n * sizeof(float));
    float* c = (float*)malloc(n * sizeof(float));
    float* d = (float*)malloc(n * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = (float)i;
        b[i] = (float)(n - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // First target region with teams distribute parallel for simd
    // This should trigger SIMT transformation
    #pragma omp target data map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        // Use collapse(2) to create 2D loop structure
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
        
        // Second similar offloaded loop with different computation
        // Increases chance of hitting the transformation again
        #pragma omp target teams distribute parallel for simd \
            num_teams(2) thread_limit(256)
        for (int i = 0; i < n; i++) {
            // Conditional computation to prevent simple vectorization
            if (a[i] > b[i]) {
                c[i] = c[i] * scale - b[i];
            } else {
                c[i] = c[i] + a[i] * 0.5f;
            }
        }
    }
    
    // Third target region with different data mapping
    // Another opportunity for SIMT transformation
    #pragma omp target data map(to: a[0:n]) map(tofrom: d[0:n])
    {
        // Reduction pattern - different enough from previous loops
        float sum = 0.0f;
        #pragma omp target teams distribute parallel for simd \
            reduction(+:sum) num_teams(3)
        for (int i = 0; i < n; i++) {
            d[i] = a[i] * a[i];
            sum += d[i];
        }
        
        // Use the reduction result to prevent dead code elimination
        volatile float dummy = sum;
        (void)dummy;
    }
    
    // Compute checksum to ensure computation happened
    float checksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("First/last values: c[0]=%f, c[%d]=%f, d[0]=%f, d[%d]=%f\n",
           c[0], n-1, c[n-1], d[0], n-1, d[n-1]);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
