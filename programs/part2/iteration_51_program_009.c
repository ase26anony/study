#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N = N_DEFAULT;
    
    // Read N from command line if provided
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Allocate arrays with dynamic size to avoid static analysis
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
        a[i] = (double)(i % 100);
        b[i] = (double)((i + 1) % 100);
        c[i] = 0.0;
        d[i] = (double)(i * 2);
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime-dependent condition for SIMD execution
        // This creates the branching logic needed for IFN_GOMP_USE_SIMT
        int use_simd = (N > 100) && (thread_id % 2 == 0);
        
        // First loop: SIMD transformation candidate
        // The if condition ensures runtime decision for SIMD
        if (use_simd) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Data-dependent computation with array accesses
                // Non-trivial index calculation to prevent optimization
                int idx = (i * 17) % N;  // Non-linear access pattern
                c[idx] = a[i] + b[(i + thread_id) % N] * 2.5;
                
                // Additional computation to make body substantial
                d[i] = c[idx] * 0.7 + d[(i + 1) % N];
            }
        }
        
        // Second loop: Non-SIMD for contrast
        // Different stride to create different iteration pattern
        #pragma omp for schedule(static) nowait
        for (int i = 0; i < N; i += 2) {
            // Different operation for contrast
            a[i] = b[i] * 3.0 + d[i];
            
            // Conditional update to create control flow
            if (i % 3 == 0) {
                b[i] = a[i] / 2.0;
            } else {
                b[i] = a[i] * 1.5;
            }
        }
        
        // Third loop: Another SIMD loop with different bounds
        // This creates multiple gomp_for statements
        if (N > 500) {
            #pragma omp for simd
            for (int i = N/2; i < N; i++) {
                // Complex data dependency chain
                double temp = c[i] * a[i] - b[i];
                c[i] = temp + d[(i - 1 + N) % N];
                d[i] = temp * 0.3 + c[(i + 2) % N];
            }
        }
        
        // Local reduction to prevent dead code elimination
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to ensure arrays are used
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f, Final sum: %f\n", checksum, final_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
