#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

/* Helper function to create runtime-dependent condition */
static int should_use_simd(int n) {
    volatile int threshold = 500; /* volatile prevents constant propagation */
    return n > threshold;
}

int main(int argc, char *argv[]) {
    int n = N;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    /* Create arrays with data dependencies */
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    /* Initialize arrays with non-uniform patterns */
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100) * 0.5;
        b[i] = (double)(i % 50) * 1.5;
        d[i] = (double)(i % 75) * 0.8;
    }
    
    double checksum = 0.0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        
        /* Runtime-dependent condition for SIMD execution */
        int use_simd = should_use_simd(n);
        
        /* First loop: SIMD with runtime condition */
        if (use_simd) {
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                /* Complex data-dependent computation */
                c[i] = a[i] + b[i] * (1.0 + (double)(i % 10) * 0.1);
                
                /* Additional computation with array dependencies */
                if (i > 0) {
                    c[i] += d[i-1] * 0.3;
                }
            }
        }
        
        /* Second loop: Non-SIMD for contrast */
        #pragma omp for schedule(static)
        for (int i = 0; i < n; i++) {
            /* Different operation without SIMD */
            a[i] = b[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        /* Third loop: Another SIMD loop with different stride */
        if (use_simd && (n > 1000)) {
            #pragma omp for simd schedule(static) nowait
            for (int i = 2; i < n-2; i += 2) {
                /* Strided access pattern */
                d[i] = c[i-1] + c[i+1] * 0.5;
            }
        }
        
        /* Calculate partial checksum */
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + a[i] + d[i];
        }
    }
    
    /* Additional computation to prevent dead code elimination */
    double final_result = 0.0;
    for (int i = 0; i < n; i += 100) {
        final_result += c[i] * a[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    /* Verify results */
    if (checksum != 0.0) {
        printf("Computation completed successfully.\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
