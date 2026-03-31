#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

/* Helper function to introduce runtime variability */
static int get_iteration_count(int argc, char **argv) {
    if (argc > 1) {
        int n = atoi(argv[1]);
        return (n > 0) ? n : N;
    }
    /* Use volatile to prevent constant propagation */
    volatile int dynamic_n = N;
    return dynamic_n;
}

/* Function with runtime-dependent SIMD usage */
void process_arrays(double *a, double *b, double *c, double *d, int n, int use_simd) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Runtime condition to decide SIMD usage - ensures IFN_GOMP_USE_SIMD generation */
        if (use_simd && n > 100) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {  /* Non-unit stride to stress transformation */
                /* Data-dependent array accesses with computation */
                double temp = a[i] * b[i];
                c[i] = temp + (double)thread_id;
                d[i] = temp - (double)thread_id;
                
                /* Additional computation for non-trivial loop body */
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] * 0.5 + b[i + 1] * 0.5;
                    d[i + 1] = a[i + 1] - b[i + 1];
                }
            }
        }
        
        /* Standard non-SIMD loop for contrast - creates multiple gomp_for statements */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        /* Another SIMD loop with different pattern */
        if (n > 500) {
            #pragma omp for simd
            for (int i = n/2; i < n; i++) {
                /* Complex data dependency chain */
                double val1 = a[i] * 3.14159;
                double val2 = b[i] * 2.71828;
                c[i] = val1 + val2;
                d[i] = val1 - val2;
                
                /* Cross-array dependency */
                if (i > n/2) {
                    a[i] += d[i-1] * 0.1;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    int n = get_iteration_count(argc, argv);
    
    /* Allocate and initialize arrays with non-trivial patterns */
    double *a = (double *)malloc(n * sizeof(double));
    double *b = (double *)malloc(n * sizeof(double));
    double *c = (double *)malloc(n * sizeof(double));
    double *d = (double *)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)(n - i) * 0.7;
        c[i] = 0.0;
        d[i] = 0.0;
    }
    
    /* Runtime-dependent SIMD usage flag */
    int use_simd_flag = (n % 3 == 0) ? 1 : 0;  /* Varies based on input */
    
    /* Process arrays with potential SIMD transformation */
    process_arrays(a, b, c, d, n, use_simd_flag);
    
    /* Final reduction to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array size: %d, SIMD flag: %d\n", n, use_simd_flag);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
