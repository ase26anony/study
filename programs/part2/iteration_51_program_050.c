#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile = N_DEFAULT;
    
    // Read N from command line or use default
    int N = N_DEFAULT;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Make N dependent on volatile variable to prevent optimization
    N = (N_volatile > 0) ? N : N_DEFAULT;
    
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
        a[i] = (double)i * 1.5;
        b[i] = (double)(N - i) * 0.7;
        c[i] = 0.0;
        d[i] = (double)i * 2.3;
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide whether to use SIMD
        // This should help trigger IFN_GOMP_USE_SIMT
        int use_simd = (N > 100) && (thread_id % 2 == 0);
        
        // First loop: SIMD loop with runtime condition
        if (use_simd) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Complex data-dependent computation
                double temp = a[i] * b[i];
                c[i] = temp + d[i] * (i % 10);
                
                // Additional computation with conditional
                if (i % 3 == 0) {
                    c[i] += 1.0;
                }
            }
        }
        
        // Second loop: Non-SIMD loop with different stride
        // This creates contrast and additional gomp_for statements
        #pragma omp for schedule(static)
        for (int i = 0; i < N; i += 2) {
            // Different operation pattern
            a[i] = b[i] * 2.0 + thread_id * 0.1;
            
            // Nested computation
            for (int j = 0; j < 3; j++) {
                a[i] += j * 0.01;
            }
        }
        
        // Third loop: Another SIMD loop with different bounds
        // Using private clause to create more complex context
        #pragma omp for simd private(thread_id) reduction(+:checksum)
        for (int i = N/2; i < N; i++) {
            // Vectorizable reduction pattern
            checksum += c[i] * 0.5;
            
            // Additional array access with computed index
            int idx = (i * 7) % N;
            d[idx] = a[i] + b[i];
        }
        
        // Fourth loop: Mixed stride SIMD loop
        // This tests different iteration patterns
        #pragma omp for simd
        for (int i = 1; i < N; i += 3) {
            // Cross-array computation
            b[i] = a[i-1] + c[i+1 < N ? i+1 : N-1];
        }
    }
    
    // Final reduction and output to prevent dead code elimination
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Array sum: %f\n", final_sum);
    printf("N: %d\n", N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
