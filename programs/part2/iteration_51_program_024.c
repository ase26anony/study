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
        
        // Runtime-dependent condition inside parallel region
        // This creates the branching needed for IFN_GOMP_USE_SIMT
        if (use_simd && N > 100) {
            // SIMD loop with data-dependent array accesses
            // Non-unit stride to make transformation more interesting
            #pragma omp for simd nowait
            for (int i = 0; i < N - 3; i += 2) {
                // Complex data-dependent computation
                double temp1 = a[i] * b[i + 1];
                double temp2 = a[i + 1] * b[i];
                c[i] = temp1 + d[i] * 0.5;
                c[i + 1] = temp2 - d[i + 1] * 0.3;
                
                // Additional computation with condition
                if (i % 4 == 0) {
                    c[i] *= 1.1;
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd
            for (int i = 1; i < N - 1; i++) {
                // Cross-element dependency pattern
                a[i] = (b[i - 1] + b[i] + b[i + 1]) / 3.0;
            }
        }
        
        // Standard non-SIMD loop for contrast
        // This creates another gomp_for statement
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation to avoid fusion with SIMD loops
            d[i] = a[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        // Reduction loop with SIMD
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i += 4) {
        final_result += a[i] * b[i] - c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    printf("Array sizes: %d\n", N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
