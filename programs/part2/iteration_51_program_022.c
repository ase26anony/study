#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int n = N;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    // Declare arrays with runtime size
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100);
        b[i] = (double)((i * 3) % 97);
        c[i] = 0.0;
        d[i] = (double)(i % 50);
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime condition to decide SIMD usage
        int use_simd = (n > 100) && (thread_id % 2 == 0);
        
        // First loop: SIMD transformation candidate
        if (use_simd) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                // Complex data-dependent computation
                double temp = a[i] * b[i];
                c[i] = temp + d[i] * 2.5;
                // Additional computation to create dependencies
                if (i > 0) {
                    c[i] += a[i-1] * 0.1;
                }
            }
        }
        
        // Second loop: Non-SIMD for contrast
        #pragma omp for schedule(static)
        for (int i = 0; i < n; i += 2) {  // Non-unit stride
            // Different operation pattern
            a[i] = b[i] * 3.14159;
            if (i + 1 < n) {
                a[i+1] = b[i] * 2.71828;
            }
        }
        
        // Third loop: Another SIMD loop with different bounds
        int local_n = n / 2;
        if (local_n > 0) {
            #pragma omp for simd
            for (int i = local_n; i < n; i++) {
                // More data-dependent computation
                d[i] = c[i] * a[i] - b[i];
                // Cross-array dependency
                if (i > local_n) {
                    d[i] += d[i-1] * 0.01;
                }
            }
        }
        
        // Calculate partial checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional sequential computation to prevent optimization
    double final_result = 0.0;
    for (int i = 0; i < n; i++) {
        final_result += a[i] * 0.5 + b[i] * 0.3;
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
