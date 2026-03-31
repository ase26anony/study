#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

/* Volatile variable to prevent constant propagation */
volatile int use_simd_flag = 1;

void process_arrays(double *a, double *b, double *c, int n, int use_simd) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Runtime condition to decide SIMD usage */
        if (use_simd && n > 100) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {  /* Non-unit stride */
                /* Data-dependent array accesses with computation */
                double temp = a[i] * 2.5 + b[i] * 1.5;
                c[i] = temp + (double)i * 0.01;
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] * 3.0 - b[i + 1] * 0.5 + (double)(i + 1) * 0.02;
                }
            }
            
            /* Another SIMD loop with different bounds */
            #pragma omp for simd
            for (int i = n/4; i < 3*n/4; i++) {
                b[i] = a[i] * c[i] * 0.5 + (double)thread_id * 0.001;
            }
        }
        
        /* Standard non-SIMD loop for contrast */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * 2.0 + (double)thread_id * 0.0001;
        }
        
        /* SIMD loop with runtime-dependent upper bound */
        int upper_bound = n - (thread_id % 10);
        #pragma omp for simd
        for (int i = 0; i < upper_bound; i++) {
            double factor = (i % 3 == 0) ? 1.2 : 0.8;
            c[i] = c[i] * factor + a[i] * 0.3;
        }
    }
}

int main(int argc, char *argv[]) {
    int n = N;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    /* Allocate and initialize arrays */
    double *a = (double *)malloc(n * sizeof(double));
    double *b = (double *)malloc(n * sizeof(double));
    double *c = (double *)malloc(n * sizeof(double));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        a[i] = (double)i * 0.1;
        b[i] = (double)(n - i) * 0.05;
        c[i] = 0.0;
    }
    
    /* Process arrays with SIMD condition based on runtime flag */
    process_arrays(a, b, c, n, use_simd_flag);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Additional verification */
    double max_val = c[0];
    for (int i = 1; i < n; i++) {
        if (c[i] > max_val) max_val = c[i];
    }
    printf("Maximum value in c: %f\n", max_val);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
