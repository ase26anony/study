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
    return dynamic_n + (omp_get_thread_num() % 2); /* Small variation per thread */
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
    
    /* Initialize arrays with non-uniform patterns */
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i % 100) * 0.1;
        b[i] = (double)((i + 1) % 50) * 0.2;
        c[i] = 0.0;
        d[i] = (double)i * 0.05;
    }
    
    double checksum = 0.0;
    
    /* Outer parallel region - creates the context needed for SIMT transformation */
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Runtime condition to decide SIMD usage - ensures IFN_GOMP_USE_SIMT generation */
        int use_simd = (n > 100) && (thread_id % 2 == 0);
        
        /* First: SIMD loop with runtime condition */
        if (use_simd) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                /* Complex index calculation to prevent optimization */
                int idx = (i * 7 + thread_id) % n;
                /* Data-dependent computation with multiple arrays */
                c[idx] = a[idx] * 2.5 + b[(idx + 1) % n] * 1.5;
                /* Additional computation to create non-trivial loop body */
                d[idx] = c[idx] / (a[idx] + 1.0);
            }
        }
        
        /* Second: Non-SIMD loop for contrast - creates another gomp_for statement */
        #pragma omp for schedule(static)
        for (int i = 0; i < n; i += 2) { /* Non-unit stride */
            /* Different operation pattern */
            a[i] = b[i] * 3.0 + d[(i + thread_id) % n];
            if (i + 1 < n) {
                a[i + 1] = b[i + 1] * 2.0 - d[(i + thread_id + 1) % n];
            }
        }
        
        /* Third: Another SIMD loop with different bounds */
        int start = thread_id * (n / num_threads);
        int end = (thread_id + 1) * (n / num_threads);
        
        #pragma omp for simd
        for (int i = start; i < end; i++) {
            /* Vectorizable reduction-like operation */
            double temp = a[i] + b[i] * c[i % 100];
            checksum += temp;
            d[i] = temp * 0.5;
        }
        
        /* Fourth: Mixed loop with conditional SIMD */
        int simd_threshold = 500;
        if (n > simd_threshold) {
            #pragma omp for simd
            for (int i = 0; i < n; i++) {
                /* Cross-array dependency */
                b[i] = a[(i + n/2) % n] * c[i] + d[i];
            }
        } else {
            #pragma omp for
            for (int i = 0; i < n; i++) {
                b[i] = a[i] * 2.0;
            }
        }
    }
    
    /* Final verification computation to prevent dead code elimination */
    double final_sum = 0.0;
    for (int i = 0; i < n; i++) {
        final_sum += c[i] + d[i];
    }
    
    printf("Checksum: %f, Final sum: %f\n", checksum, final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
