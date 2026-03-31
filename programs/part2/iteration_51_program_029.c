#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        use_simd = atoi(argv[2]) > 0;
    }
    
    // Allocate arrays with runtime size
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100);
        b[i] = (double)((i * 3) % 97);
        c[i] = 0.0;
        d[i] = (double)(i * 2);
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage
        // This creates the branching needed for IFN_GOMP_USE_SIMT
        if (use_simd && n > 100) {
            // SIMD loop with non-unit stride and data-dependent access
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {
                // Complex data-dependent computation
                double temp = a[i] * b[i];
                c[i] = temp + d[i];
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] * b[i + 1] - d[i + 1];
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd
            for (int i = 1; i < n - 1; i++) {
                // Stencil-like computation
                a[i] = (b[i-1] + b[i] + b[i+1]) / 3.0;
            }
        } else {
            // Non-SIMD version for comparison
            #pragma omp for
            for (int i = 0; i < n; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop in same parallel region
        // This creates contrast for the transformation logic
        #pragma omp for
        for (int i = 0; i < n; i++) {
            // Different operation to avoid fusion
            d[i] = b[i] * 2.0 + (double)thread_id * 0.001;
        }
        
        // Reduction on SIMD results
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < n; i += 7) {
        final_result += a[i] * c[i] - d[i];
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
