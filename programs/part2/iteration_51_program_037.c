#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int use_simd = 1;
    int n = N;
    
    // Read iteration count from command line if provided
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    // Create arrays with runtime-dependent size
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100) * 0.1;
        b[i] = (double)((i + 1) % 50) * 0.2;
        c[i] = 0.0;
        d[i] = (double)i * 0.05;
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide whether to use SIMD
        // This should help trigger IFN_GOMP_USE_SIMD generation
        if (use_simd && n > 100) {
            // SIMD loop with data-dependent array accesses
            // Non-unit stride to make transformation more interesting
            #pragma omp for simd schedule(static) nowait
            for (int i = 0; i < n; i += 2) {
                // Complex data-dependent computation
                int idx = (i * 7) % n;  // Non-linear index
                c[i] = a[idx] * b[i] + d[(i + 1) % n];
                if (i + 1 < n) {
                    c[i + 1] = a[(i + 3) % n] - b[(i + 2) % n] * d[i];
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd schedule(dynamic, 16)
            for (int i = 1; i < n - 1; i++) {
                // Stencil-like computation
                a[i] = (b[i-1] + b[i] + b[i+1]) / 3.0;
            }
        }
        
        // Standard non-SIMD loop for contrast
        // This creates additional gomp_for statements
        #pragma omp for schedule(guided)
        for (int i = 0; i < n; i++) {
            // Different operation to avoid fusion
            d[i] = a[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        // Conditional SIMD loop based on thread ID
        // Increases chance of hitting different paths
        if (thread_id % 2 == 0) {
            #pragma omp for simd schedule(static, 8)
            for (int i = 0; i < n; i++) {
                b[i] = c[i] * d[(n - i - 1) % n];
            }
        }
        
        // Reduction loop - not SIMD to create variety
        #pragma omp for
        for (int i = 0; i < n; i++) {
            checksum += a[i] + b[i] + c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < n; i += 4) {
        final_result += a[i] * b[i] - c[i] / (d[i] + 1.0);
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    // Verify results are non-zero
    if (checksum == 0.0 || final_result == 0.0) {
        printf("Warning: Results may have been optimized away\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
