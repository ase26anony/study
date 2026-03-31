#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000

int main(int argc, char *argv[]) {
    int N = DEFAULT_N;
    
    // Use volatile to prevent constant propagation
    volatile int use_simd = 1;
    
    // Read N from command line if provided
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = DEFAULT_N;
    }
    
    // Runtime condition for SIMD usage
    if (argc > 2) {
        use_simd = atoi(argv[2]);
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
        d[i] = (double)i * 2.0;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime-dependent condition for SIMD execution
        // This should trigger IFN_GOMP_USE_SIMT generation
        if (use_simd && N > 100) {
            // SIMD loop with non-unit stride to create interesting pattern
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array accesses with computation
                c[i] = a[i] * b[i] + d[i];
                if (i + 1 < N) {
                    c[i + 1] = a[i + 1] - b[i + 1] * d[i];
                }
            }
            
            // Another SIMD loop with different bounds
            #pragma omp for simd
            for (int i = 1; i < N - 1; i++) {
                // More complex data dependencies
                double temp = (a[i-1] + a[i] + a[i+1]) / 3.0;
                b[i] = temp * temp - c[i];
            }
        } else {
            // Non-SIMD version when condition is false
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop for contrast
        // This creates another gomp_for statement
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation to avoid redundancy elimination
            d[i] = a[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        // SIMD loop with runtime-dependent bounds
        int start = thread_id * (N / num_threads);
        int end = (thread_id == num_threads - 1) ? N : (thread_id + 1) * (N / num_threads);
        
        #pragma omp for simd
        for (int i = start; i < end; i++) {
            // Complex computation to ensure non-trivial loop body
            a[i] = b[i] * c[i] + d[i] * (double)i / N;
        }
        
        // Local reduction for checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += a[i] + b[i] + c[i] + d[i];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * b[i] - c[i] / (d[i] + 1.0);
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    printf("Array values at boundaries: a[0]=%f, a[%d]=%f\n", 
           a[0], N-1, a[N-1]);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
