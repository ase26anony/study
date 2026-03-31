#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

/* Helper function to make loop bounds runtime-dependent */
static int get_iteration_count(int base, int multiplier) {
    volatile int result = base * multiplier; /* volatile prevents constant propagation */
    return result;
}

int main(int argc, char *argv[]) {
    int i;
    double checksum = 0.0;
    
    /* Use command-line argument or default to make it runtime-dependent */
    int n = N;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    /* Allocate and initialize arrays */
    double *a = (double *)malloc(n * sizeof(double));
    double *b = (double *)malloc(n * sizeof(double));
    double *c = (double *)malloc(n * sizeof(double));
    double *d = (double *)malloc(n * sizeof(double));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (i = 0; i < n; i++) {
        a[i] = (double)i * 0.5;
        b[i] = (double)(n - i) * 0.3;
        c[i] = 0.0;
        d[i] = (double)i * 2.0;
    }
    
    /* Runtime condition that affects SIMD usage */
    int use_simd = (n > 100);  /* This creates the condition for IFN_GOMP_USE_SIMT */
    
    /* Parallel region containing both SIMD and non-SIMD loops */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Runtime-dependent loop bound to prevent optimization */
        int local_n = get_iteration_count(n, thread_id + 1) / (thread_id + 1);
        if (local_n > n) local_n = n;
        
        /* First: Standard non-SIMD loop */
        #pragma omp for schedule(static) nowait
        for (i = 0; i < n; i += 2) {  /* Non-unit stride */
            /* Data-dependent array access pattern */
            int idx = (i * 17) % n;  /* Non-linear index calculation */
            a[idx] = b[i] * d[idx] + (double)thread_id;
        }
        
        /* Second: SIMD loop with runtime condition */
        if (use_simd) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd schedule(static) nowait
            for (i = 0; i < local_n; i++) {
                /* Vectorizable operation with data dependencies */
                double temp = a[i] * 2.0 + b[i];
                c[i] = temp * d[i] - a[(i + 1) % n];
                
                /* Additional computation to make body non-trivial */
                if (c[i] < 0.0) {
                    c[i] = -c[i];
                }
            }
        } else {
            /* Fallback non-SIMD version */
            #pragma omp for schedule(static) nowait
            for (i = 0; i < local_n; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        /* Third: Another SIMD loop with different pattern */
        int start = thread_id * (n / num_threads);
        int end = (thread_id + 1) * (n / num_threads);
        if (thread_id == num_threads - 1) end = n;
        
        #pragma omp for simd schedule(dynamic, 16) nowait
        for (i = start; i < end; i += 1) {
            /* Complex data-dependent computation */
            b[i] = c[(i + n/2) % n] * 3.14159 - a[i];
            d[i] = (b[i] > 0.0) ? b[i] * 0.5 : b[i] * 2.0;
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    for (i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Array values at boundaries: a[0]=%f, a[%d]=%f\n", a[0], n-1, a[n-1]);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
