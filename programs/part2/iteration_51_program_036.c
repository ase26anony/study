#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000

int main(int argc, char *argv[]) {
    int N = DEFAULT_N;
    
    // Use command-line argument for runtime-dependent loop bounds
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = DEFAULT_N;
    }
    
    // Use volatile to prevent constant propagation
    volatile int use_simd_condition = 1;
    
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
        
        // Runtime condition to decide SIMD usage - this creates branching
        // that should trigger IFN_GOMP_USE_SIMT generation
        if (use_simd_condition && N > 100) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Data-dependent array accesses with non-trivial computation
                c[i] = a[i] + b[i] * (thread_id + 1) / (double)omp_get_num_threads();
            }
            
            // Another SIMD loop with different stride
            #pragma omp for simd schedule(static, 32)
            for (int i = 0; i < N - 1; i += 2) {
                d[i] = c[i] * c[i + 1];
                if (i > 0) {
                    d[i] += a[i - 1] * 0.5;
                }
            }
        } else {
            // Fallback path without SIMD
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Contrast: Standard non-SIMD loop in same parallel region
        #pragma omp for
        for (int i = 0; i < N; i++) {
            a[i] = b[i] * 2.0 + (double)thread_id;
        }
        
        // Reduction for checksum to prevent dead code elimination
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i] + a[i];
        }
    }
    
    // Additional computation to ensure arrays are used
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += c[i] * d[i];
    }
    
    printf("Checksum: %f, Final result: %f\n", checksum, final_result);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
