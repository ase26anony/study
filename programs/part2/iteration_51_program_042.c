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
    
    // Allocate and initialize arrays
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    double *d = (double*)malloc(N * sizeof(double));
    
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
        
        // Runtime condition to decide whether to use SIMD
        // This should generate the IFN_GOMP_USE_SIMT call
        if (use_simd && N > 100) {
            // SIMD loop with non-unit stride to increase complexity
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array accesses
                int idx = (i * 13) % N;  // Non-linear index
                c[i] = a[idx] + b[i] * d[idx];
                
                // Additional computation to make body non-trivial
                if (i > 0) {
                    c[i] += a[i-1] * 0.5;
                }
            }
            
            // Second SIMD loop with different bounds
            #pragma omp for simd
            for (int i = 1; i < N-1; i++) {
                // Cross-element dependency
                b[i] = (a[i-1] + a[i] + a[i+1]) / 3.0;
            }
        }
        
        // Non-SIMD loop for contrast (standard OpenMP for)
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation
            d[i] = a[i] * b[i] + (double)thread_id * 0.01;
        }
        
        // Reduction on SIMD results
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    
    // Additional verification to prevent dead code elimination
    double verify = 0.0;
    for (int i = 0; i < N; i++) {
        verify += a[i] + b[i];
    }
    printf("Array sum verification: %f\n", verify);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
