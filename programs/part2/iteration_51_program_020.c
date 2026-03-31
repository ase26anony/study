#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

/* Helper function to make loop bounds runtime-dependent */
int get_iteration_count(int base, int multiplier) {
    volatile int result = base * multiplier; /* volatile prevents constant propagation */
    return result;
}

/* Function with nested parallel and SIMD regions */
void process_arrays(double *a, double *b, double *c, int n, int use_simd) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Runtime condition to decide whether to use SIMD */
        if (use_simd && n > 100) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) { /* Non-unit stride to add complexity */
                /* Data-dependent array accesses with computation */
                double temp = a[i] * 2.5 + b[i] * 1.5;
                c[i] = temp + (double)thread_id * 0.01;
                if (i + 1 < n) {
                    c[i + 1] = a[i + 1] * 3.0 - b[i + 1] * 0.5 + (double)thread_id * 0.02;
                }
            }
            
            /* Second SIMD loop with different pattern */
            #pragma omp for simd
            for (int i = n/2; i < n; i++) {
                /* Cross-array dependencies */
                a[i] = b[i] * c[i - n/2] + (double)i * 0.001;
            }
        }
        
        /* Standard non-SIMD loop for contrast */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2.0 + (double)thread_id * 0.001;
        }
        
        /* Another SIMD loop with runtime-dependent bounds */
        int dynamic_bound = get_iteration_count(n, thread_id + 1) % n;
        if (dynamic_bound > 0) {
            #pragma omp for simd
            for (int i = 0; i < dynamic_bound; i++) {
                c[i] += a[i] * b[i] * 0.5;
            }
        }
    }
}

/* Another function with different SIMD pattern */
void process_arrays_2d(double *a, double *b, double *c, int n, int m) {
    #pragma omp parallel
    {
        /* Nested loops with SIMD on inner loop */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp simd
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                a[idx] = b[idx] * 1.5 + c[idx] * 0.5;
                if (j % 3 == 0) {
                    b[idx] = a[idx] * 2.0;
                }
            }
        }
        
        /* SIMD loop with reduction */
        double local_sum = 0.0;
        #pragma omp for simd reduction(+:local_sum)
        for (int i = 0; i < n * m; i++) {
            local_sum += a[i] + b[i];
            c[i] = local_sum * 0.01;
        }
    }
}

int main(int argc, char *argv[]) {
    int n = N;
    
    /* Make size runtime-dependent from command line */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    /* Use volatile to prevent constant propagation */
    volatile int use_simd_flag = 1;
    if (argc > 2) {
        use_simd_flag = atoi(argv[2]);
    }
    
    /* Allocate and initialize arrays */
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        a[i] = (double)i * 0.1;
        b[i] = (double)(n - i) * 0.2;
        c[i] = 0.0;
    }
    
    /* Process arrays with SIMD transformation opportunity */
    process_arrays(a, b, c, n, use_simd_flag);
    
    /* Process with 2D pattern */
    int m = (n > 100) ? 100 : n;
    process_arrays_2d(a, b, c, n/m, m);
    
    /* Final reduction to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
