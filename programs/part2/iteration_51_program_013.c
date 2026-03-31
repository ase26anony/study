#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    int N = N_DEFAULT;
    
    // Use command-line argument to make N runtime-dependent
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Use volatile to prevent constant propagation
    volatile int use_simd_condition = 1;
    
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
        int num_threads = omp_get_num_threads();
        
        // Runtime condition to decide whether to use SIMD
        // This should help trigger IFN_GOMP_USE_SIMT generation
        if (use_simd_condition && N > 100) {
            // SIMD loop with non-unit stride in some cases
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 1 + (i % 3 == 0 ? 1 : 0)) {
                // Complex data-dependent computation
                c[i] = a[i] * 2.5 + b[i] / (1.0 + (double)(i % 10));
                // Additional array access with computed index
                int idx = (i + thread_id) % N;
                d[idx] = c[i] * 0.5;
            }
        }
        
        // Non-SIMD loop for contrast - should create a different gomp_for
        #pragma omp for schedule(static)
        for (int i = 0; i < N; i++) {
            // Different operation pattern
            a[i] = b[i] * (1.0 + (double)(thread_id % 4));
            // Cross-thread data dependency simulation
            if (i > 0) {
                b[i] += a[i-1] * 0.1;
            }
        }
        
        // Another SIMD loop with different bounds
        if (N < 5000) {
            #pragma omp for simd reduction(+:checksum)
            for (int i = 0; i < N; i++) {
                checksum += c[i] + d[i];
            }
        }
    }
    
    // Final computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Checksum: %f\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
