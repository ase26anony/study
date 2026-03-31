#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile;
    
    if (argc > 1) {
        N_volatile = atoi(argv[1]);
    } else {
        N_volatile = DEFAULT_N;
    }
    
    // Non-volatile copy for array sizing
    int N = N_volatile;
    
    if (N <= 0) {
        fprintf(stderr, "N must be positive\n");
        return 1;
    }
    
    // Allocate arrays with runtime-dependent size
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
        int num_threads = omp_get_num_threads();
        
        // Runtime condition to decide SIMD usage - depends on input
        int use_simd = (N > 100) && (thread_id % 2 == 0);
        
        // First loop: SIMD with runtime condition
        if (use_simd) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i++) {
                // Data-dependent computation with array accesses
                c[i] = a[i] + b[i] * (d[i % (N/2 + 1)] + 1.0);
            }
        }
        
        // Second loop: Non-SIMD for contrast
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different stride pattern
            int idx = (i * 3) % N;
            a[idx] = b[i] * 2.0 + (double)thread_id * 0.1;
        }
        
        // Third loop: Another SIMD loop with different bounds
        // Using runtime-dependent loop bound
        int limit = N - (thread_id * 10);
        if (limit > 0) {
            #pragma omp for simd
            for (int i = 0; i < limit; i++) {
                // Complex data dependency
                double temp = d[i] * 0.5;
                c[i] += temp * (a[(i + 1) % N] - b[(i + N - 1) % N]);
            }
        }
        
        // Fourth loop: SIMD with non-unit stride
        #pragma omp for simd
        for (int i = 0; i < N; i += 2) {
            d[i] = c[i] * a[i] - b[i];
        }
        
        // Calculate partial checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i % (N/3 + 1)];
        }
    }
    
    // Additional sequential computation to prevent optimization
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * 0.01 + c[i] * 0.02;
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    printf("Array samples: a[0]=%f, c[N-1]=%f\n", a[0], c[N-1]);
    
    // Free allocated memory
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
