#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile = N_DEFAULT;
    
    // Read N from command line or use default
    int N = (argc > 1) ? atoi(argv[1]) : N_volatile;
    
    // Ensure N is positive and reasonable
    if (N <= 0) N = N_DEFAULT;
    
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
    
    // Runtime condition that will be evaluated inside parallel region
    int use_simd = 0;
    if (N > 100) {
        use_simd = 1;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime-dependent condition inside parallel region
        // This creates the branching needed for IFN_GOMP_USE_SIMT
        if (use_simd) {
            // First SIMD loop with non-unit stride - should trigger SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array access with computation
                int idx = (i * 3) % N;
                c[idx] = a[i] + b[i] * (thread_id + 1);
            }
            
            // Second SIMD loop with different pattern
            #pragma omp for simd
            for (int i = 1; i < N; i += 3) {
                // More complex data dependency
                d[i] = a[(i + thread_id) % N] * b[(i * 2) % N];
                if (d[i] > 100.0) {
                    d[i] = 100.0;
                }
            }
        } else {
            // Fallback non-SIMD path
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop for contrast - creates another gomp_for statement
        #pragma omp for
        for (int i = 0; i < N; i++) {
            a[i] = b[i] * 2.0 + (double)thread_id;
        }
        
        // Reduction computation
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * 0.5 + c[i] * 0.3;
    }
    
    printf("Checksum: %f, Final result: %f\n", checksum, final_result);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
