#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    int N = N_DEFAULT;
    
    // Use command-line argument for runtime-dependent loop bound
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
        
        // Runtime condition to decide whether to use SIMD
        // This should trigger IFN_GOMP_USE_SIMT generation
        if (use_simd_condition && N > 100) {
            // First SIMD loop with non-unit stride - should trigger SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array access with computation
                int idx = (i * 13) % N;  // Non-linear index
                c[idx] = a[i] + b[i] * (thread_id + 1);
            }
            
            // Second SIMD loop with different bounds
            #pragma omp for simd
            for (int i = 1; i < N - 10; i++) {
                // More complex data dependency
                d[i] = a[i-1] * b[i+1] - c[(i*7) % N];
            }
        }
        
        // Non-SIMD loop for contrast - standard OpenMP for
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation to ensure separate statement
            a[i] = b[i] * 2.0 + (double)thread_id;
        }
        
        // Reduction on SIMD results
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional nested parallel region with SIMD
    #pragma omp parallel
    {
        #pragma omp for simd
        for (int i = 0; i < N/2; i++) {
            b[i] = a[i*2] * 3.14;
        }
    }
    
    // Final computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    printf("N: %d\n", N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
