#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1000

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
    
    // Declare arrays with runtime size
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = (double)i;
        b[i] = (double)(n - i);
        c[i] = 0.0;
        d[i] = 0.0;
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage
        // This creates the branching needed for IFN_GOMP_USE_SIMT
        if (use_simd && n > 100) {
            // SIMD loop with non-unit stride to increase transformation likelihood
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {
                // Data-dependent array accesses with computation
                c[i] = a[i] * b[i] + (double)thread_id;
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] * b[i + 1] - (double)thread_id;
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd
            for (int i = 1; i < n; i += 3) {
                d[i] = a[i] * 2.0 + b[i] * 0.5;
                if (i + 1 < n) d[i + 1] = a[i + 1] * 1.5;
                if (i + 2 < n) d[i + 2] = b[i + 2] * 2.5;
            }
        } else {
            // Non-SIMD version when condition is false
            #pragma omp for
            for (int i = 0; i < n; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop for contrast
        // This creates another gomp_for statement for scan_omp_for
        #pragma omp for
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        // Reduction for checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < n; i++) {
        final_result += a[i] * 0.1 + c[i] * 0.2 + d[i] * 0.3;
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
