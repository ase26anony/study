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
        a[i] = i * 1.5;
        b[i] = i * 2.0;
        c[i] = 0.0;
        d[i] = 0.0;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage
        // This should generate IFN_GOMP_USE_SIMT call
        if (use_simd && N > 100) {
            // First SIMD loop with non-unit stride
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {
                // Data-dependent array access with computation
                int idx = (i * 17) % N;  // Non-linear index
                c[idx] = a[i] + b[i] * (thread_id + 1);
            }
            
            // Second SIMD loop with different bounds
            #pragma omp for simd
            for (int i = 1; i < N - 10; i++) {
                // Complex data dependency
                d[i] = a[i-1] * 0.5 + b[i+1] * 1.5 + c[(i * 3) % N];
            }
        } else {
            // Fallback non-SIMD path
            #pragma omp for
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Standard non-SIMD loop for contrast
        // This creates another gomp_for statement
        #pragma omp for
        for (int i = 0; i < N; i++) {
            a[i] = b[i] * 2.0 + thread_id * 0.01;
        }
        
        // Reduction for checksum
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    // Additional computation to prevent optimization
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * 0.1 + c[i] * 0.2;
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    printf("Array c[0]=%f, c[%d]=%f\n", c[0], N-1, c[N-1]);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
