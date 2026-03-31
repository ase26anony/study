#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare target
void device_compute(float* a, float* b, float* c, int n, float scale) {
    // Empty - just to declare target
}
#pragma omp end declare target

int main(int argc, char** argv) {
    // Use volatile and command-line args to prevent constant folding
    volatile int base_n = 1000;
    int n = base_n;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = base_n;
    }
    
    // Additional volatile variable to prevent optimizations
    volatile float scale = 2.5f;
    
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
    // This should trigger the SIMT transformation
    #pragma omp target data map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        #pragma omp target teams distribute parallel for simd \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < n; i++) {
            // Use scale (volatile) to prevent constant propagation
            c[i] = a[i] + b[i] * scale;
        }
        
        // Second target region with different loop structure
        // Using collapse(2) on a 2D loop to increase complexity
        int m = (n > 100) ? 100 : n;
        #pragma omp target teams distribute parallel for simd collapse(2) \
            num_teams(2) thread_limit(64)
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                if (idx < n) {
                    // Different computation pattern
                    c[idx] = c[idx] * 0.5f + a[idx] - b[idx];
                }
            }
        }
    }
    
    // Third target region with conditional update
    // This creates different control flow
    #pragma omp target data map(to: a[0:n], b[0:n]) map(tofrom: d[0:n])
    {
        float threshold = (float)(n / 2);
        #pragma omp target teams distribute parallel for simd \
            num_teams(3) thread_limit(256)
        for (int i = 0; i < n; i++) {
            // Conditional computation to create more complex CFG
            if (a[i] > threshold) {
                d[i] = a[i] * b[i] / scale;
            } else {
                d[i] = a[i] + b[i] - scale;
            }
        }
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum_c = 0.0;
    double checksum_d = 0.0;
    for (int i = 0; i < n; i++) {
        checksum_c += c[i];
        checksum_d += d[i];
    }
    
    printf("Checksum c: %f\n", checksum_c);
    printf("Checksum d: %f\n", checksum_d);
    printf("Scale used: %f\n", (float)scale);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
