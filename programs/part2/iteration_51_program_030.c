#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 10000

/* Helper function to prevent constant propagation */
static int get_iteration_count(int argc, char **argv) {
    volatile int count = N;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = N;
    }
    return count;
}

int main(int argc, char **argv) {
    int n = get_iteration_count(argc, argv);
    
    /* Allocate arrays with dynamic size to prevent static analysis */
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-uniform patterns */
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100) * 0.1;
        b[i] = (double)((i + 1) % 50) * 0.2;
        c[i] = 0.0;
        d[i] = (double)i * 0.05;
    }
    
    double checksum = 0.0;
    
    /* Outer parallel region - creates the necessary context */
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Runtime condition to control SIMD execution */
        volatile int use_simd = (n > 100) && (thread_id % 2 == 0);
        
        /* First loop: SIMD transformation candidate */
        if (use_simd) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {  /* Non-unit stride */
                /* Complex data-dependent computation */
                double temp = a[i] * b[i];
                c[i] = temp + d[i] * 0.5;
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] * b[i + 1] - d[i + 1];
                }
            }
        }
        
        /* Second loop: Different iteration pattern */
        if (n > 500) {
            #pragma omp for simd
            for (int i = 1; i < n - 1; i++) {  /* Different bounds */
                /* Another data-dependent computation */
                b[i] = (a[i - 1] + a[i] + a[i + 1]) / 3.0;
            }
        }
        
        /* Third loop: Standard OpenMP for (non-SIMD) for contrast */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            /* Simple computation */
            d[i] = a[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        /* Local reduction within parallel region */
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + d[i];
        }
    }
    
    /* Additional computation to prevent dead code elimination */
    double final_result = 0.0;
    for (int i = 0; i < n; i += 4) {  /* Strided access pattern */
        final_result += a[i] * b[i] - c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    /* Verify results aren't all zeros */
    if (checksum == 0.0 && final_result == 0.0) {
        printf("Warning: Results may have been optimized away\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
