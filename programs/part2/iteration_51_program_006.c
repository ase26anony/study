#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

/* Helper function to introduce runtime dependency */
static int get_iteration_count(int argc, char **argv) {
    if (argc > 1) {
        int n = atoi(argv[1]);
        return (n > 0) ? n : N;
    }
    /* Use volatile to prevent constant propagation */
    volatile int default_n = N;
    return default_n;
}

/* Function with complex data dependencies */
void process_arrays(double *a, double *b, double *c, double *d, int n, int use_simd) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Runtime condition to decide SIMD usage */
        if (use_simd && n > 100) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i += 2) {  /* Non-unit stride */
                /* Complex data-dependent computation */
                double temp = a[i] * b[i];
                c[i] = temp + (i % 8) * 0.1;
                d[i] = c[i] * thread_id * 0.01;
                
                /* Handle stride 2 */
                if (i + 1 < n) {
                    double temp2 = a[i+1] * b[i+1];
                    c[i+1] = temp2 - (i % 8) * 0.1;
                    d[i+1] = c[i+1] * thread_id * 0.01;
                }
            }
            
            /* Another SIMD loop with different bounds */
            #pragma omp for simd
            for (int i = n/2; i < n; i++) {
                /* Different computation pattern */
                a[i] = b[i] * 2.0 + c[i] * 0.5;
                b[i] = a[i] * d[i] * 0.25;
            }
        }
        
        /* Standard non-SIMD loop for contrast */
        #pragma omp for
        for (int i = 0; i < n; i += 3) {  /* Different stride */
            /* Different computation to avoid fusion */
            d[i] = a[i] + b[i] * thread_id;
            if (i + 1 < n) d[i+1] = a[i+1] - b[i+1] * thread_id;
            if (i + 2 < n) d[i+2] = a[i+2] * b[i+2] * thread_id;
        }
        
        /* Mixed loop with conditional SIMD */
        if (thread_id % 2 == 0) {
            #pragma omp for simd
            for (int i = 0; i < n/4; i++) {
                c[i] = d[i] * a[i] * 0.33;
            }
        }
    }
}

/* Another function with nested parallel regions */
void nested_simd_test(double *arr, int n) {
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for simd
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 2.0 + i * 0.001;
        }
        
        #pragma omp single
        {
            #pragma omp simd
            for (int i = 0; i < n/2; i++) {
                arr[i] = arr[i] * 1.5;
            }
        }
    }
}

int main(int argc, char **argv) {
    int n = get_iteration_count(argc, argv);
    int use_simd = (argc > 2) ? atoi(argv[2]) : 1;
    
    printf("Running with n=%d, use_simd=%d\n", n, use_simd);
    
    /* Allocate and initialize arrays */
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2;
        c[i] = i * 0.3;
        d[i] = i * 0.4;
    }
    
    /* Call the main processing function */
    process_arrays(a, b, c, d, n, use_simd);
    
    /* Call nested test */
    nested_simd_test(a, n);
    
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
