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
    
    // Runtime-dependent condition for SIMD usage
    if (argc > 2) {
        use_simd = atoi(argv[2]) > 0;
    }
    
    // Allocate arrays with runtime size
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with non-trivial patterns
    for (int i = 0; i < n; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)(n - i) * 0.7;
        c[i] = 0.0;
        d[i] = (double)i * 2.3;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        // Runtime condition to decide SIMD usage - creates branching
        if (use_simd && n > 100) {
            // This should trigger the SIMT transformation
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                // Data-dependent array accesses with computation
                c[i] = a[i] + b[i] * (1.0 + (double)(i % 10) * 0.1);
            }
            
            // Another SIMD loop with different stride
            #pragma omp for simd schedule(static, 64)
            for (int i = 1; i < n-1; i += 2) {
                d[i] = a[i-1] + a[i] + a[i+1];
            }
        } else {
            // Fallback non-SIMD path
            #pragma omp for
            for (int i = 0; i < n; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        // Non-SIMD loop for contrast - standard OpenMP for
        #pragma omp for
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * (2.0 + (double)thread_id * 0.01);
        }
        
        // Reduction loop with SIMD - nested complexity
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] * d[i % (n/2 + 1)];
        }
    }
    
    // Additional computation to prevent dead code elimination
    double final_result = 0.0;
    for (int i = 0; i < n; i++) {
        final_result += a[i] + b[i] + c[i] + d[i];
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
