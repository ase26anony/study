#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    int N = N_DEFAULT;
    
    // Use volatile to prevent constant propagation
    volatile int use_simd = 1;
    
    // Read N from command line if provided
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Runtime-dependent condition for SIMD usage
    if (argc > 2) {
        use_simd = atoi(argv[2]) > 0;
    }
    
    printf("Running with N = %d, use_simd = %d\n", N, use_simd);
    
    // Allocate arrays with runtime size
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    double *d = (double*)malloc(N * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with pattern
    for (int i = 0; i < N; i++) {
        a[i] = (double)i;
        b[i] = (double)(N - i);
        c[i] = 0.0;
        d[i] = 1.0;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage
        // This should generate IFN_GOMP_USE_SIMT call
        if (use_simd && N > 100) {
            // SIMD loop with non-unit stride to increase complexity
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array accesses with computation
                c[i] = a[i] * b[i] + d[i];
                if (i + 1 < N) {
                    c[i + 1] = a[i + 1] - b[i + 1] * d[i];
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd
            for (int i = 1; i < N - 1; i++) {
                // Stencil-like computation for data dependencies
                d[i] = (a[i-1] + a[i] + a[i+1]) / 3.0;
            }
        } else {
            // Non-SIMD version when condition is false
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop for contrast
        // This creates another gomp_for statement
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation to avoid redundancy elimination
            a[i] = b[i] * (thread_id + 1.0);
        }
        
        // Reduction for checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * c[i] - d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    // Verify results aren't trivial
    if (checksum == 0.0 && final_result == 0.0) {
        printf("Warning: Results appear to be zero\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
