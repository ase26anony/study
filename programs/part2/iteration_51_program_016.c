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
        a[i] = (double)i;
        b[i] = (double)(N - i);
        c[i] = 0.0;
        d[i] = 1.0;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Runtime-dependent condition - varies by thread
        int use_simd = (thread_id % 2 == 0) && (N > 100);
        
        // First: SIMD loop with runtime condition
        if (use_simd) {
            // This should trigger SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < N; i += 2) {  // Non-unit stride
                // Data-dependent array accesses
                int idx = (i * 17) % N;  // Non-linear index
                c[i] = a[idx] + b[(i + 1) % N] * d[i];
            }
            
            // Another SIMD loop with different bounds
            #pragma omp for simd
            for (int i = N/4; i < 3*N/4; i++) {
                // Complex data dependency
                double temp = (i % 3 == 0) ? 2.0 : 1.0;
                d[i] = a[i] * temp - b[N - i - 1];
            }
        }
        
        // Second: Non-SIMD loop for contrast
        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Simple operation without SIMD
            a[i] = b[i] * (double)(i + 1);
        }
        
        // Third: Mixed SIMD/non-SIMD with conditional
        int block_size = N / (num_threads * 4);
        if (block_size < 1) block_size = 1;
        
        #pragma omp for
        for (int i = 0; i < N; i += block_size) {
            int end = i + block_size;
            if (end > N) end = N;
            
            // Inner loop that could be SIMD
            if (use_simd && (end - i) >= 4) {
                #pragma omp simd
                for (int j = i; j < end; j++) {
                    c[j] += a[j] * 0.5;
                }
            } else {
                for (int j = i; j < end; j++) {
                    b[j] = c[j] * 2.0;
                }
            }
        }
        
        // Local reduction
        double local_sum = 0.0;
        #pragma omp for reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += c[i] + d[i];
        }
        checksum += local_sum;
    }
    
    // Final computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] * b[i] - c[i] / d[i];
    }
    
    printf("Checksum: %f, Final: %f\n", checksum, final_result);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
