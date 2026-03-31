#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int use_simd = 1;
    int n = N;
    
    // Read iteration count from command line if provided
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    // Runtime-dependent condition for SIMD usage
    if (argc > 2) {
        use_simd = atoi(argv[2]);
    }
    
    // Declare arrays with data dependencies
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.0;
        c[i] = 0.0;
        d[i] = i * 0.5;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage
        // This should generate IFN_GOMP_USE_SIMT and conditional branching
        if (use_simd && n > 100) {
            // SIMD loop with non-unit stride to increase transformation likelihood
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {
                // Data-dependent array accesses with computation
                c[i] = a[i] + b[i] * d[i];
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] - b[i + 1] / (d[i + 1] + 1.0);
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd
            for (int i = 1; i < n - 1; i++) {
                // Cross-element dependency pattern
                a[i] = (b[i-1] + b[i] + b[i+1]) / 3.0;
            }
        }
        
        // Non-SIMD loop for contrast (standard OpenMP for)
        #pragma omp for
        for (int i = 0; i < n; i++) {
            // Different operation to ensure distinct loop body
            d[i] = a[i] * 2.0 + thread_id * 0.01;
        }
        
        // Reduction for checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional SIMD region outside parallel for more coverage
    if (use_simd) {
        #pragma omp parallel for simd
        for (int i = 0; i < n; i++) {
            b[i] = c[i] * d[i] / (a[i] + 1.0);
        }
    }
    
    printf("Checksum: %f\n", checksum);
    
    // Prevent dead code elimination
    volatile double dummy = a[n/2] + b[n/3] + c[n/4] + d[n/5];
    (void)dummy;
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
