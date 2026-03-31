#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int n = N;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    
    // Declare arrays with runtime-dependent size
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100);
        b[i] = (double)((i * 3) % 97);
        c[i] = 0.0;
        d[i] = 0.0;
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime-dependent condition to trigger SIMT transformation
        int use_simd = (n > 100) && (thread_id == 0);
        
        // First: SIMD loop with runtime condition
        if (use_simd) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                // Data-dependent array accesses with non-unit stride pattern
                int idx = (i * 2) % n;
                c[idx] = a[i] + b[(i + 1) % n] * 2.5;
            }
        }
        
        // Second: Non-SIMD loop for contrast
        #pragma omp for
        for (int i = 0; i < n; i++) {
            // Different operation pattern
            d[i] = a[i] * b[i] - 1.5;
        }
        
        // Third: Another SIMD loop with different bounds
        #pragma omp for simd
        for (int i = n/4; i < 3*n/4; i++) {
            // Complex data dependency
            a[i] = c[(i + n/2) % n] + d[i] * 0.75;
        }
        
        // Calculate partial checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += a[i] + b[i] + c[i] + d[i];
        }
    }
    
    // Additional SIMD loop outside parallel region
    // This creates another context for the transformation
    #pragma omp parallel simd
    for (int i = 0; i < n; i += 2) {
        b[i] = a[i] * 1.1;
    }
    
    printf("Checksum: %f\n", checksum);
    
    // Verify results with another SIMD operation
    double final_sum = 0.0;
    #pragma omp simd reduction(+:final_sum)
    for (int i = 0; i < n; i++) {
        final_sum += a[i] + b[i];
    }
    printf("Final sum: %f\n", final_sum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
