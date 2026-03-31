#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile = N_DEFAULT;
    
    // Get N from command line or use default
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
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime-dependent condition for SIMD execution
        // This should generate IFN_GOMP_USE_SIMT
        if (N > 100 || thread_id % 2 == 0) {
            // SIMD loop with non-unit stride and data-dependent access
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {
                // Complex data-dependent computation
                int idx = (i * 13) % N;  // Non-linear index
                c[idx] = a[i] * b[i] + d[i];
                
                // Additional computation with conditional
                if (i % 3 == 0) {
                    c[idx] += 2.0 * a[i];
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd
            for (int i = 1; i < N - 1; i++) {
                // Stencil-like computation
                d[i] = (a[i-1] + a[i] + a[i+1]) / 3.0;
            }
        }
        
        // Non-SIMD loop for contrast
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation pattern
            a[i] = b[i] * (2.0 + (double)(i % 5));
        }
        
        // Reduction loop with SIMD
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation outside parallel region
    // to ensure arrays are used
    double final_sum = 0.0;
    for (int i = 0; i < N; i += 4) {
        final_sum += a[i] * c[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final sum: %f\n", final_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
