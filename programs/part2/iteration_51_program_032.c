#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile = N_DEFAULT;
    int N = N_volatile;
    
    // Read N from command line if provided
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = N_DEFAULT;
    }
    
    // Declare arrays with runtime size
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    double *d = (double*)malloc(N * sizeof(double));
    
    // Initialize arrays
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)(N - i) * 0.7;
        c[i] = 0.0;
        d[i] = (double)i * 2.3;
    }
    
    double checksum = 0.0;
    
    // Outer parallel region - creates the outer_ctx needed for SIMT transformation
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime-dependent condition to decide whether to use SIMD
        // This should trigger IFN_GOMP_USE_SIMT generation
        int use_simd = (N > 100) && (thread_id % 2 == 0);
        
        // First: SIMD loop with runtime condition
        // This should trigger the SIMT transformation when use_simd is true
        if (use_simd) {
            // #pragma omp for simd with non-unit stride to increase complexity
            #pragma omp for simd schedule(static) nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array accesses with computation
                int idx = (i * 13) % N;  // Non-linear index
                c[idx] = a[i] * b[i] + d[(i + 1) % N];
                
                // Additional computation to make loop body substantial
                double temp = a[i] - b[i];
                if (temp > 0) {
                    d[i] = temp * 0.5;
                } else {
                    d[i] = temp * (-0.5);
                }
            }
        }
        
        // Second: Non-SIMD loop for contrast
        // This creates another gomp_for statement for scan_omp_for
        #pragma omp for schedule(dynamic, 16)
        for (int i = 1; i < N; i += 3) {
            // Different operation pattern
            a[i] = b[i] * 2.0 + (double)thread_id * 0.01;
            
            // Nested condition to create more complex control flow
            if (i % 7 == 0) {
                c[i] = a[i] * 0.3;
            } else {
                c[i] = b[i] * 0.7;
            }
        }
        
        // Third: Another SIMD loop with different bounds
        // Multiple SIMD loops increase chance of hitting transformation
        int start = (thread_id * N) / num_threads;
        int end = ((thread_id + 1) * N) / num_threads;
        
        if (end - start > 50) {
            #pragma omp for simd schedule(guided) nowait
            for (int i = start; i < end; i++) {
                // Complex computation with multiple array accesses
                double val1 = a[i] * 0.25;
                double val2 = b[(i + 5) % N] * 0.75;
                double val3 = c[(i * 2) % N] * 0.5;
                
                d[i] = val1 + val2 - val3;
                
                // Conditional store
                if (d[i] > 100.0) {
                    a[i] = d[i] * 0.1;
                }
            }
        }
        
        // Local reduction for checksum
        double local_sum = 0.0;
        #pragma omp for reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += c[i] + d[i];
        }
        checksum += local_sum;
    }
    
    // Final verification computation to prevent dead code elimination
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %.6f\n", checksum);
    printf("Final sum: %.6f\n", final_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
