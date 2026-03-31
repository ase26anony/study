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
    
    /* Allocate arrays with runtime size */
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100) * 0.1;
        b[i] = (double)((i + 1) % 100) * 0.2;
        c[i] = 0.0;
        d[i] = (double)(i % 50) * 0.3;
    }
    
    double checksum = 0.0;
    
    /* Parallel region containing both SIMD and non-SIMD loops */
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Runtime condition to decide SIMD usage - makes the condition dynamic */
        int use_simd = (n > 100) && (thread_id % 2 == 0);
        
        /* First: SIMD loop with runtime condition */
        if (use_simd) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                /* Complex data-dependent computation */
                double temp = a[i] * b[i];
                c[i] = temp + d[i % (n/2 + 1)];
                /* Additional computation to make body non-trivial */
                if (c[i] > 100.0) {
                    c[i] = 100.0;
                }
            }
        }
        
        /* Second: Another SIMD loop with different stride */
        if (n > 500) {
            #pragma omp for simd schedule(static, 64)
            for (int i = 1; i < n - 1; i += 2) {
                /* Strided access pattern */
                b[i] = a[i-1] + a[i] + a[i+1];
            }
        }
        
        /* Third: Non-SIMD loop for contrast - regular OpenMP for */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            /* Different operation without SIMD */
            d[i] = a[i] * 2.0 + (double)thread_id * 0.01;
        }
        
        /* Fourth: SIMD loop with reduction */
        #pragma omp for simd reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] * d[i];
        }
        
        /* Fifth: Nested loops to create more complex control flow */
        if (num_threads > 1) {
            #pragma omp for simd collapse(2)
            for (int i = 0; i < n/2; i++) {
                for (int j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < n) {
                        a[idx] = b[idx] * (double)j;
                    }
                }
            }
        }
    }
    
    /* Final computation and output to prevent dead code elimination */
    double final_result = 0.0;
    for (int i = 0; i < n; i++) {
        final_result += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Final result: %f, Checksum: %f\n", final_result, checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
