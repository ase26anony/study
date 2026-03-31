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
    volatile int use_simd = 1;
    
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
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage - creates branching
        if (N > 100 && use_simd) {
            // This should trigger the SIMT transformation
            // Multiple loops with different patterns
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Data-dependent array accesses with computation
                c[i] = a[i] + b[i] * (i % 10 + 1);
            }
            
            // Another SIMD loop with different stride
            #pragma omp for simd schedule(static, 64)
            for (int i = 1; i < N - 1; i += 2) {
                // More complex data dependency
                d[i] = c[i-1] + c[i] + c[i+1];
            }
        }
        
        // Non-SIMD loop for contrast - standard OpenMP for
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation to ensure separate loop body
            a[i] = b[i] * 2.0 + (double)thread_id * 0.001;
        }
        
        // Reduction computation
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional SIMD region outside parallel to test different context
    if (N < 2000) {
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 0.5;
        }
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final values: a[0]=%f, b[0]=%f, c[0]=%f, d[0]=%f\n", 
           a[0], b[0], c[0], d[0]);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
