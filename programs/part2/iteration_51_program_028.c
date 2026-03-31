#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10000

/* Helper function to make loop bounds runtime-dependent */
static int get_iteration_count(int base, int multiplier) {
    volatile int vol_mult = multiplier; /* Prevent constant propagation */
    return base * vol_mult;
}

int main(int argc, char *argv[]) {
    int n = N;
    
    /* Make iteration count runtime-dependent */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N;
    }
    
    /* Create arrays with data dependencies */
    double *a = (double *)malloc(n * sizeof(double));
    double *b = (double *)malloc(n * sizeof(double));
    double *c = (double *)malloc(n * sizeof(double));
    double *d = (double *)malloc(n * sizeof(double));
    
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
    volatile int use_simd = 1; /* Runtime condition variable */
    
    #pragma omp parallel reduction(+:checksum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Runtime-dependent condition to trigger SIMT transformation */
        int local_use_simd = use_simd;
        if (thread_id % 2 == 0) {
            local_use_simd = 1;
        } else {
            local_use_simd = (n > 100) ? 1 : 0;
        }
        
        /* First: SIMD loop with runtime condition */
        if (local_use_simd) {
            /* This should trigger the SIMT transformation */
            #pragma omp for simd nowait
            for (int i = 0; i < n; i++) {
                /* Complex data-dependent computation */
                double temp = a[i] * b[i];
                c[i] = temp + d[i] * (i % 10);
                
                /* Additional computation to create more complex GIMPLE */
                if (i % 2 == 0) {
                    c[i] += 1.0;
                } else {
                    c[i] -= 0.5;
                }
            }
        }
        
        /* Second: Non-SIMD loop for contrast */
        #pragma omp for schedule(static)
        for (int i = 0; i < n; i++) {
            /* Different operation pattern */
            a[i] = b[i] * 2.0 + (double)(i % 3);
            d[i] = a[i] / (b[i] + 1.0);
        }
        
        /* Third: Another SIMD loop with different stride */
        int step = get_iteration_count(1, 2); /* Runtime-dependent stride */
        #pragma omp for simd schedule(static)
        for (int i = 0; i < n; i += step) {
            /* Vectorizable reduction-like operation */
            if (i + step - 1 < n) {
                double sum = 0.0;
                for (int j = i; j < i + step && j < n; j++) {
                    sum += c[j];
                }
                b[i] = sum / step;
            }
        }
        
        /* Local reduction for checksum */
        #pragma omp for reduction(+:checksum)
        for (int i = 0; i < n; i++) {
            checksum += c[i] + a[i] + b[i] + d[i];
        }
    }
    
    /* Additional computation to prevent dead code elimination */
    double final_result = 0.0;
    for (int i = 0; i < n; i++) {
        final_result += c[i] * a[i] - b[i] / (d[i] + 1.0);
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Final result: %f\n", final_result);
    
    /* Verify results */
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (c[i] != c[i]) { /* Check for NaN */
            errors++;
        }
    }
    
    printf("Errors detected: %d\n", errors);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return errors > 0 ? 1 : 0;
}
