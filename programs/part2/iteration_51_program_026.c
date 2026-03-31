#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    int N = N_DEFAULT;
    
    // Use command-line argument for runtime-dependent loop bounds
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Use volatile to prevent constant propagation
    volatile int use_simd_condition = 1;
    
    // Declare arrays with runtime size
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
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide whether to use SIMD
        // This should trigger IFN_GOMP_USE_SIMT generation
        if (use_simd_condition && N > 100) {
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
        }
        
        // Standard non-SIMD loop for contrast
        // This creates another gomp_for statement
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation to avoid fusion
            a[i] = b[i] * thread_id + 1.0;
        }
        
        // SIMD loop with runtime-dependent bounds
        int start = (thread_id * N) / omp_get_num_threads();
        int end = ((thread_id + 1) * N) / omp_get_num_threads();
        
        #pragma omp for simd
        for (int i = start; i < end; i++) {
            // Complex computation to ensure non-trivial body
            b[i] = c[i] * (i % 10) - d[i] / (a[i] + 1.0);
        }
        
        // Reduction with SIMD
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * b[i] - c[i] / d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    // Verify results aren't all zeros
    if (checksum == 0.0 && final_result == 0.0) {
        printf("Warning: Results may have been optimized away\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
