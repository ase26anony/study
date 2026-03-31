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
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage - creates branching
        if (use_simd_condition && N > 100) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Data-dependent array access with computation
                int idx = (i * 7 + thread_id) % N;  // Non-trivial index
                c[idx] = a[i] + b[i] * (thread_id + 1);
            }
            
            // Another SIMD loop with different stride
            #pragma omp for simd schedule(static, 32)
            for (int i = 1; i < N - 1; i += 2) {
                // Strided access pattern
                d[i] = a[i-1] + a[i] + a[i+1];
            }
        } else {
            // Fallback without SIMD
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop in same parallel region
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation for contrast
            a[i] = b[i] * 2.0 + (double)thread_id;
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
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
