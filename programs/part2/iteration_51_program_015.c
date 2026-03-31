#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000

int main(int argc, char *argv[]) {
    int N = DEFAULT_N;
    
    // Use command-line argument for runtime-dependent loop bound
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = DEFAULT_N;
    }
    
    // Use volatile to prevent constant propagation
    volatile int use_simd = 1;
    
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
        
        // Runtime condition to decide SIMD usage - ensures IFN_GOMP_USE_SIMT generation
        if (use_simd && N > 100) {
            // Multiple SIMD loops with different patterns to increase coverage
            
            // SIMD loop with unit stride - likely vectorizable
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 2.5;
            }
            
            // SIMD loop with non-unit stride (stride 2)
            #pragma omp for simd
            for (int i = 0; i < N/2; i++) {
                int idx = i * 2;
                d[idx] = a[idx] * b[idx] - c[idx];
            }
            
            // SIMD loop with data-dependent array access
            #pragma omp for simd
            for (int i = 1; i < N-1; i++) {
                // Non-trivial data dependency
                a[i] = (b[i-1] + b[i] + b[i+1]) / 3.0;
            }
        } else {
            // Fallback non-SIMD path
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop in same parallel region for contrast
        #pragma omp for
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * thread_id + 1.0;
        }
        
        // Reduction loop with SIMD - nested complexity
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] * d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    
    printf("Checksum: %f, Final result: %f\n", checksum, final_result);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
