#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N = N_DEFAULT;
    
    // Read N from command line if provided
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Allocate arrays with dynamic size
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    double *d = (double*)malloc(N * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < N; i++) {
        a[i] = (double)(i % 100) * 0.5;
        b[i] = (double)((i + 1) % 50) * 1.5;
        c[i] = 0.0;
        d[i] = (double)i * 2.0;
    }
    
    double checksum = 0.0;
    
    // Runtime condition to control SIMD execution
    int use_simd = (N > 100);  // This creates runtime dependency
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime-dependent condition for SIMD transformation
        if (use_simd) {
            // This should trigger the SIMT transformation
            // Non-unit stride to create more complex loop structure
            #pragma omp for simd schedule(static) nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array accesses with computation
                int idx = (i * thread_id) % N;
                if (idx >= 0 && idx < N) {
                    c[idx] = a[idx] * b[idx] + (double)thread_id;
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd schedule(dynamic, 16)
            for (int i = 1; i < N; i += 3) {
                // Complex data dependency
                double temp = a[i] * 2.0;
                b[i] = temp + d[i % (N/2 + 1)];
                if (i > 0) {
                    a[i] = b[i-1] * 0.5;
                }
            }
        } else {
            // Fallback non-SIMD path
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop for contrast
        // This creates multiple gomp_for statements
        #pragma omp for schedule(guided)
        for (int i = 0; i < N; i++) {
            d[i] = a[i] * 3.0 + (double)(i % 10);
        }
        
        // Nested loop to create more complex control flow
        #pragma omp for collapse(2)
        for (int i = 0; i < N/2; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < N) {
                    a[idx] = b[idx] * (double)j;
                }
            }
        }
        
        // Local reduction to prevent optimization
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to ensure arrays are used
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f, Final sum: %f\n", checksum, final_sum);
    
    // Clean up
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
