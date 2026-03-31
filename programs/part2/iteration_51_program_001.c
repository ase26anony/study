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
        
        // Runtime condition to decide SIMD usage - this creates branching
        if (use_simd_condition && N > 100) {
            // This should trigger SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Data-dependent array access with computation
                int idx = (i + thread_id) % N;
                c[idx] = a[i] * b[i] + (double)thread_id;
                
                // Additional computation to make loop body substantial
                if (i % 2 == 0) {
                    d[i] = c[idx] * 0.5;
                } else {
                    d[i] = c[idx] * 2.0;
                }
            }
        }
        
        // Non-SIMD loop for contrast - standard OpenMP for
        #pragma omp for
        for (int i = 0; i < N; i += 2) {  // Non-unit stride
            // Different operation pattern
            a[i] = b[i] * 3.14159 + (double)thread_id;
            
            // Nested condition to create more complex control flow
            if (a[i] > 1000.0) {
                b[i] = a[i] / 2.0;
            } else {
                b[i] = a[i] * 2.0;
            }
        }
        
        // Another SIMD loop with different bounds
        if (N < 5000) {
            #pragma omp for simd schedule(static, 64)
            for (int i = N/2; i < N; i++) {
                // Vectorizable reduction-like operation
                c[i] += a[i] - b[i];
                d[i] = c[i] * d[i];
            }
        }
        
        // Local reduction for checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional sequential computation to prevent optimization
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * 0.1 + b[i] * 0.2 + c[i] * 0.3 + d[i] * 0.4;
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    // Verify results aren't all zeros
    if (checksum == 0.0 || final_result == 0.0) {
        printf("Warning: Results may have been optimized away\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
