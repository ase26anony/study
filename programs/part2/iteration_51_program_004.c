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
        use_simd = atoi(argv[2]) > 0;
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
        d[i] = 1.0;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage
        // This should trigger IFN_GOMP_USE_SIMT generation
        if (use_simd && n > 100) {
            // SIMD loop with non-unit stride to increase complexity
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {
                // Data-dependent array accesses with computation
                c[i] = a[i] * b[i] + d[i];
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] - b[i + 1] * d[i];
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd schedule(static, 64)
            for (int i = 1; i < n - 1; i++) {
                // Stencil-like computation for data dependencies
                d[i] = (a[i-1] + a[i] + a[i+1]) / 3.0;
            }
        } else {
            // Non-SIMD version for comparison
            #pragma omp for
            for (int i = 0; i < n; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop in same parallel region
        // This creates multiple gomp_for statements
        #pragma omp for
        for (int i = 0; i < n; i++) {
            // Different operation to avoid fusion
            a[i] = b[i] * 2.0 + (double)thread_id;
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
