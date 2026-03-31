#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile;
    
    // Read N from command line or use default
    if (argc > 1) {
        N_volatile = atoi(argv[1]);
    } else {
        N_volatile = DEFAULT_N;
    }
    
    // Convert to non-volatile for array sizing
    int N = N_volatile;
    
    // Allocate and initialize arrays
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
        a[i] = i * 1.5;
        b[i] = i * 2.0;
        c[i] = 0.0;
        d[i] = i * 0.5;
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime-dependent condition for SIMD execution
        int use_simd = (N > 100) && (thread_id % 2 == 0);
        
        // First: SIMD loop with runtime condition
        if (use_simd) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {  // Non-unit stride
                // Data-dependent array accesses
                int idx = (i * 3) % N;
                c[i] = a[idx] + b[(i + 1) % N] * d[i];
            }
        }
        
        // Second: Another SIMD loop with different pattern
        #pragma omp for simd
        for (int i = 1; i < N - 1; i++) {  // Different bounds
            // Complex data dependency
            a[i] = b[i-1] * 0.3 + b[i] * 0.4 + b[i+1] * 0.3;
        }
        
        // Third: Non-SIMD loop for contrast
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Simple operation without SIMD
            d[i] = c[i] * 2.0 - a[i];
        }
        
        // Fourth: SIMD loop with reduction
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
        
        // Fifth: Conditional SIMD loop inside another if
        if (N > 500) {
            #pragma omp for simd
            for (int i = 0; i < N/2; i++) {
                int j = N - i - 1;
                double temp = a[i];
                a[i] = b[j];
                b[j] = temp;
            }
        }
    }
    
    // Additional sequential computation to prevent optimization
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum from parallel region: %f\n", checksum);
    printf("Final sum from sequential: %f\n", final_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
