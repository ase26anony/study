#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000

/* Volatile variable to prevent constant propagation */
volatile int use_simd_flag = 1;

/* Function with runtime-dependent condition */
void process_arrays(double *a, double *b, double *c, int N, int use_simd) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Runtime condition to decide SIMD usage */
        if (use_simd && N > 100) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd schedule(static) nowait
            for (int i = 0; i < N; i += 2) {  /* Non-unit stride */
                /* Data-dependent array accesses with computation */
                double temp = a[i] * 2.5 + thread_id * 0.1;
                c[i] = temp + b[i] * 3.7;
                
                /* Additional computation for odd indices */
                if (i + 1 < N) {
                    c[i + 1] = a[i + 1] * 1.8 - b[i + 1] * 0.9 + thread_id * 0.05;
                }
            }
            
            /* Second SIMD loop with different bounds */
            #pragma omp for simd schedule(dynamic, 16)
            for (int i = N/2; i < N; i++) {
                /* More complex data dependencies */
                a[i] = b[i] * c[i - N/2] + i * 0.01;
            }
        }
        
        /* Standard non-SIMD loop for contrast */
        #pragma omp for schedule(guided)
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 2.0 + i * 0.001;
        }
        
        /* Another SIMD loop with runtime-dependent upper bound */
        int limit = N - (thread_id % 10);
        #pragma omp for simd
        for (int i = 0; i < limit; i++) {
            /* Cross-array computation */
            c[i] = (a[i] + b[(i + 1) % N]) * 0.5;
        }
    }
}

/* Helper function with different loop pattern */
void process_with_offset(double *a, double *b, int N, int offset) {
    #pragma omp parallel
    {
        /* SIMD loop with variable bounds based on offset */
        #pragma omp for simd
        for (int i = offset; i < N - offset; i++) {
            a[i] = b[i] * (i - offset) * 0.1;
        }
    }
}

int main(int argc, char *argv[]) {
    int N = DEFAULT_N;
    
    /* Read N from command line to make it runtime-dependent */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = DEFAULT_N;
    }
    
    /* Allocate arrays with dynamic size */
    double *a = (double *)malloc(N * sizeof(double));
    double *b = (double *)malloc(N * sizeof(double));
    double *c = (double *)malloc(N * sizeof(double));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.0;
        c[i] = 0.0;
    }
    
    /* Make SIMD usage dependent on runtime value */
    int use_simd = (N % 3 != 0) ? use_simd_flag : 0;
    
    /* Process arrays - this should trigger the SIMT transformation */
    process_arrays(a, b, c, N, use_simd);
    
    /* Call another function with different parameters */
    process_with_offset(a, b, N, N/4);
    
    /* Final reduction to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Array size: %d, SIMD used: %d\n", N, use_simd);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
