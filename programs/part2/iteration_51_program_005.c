#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_vol = N_DEFAULT;
    
    // Read N from command line or use default
    int N = (argc > 1) ? atoi(argv[1]) : N_vol;
    
    // Ensure N is positive and reasonable
    if (N <= 0) N = N_DEFAULT;
    
    // Allocate arrays with dynamic size to prevent static analysis
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
        a[i] = (double)(i % 100);
        b[i] = (double)((i + 1) % 100);
        c[i] = 0.0;
        d[i] = (double)(i * 2);
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage - varies per thread
        int use_simd = (N > 100) && (thread_id % 2 == 0);
        
        // FIRST: Standard parallel for loop (non-SIMD)
        // This creates a gomp_for statement for scan_omp_for
        #pragma omp for schedule(static)
        for (int i = 0; i < N; i++) {
            // Data-dependent array access with non-trivial computation
            int idx = (i * 7) % N;  // Non-linear index
            a[idx] = b[i] * 2.0 + (double)thread_id;
        }
        
        // SECOND: SIMD loop with runtime condition
        // This should trigger the SIMT transformation when use_simd is true
        if (use_simd) {
            // Loop with non-unit stride to increase complexity
            #pragma omp for simd schedule(static) simdlen(4)
            for (int i = 0; i < N; i += 1) {
                // Complex data-dependent computation
                double temp = a[i] + b[(i + 3) % N];
                c[i] = temp * d[i] - (double)(i % 10);
                
                // Additional conditional inside SIMD loop
                if (c[i] > 100.0) {
                    c[i] = 100.0;
                }
            }
        } else {
            // Fallback non-SIMD version
            #pragma omp for schedule(static)
            for (int i = 0; i < N; i += 1) {
                double temp = a[i] + b[(i + 3) % N];
                c[i] = temp * d[i] - (double)(i % 10);
                if (c[i] > 100.0) {
                    c[i] = 100.0;
                }
            }
        }
        
        // THIRD: Another SIMD loop with different bounds
        // Multiple SIMD loops increase chance of hitting transformation
        int start = thread_id * (N / omp_get_num_threads());
        int end = (thread_id + 1) * (N / omp_get_num_threads());
        
        if (end > N) end = N;
        
        // Nested loop structure
        #pragma omp for simd schedule(dynamic, 16) nowait
        for (int i = start; i < end; i++) {
            // Cross-array computation
            for (int j = 0; j < 4; j++) {
                int idx = (i + j) % N;
                d[idx] = c[i] * 0.5 + a[idx] * 0.3;
            }
        }
        
        // Local reduction for checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Final computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * 0.1 + b[i] * 0.2 + c[i] * 0.3 + d[i] * 0.4;
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    printf("N: %d\n", N);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
