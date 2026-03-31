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
    
    // Runtime condition to control SIMD usage
    if (argc > 2) {
        use_simd = (atoi(argv[2]) > 0);
    }
    
    // Allocate arrays with dynamic size based on runtime input
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with pattern
    for (int i = 0; i < n; i++) {
        a[i] = (double)i;
        b[i] = (double)(n - i);
        c[i] = 0.0;
        d[i] = 1.0;
    }
    
    double checksum = 0.0;
    
    // Parallel region containing both SIMD and non-SIMD loops
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime-dependent condition for SIMD execution
        // This should trigger IFN_GOMP_USE_SIMT generation
        if (use_simd && n > 100) {
            // SIMD loop with non-unit stride in some cases
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                // Complex data-dependent computation
                double t = a[i] * b[i];
                c[i] = t + (double)(i % 8);
                // Cross-element dependency pattern
                if (i > 0) {
                    c[i] += d[i-1] * 0.1;
                }
                d[i] = c[i] * 0.5;
            }
        }
        
        // Additional SIMD loop with different pattern
        if (n > 500) {
            #pragma omp for simd schedule(static, 64)
            for (int i = 1; i < n-1; i++) {
                // Stencil-like computation
                a[i] = (b[i-1] + b[i] + b[i+1]) / 3.0;
            }
        }
        
        // Non-SIMD loop for contrast (standard OpenMP for)
        #pragma omp for
        for (int i = 0; i < n; i += 2) {
            // Non-vectorizable pattern
            b[i] = a[i] * thread_id;
            if (i + 1 < n) {
                b[i+1] = a[i+1] * (thread_id + 1);
            }
        }
        
        // Another SIMD loop with runtime-dependent bounds
        int start = thread_id * (n / num_threads);
        int end = (thread_id == num_threads - 1) ? n : (thread_id + 1) * (n / num_threads);
        
        if (end - start > 10) {
            #pragma omp for simd
            for (int i = start; i < end; i++) {
                c[i] += (double)(i % 16) * 0.25;
            }
        }
        
        // Reduction computation
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Final verification computation
    double final_sum = 0.0;
    for (int i = 0; i < n; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f, Final sum: %f\n", checksum, final_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
