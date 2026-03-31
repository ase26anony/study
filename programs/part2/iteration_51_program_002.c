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

/* Function with complex data dependencies */
void process_arrays(double *a, double *b, double *c, double *d, int n, int use_simd) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Runtime condition to control SIMD usage */
        if (use_simd && n > 100) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {  /* Non-unit stride */
                /* Complex data-dependent computation */
                double t = a[i] * b[i];
                c[i] = t + (i % 8) * 0.1;
                d[i] = t - (i % 8) * 0.1;
                
                /* Handle odd indices with different pattern */
                if (i + 1 < n) {
                    double t2 = a[i+1] * b[i+1] * 0.5;
                    c[i+1] = t2 + ((i+1) % 8) * 0.05;
                    d[i+1] = t2 - ((i+1) % 8) * 0.05;
                }
            }
            
            /* Another SIMD loop with different bounds */
            if (n > 500) {
                #pragma omp for simd
                for (int i = n/2; i < n; i++) {
                    /* Different computation pattern */
                    c[i] = (a[i] + b[i]) * (thread_id + 1) * 0.01;
                    d[i] = (a[i] - b[i]) * (thread_id + 1) * 0.01;
                }
            }
        }
        
        /* Standard non-SIMD loop for contrast */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * (2.0 + thread_id * 0.1);
        }
        
        /* Mixed loop with conditional SIMD */
        if (n % 2 == 0) {
            #pragma omp for simd
            for (int i = 0; i < n/2; i++) {
                b[i] = c[i] + d[n-i-1];
            }
        }
    }
}

int main(int argc, char **argv) {
    int n = get_iteration_count(argc, argv);
    
    /* Allocate arrays with alignment hint for SIMD */
    double *a = (double*)aligned_alloc(64, n * sizeof(double));
    double *b = (double*)aligned_alloc(64, n * sizeof(double));
    double *c = (double*)aligned_alloc(64, n * sizeof(double));
    double *d = (double*)aligned_alloc(64, n * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.1;
        b[i] = (n - i) * 0.05;
        c[i] = 0.0;
        d[i] = 0.0;
    }
    
    /* Runtime condition for SIMD usage */
    int use_simd = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Process arrays with potential SIMT transformation */
    process_arrays(a, b, c, d, n, use_simd);
    
    /* Final reduction to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
