#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1000

int main(int argc, char *argv[]) {
    // Use volatile to prevent constant propagation
    volatile int N_volatile = N_DEFAULT;
    
    // Read N from command line or use default
    int N = (argc > 1) ? atoi(argv[1]) : N_volatile;
    
    // Ensure N is positive and reasonable
    if (N <= 0) N = N_DEFAULT;
    
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
        a[i] = (double)(i % 100);
        b[i] = (double)((i + 1) % 100);
        c[i] = 0.0;
        d[i] = (double)(i * 2);
    }
    
    // Runtime condition that will be evaluated inside parallel region
    int use_simd = 0;
    if (N > 100) {
        use_simd = 1;
    }
    
    double parallel_sum = 0.0;
    
    #pragma omp parallel reduction(+:parallel_sum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime-dependent condition inside parallel region
        // This creates the branching needed for IFN_GOMP_USE_SIMT
        if (use_simd && (thread_id % 2 == 0)) {
            // This should trigger SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {  // Non-unit stride
                // Data-dependent array access with computation
                c[i] = a[i] * 2.5 + b[i] * 1.5;
                // Additional computation to make body non-trivial
                if (i + 1 < N) {
                    c[i + 1] = a[i] - b[i] + d[i % N];
                }
            }
            
            // Another SIMD loop with different pattern
            #pragma omp for simd schedule(static, 32)
            for (int i = N/2; i < N; i++) {
                // Complex data dependency
                int idx = (i * 3) % N;
                a[i] = b[idx] * c[i % (N/2 + 1)] + d[i];
            }
        }
        
        // Standard non-SIMD loop for contrast
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Different operation pattern
            b[i] = a[i] * 3.0 + (double)(i % 10);
            parallel_sum += b[i];
        }
        
        // Conditional SIMD loop based on thread-local computation
        int local_N = N / (omp_get_num_threads() + 1);
        if (local_N > 50) {
            #pragma omp for simd
            for (int i = 0; i < local_N; i++) {
                d[i] = a[i] + b[N - i - 1] * 0.5;
            }
        }
    }
    
    // Final reduction and checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Parallel sum: %f\n", parallel_sum);
    
    // Verify results with simple computation
    double verify = 0.0;
    #pragma omp parallel for reduction(+:verify)
    for (int i = 0; i < N; i++) {
        verify += a[i] * b[i];
    }
    printf("Verification: %f\n", verify);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
