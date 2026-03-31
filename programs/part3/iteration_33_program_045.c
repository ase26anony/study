/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_bound = 100;
volatile int g_seed = 42;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(array_a, array_b, array_c)
void vec_add(int n, double *a, double *b, double *c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(int n, double *data, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) safelen(8) \
            scan(inscan:sum)
    for (int i = 0; i < n; i++) {
        double val = data[i];
        
        /* Inscan phase */
        #pragma omp scan inclusive(sum)
        {
            sum += val;
            prefix[i] = sum;
        }
        
        /* Additional computation to prevent optimization */
        if (i % 3 == 0) {
            prefix[i] *= 1.1;
        }
    }
    
    return sum;
}

/* Function with nested loops and collapse - may generate _condtemp_ */
void nested_collapse_loops(int m, int n, double *matrix, double factor) {
    int i, j;
    
    /* Use volatile variable in loop bound to prevent optimization */
    int bound = g_volatile_bound;
    if (bound > m) bound = m;
    if (bound > n) bound = n;
    
    /* Collapsed loop with non-trivial bound - may create condition temporaries */
    #pragma omp parallel for collapse(2) private(i, j) \
            firstprivate(factor) \
            schedule(dynamic, 4)
    for (i = 0; i < bound; i++) {
        for (j = 0; j < bound; j++) {
            int idx = i * n + j;
            
            /* Data-dependent computation */
            if ((i + j) % 7 == 0) {
                matrix[idx] = factor * (i + j);
            } else if (omp_get_thread_num() % 2 == 0) {
                matrix[idx] = factor * (i - j);
            } else {
                matrix[idx] = factor * (i * j);
            }
            
            /* Prevent dead code elimination */
            if (matrix[idx] < 0) {
                matrix[idx] = -matrix[idx];
            }
        }
    }
}

/* Complex reduction with multiple temporaries */
double complex_reduction(int n, double *arr1, double *arr2) {
    double total = 0.0;
    double partial_sum = 0.0;
    
    /* Parallel region with reduction - should generate _reductemp_ */
    #pragma omp parallel reduction(+:total) private(partial_sum)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        int chunk = n / nthreads;
        int start = tid * chunk;
        int end = (tid == nthreads - 1) ? n : start + chunk;
        
        /* Nested loop inside parallel region */
        #pragma omp for simd nowait reduction(+:partial_sum)
        for (int i = 0; i < n; i++) {
            double temp = arr1[i] * arr2[i];
            
            /* Conditional operation based on thread ID */
            if (tid % 3 == 0) {
                temp *= 1.5;
            } else if (tid % 3 == 1) {
                temp *= 0.8;
            }
            
            partial_sum += temp;
            
            /* Additional scan-like operation */
            if (i > 0 && i % 100 == 0) {
                #pragma omp atomic
                total += partial_sum;
                partial_sum = 0.0;
            }
        }
        
        #pragma omp atomic
        total += partial_sum;
    }
    
    return total;
}

int main(int argc, char **argv) {
    /* Use argc to create variable but reproducible sizes */
    int base_size = 1000;
    if (argc > 1) {
        base_size = atoi(argv[1]);
        if (base_size < 100) base_size = 100;
        if (base_size > 10000) base_size = 10000;
    }
    
    int n = base_size + (argc * 37) % 100;  /* Variable size */
    int m = (n / 10) + 5;
    
    printf("Testing OpenMP coverage with n=%d, m=%d\n", n, m);
    
    /* Allocate arrays with variable sizes */
    double *array_a = (double *)malloc(n * sizeof(double));
    double *array_b = (double *)malloc(n * sizeof(double));
    double *array_c = (double *)malloc(n * sizeof(double));
    double *prefix = (double *)malloc(n * sizeof(double));
    double *matrix = (double *)calloc(m * m, sizeof(double));
    
    if (!array_a || !array_b || !array_c || !prefix || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with simple patterns */
    for (int i = 0; i < n; i++) {
        array_a[i] = i * 1.5;
        array_b[i] = i * 0.7;
        array_c[i] = 0.0;
        prefix[i] = 0.0;
    }
    
    /* Test 1: Prefix sum with reduction and scan */
    printf("Test 1: Prefix sum with reduction and scan\n");
    double sum1 = compute_prefix_sum(n, array_a, prefix);
    printf("  Prefix sum total: %f\n", sum1);
    
    /* Test 2: Nested collapse loops */
    printf("Test 2: Nested collapse loops\n");
    nested_collapse_loops(m, m, matrix, 2.5);
    
    /* Test 3: Complex reduction */
    printf("Test 3: Complex reduction\n");
    double sum2 = complex_reduction(n, array_a, array_b);
    printf("  Complex reduction result: %f\n", sum2);
    
    /* Test 4: Declare target enter with to clause */
    printf("Test 4: Declare target enter\n");
    
    /* Update volatile seed */
    g_seed = argc * 12345;
    
    /* Target region using the entered function */
    #pragma omp target map(tofrom: array_c[0:n]) \
                       map(to: array_a[0:n], array_b[0:n]) \
                       device(0) if(n > 500)
    {
        vec_add(n, array_a, array_b, array_c);
    }
    
    /* Compute checksum to verify execution and prevent optimization */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < n; i += 17) {
        checksum += array_c[i] + prefix[i];
    }
    
    for (int i = 0; i < m * m; i += 23) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Conditional compilation for debugging */
#ifdef DUMP_OMP
    /* Dummy function call that might hint the compiler about OpenMP clauses */
    void dummy_omp_ref(void *clause) {
        /* Empty - just to reference OpenMP in code */
    }
    
    /* Reference various OpenMP-related constructs */
    #pragma omp parallel
    {
        dummy_omp_ref(NULL);
    }
#endif
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(prefix);
    free(matrix);
    
    return 0;
}
