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
    volatile int use_simd = 1;
    
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
    
    // Outer parallel region - creates the context for SIMT transformation
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide whether to use SIMD
        // This should trigger IFN_GOMP_USE_SIMT generation
        if (use_simd && N > 100) {
            // SIMD loop with non-unit stride in some cases
            // Multiple loops with different patterns
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 1) {
                // Data-dependent array access with computation
                c[i] = a[i] + b[i] * (i % 10 + 1);
            }
            
            // Another SIMD loop with different bounds
            #pragma omp for simd
            for (int i = 1; i < N - 1; i++) {
                // More complex data dependency
                d[i] = (a[i-1] + a[i] + a[i+1]) / 3.0;
            }
        }
        
        // Non-SIMD loop for contrast - standard OpenMP for
        // This creates a different gomp_for statement
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation to avoid fusion
            a[i] = b[i] * 2.0 + (double)thread_id;
        }
        
        // Reduction loop with SIMD - nested in same parallel region
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
        
        // Conditional SIMD loop based on thread ID
        // Adds more complexity to the control flow
        if (thread_id % 2 == 0) {
            #pragma omp for simd
            for (int i = 0; i < N; i += 2) {
                b[i] = c[i] * d[i];
            }
        }
    }
    
    // Additional computation outside parallel region
    // to ensure arrays are used
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
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
