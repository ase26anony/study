#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile = N_DEFAULT;
    int N = N_volatile;
    
    // Read N from command line if provided
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Runtime-dependent condition for SIMD execution
    int use_simd = (N > 100);
    
    // Allocate arrays with dynamic size
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
        a[i] = (double)i * 1.5;
        b[i] = (double)(N - i) * 0.7;
        c[i] = 0.0;
        d[i] = (double)i * 2.0;
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime condition to decide SIMD execution
        // This creates the branching logic needed for IFN_GOMP_USE_SIMT
        if (use_simd) {
            // SIMD loop with data-dependent array accesses
            // Using different iteration patterns to increase coverage
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Complex data-dependent computation
                double temp = a[i] * b[i];
                c[i] = temp + (double)(i % 10) * 0.1;
                // Additional computation to make body non-trivial
                d[i] = c[i] * (thread_id + 1) / (double)num_threads;
            }
            
            // Another SIMD loop with different stride
            #pragma omp for simd schedule(static, 16)
            for (int i = 1; i < N - 1; i += 2) {
                // Stencil-like computation for data dependencies
                a[i] = (b[i-1] + b[i] + b[i+1]) / 3.0;
            }
        }
        
        // Non-SIMD loop for contrast (standard OpenMP for)
        // This creates additional gomp_for statements
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different computation pattern
            b[i] = a[i] * 2.0 + (double)(i % 5);
        }
        
        // Another SIMD loop with runtime-dependent bounds
        int start = thread_id * (N / num_threads);
        int end = (thread_id == num_threads - 1) ? N : start + (N / num_threads);
        
        #pragma omp simd reduction(+:checksum)
        for (int i = start; i < end; i++) {
            // Reduction pattern
            checksum += c[i] + d[i];
        }
        
        // Conditional SIMD loop inside parallel region
        if (thread_id % 2 == 0) {
            #pragma omp for simd
            for (int i = 0; i < N; i += 3) {
                // Non-unit stride
                a[i] = b[i] * c[i] / (d[i] + 1.0);
            }
        }
    }
    
    // Final computation to prevent dead code elimination
    double final_result = 0.0;
    #pragma omp parallel for simd reduction(+:final_result)
    for (int i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Checksum: %f\n", checksum);
    
    // Verify results
    double verify = 0.0;
    for (int i = 0; i < N; i++) {
        verify += a[i];
    }
    printf("Verification sum: %f\n", verify);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
