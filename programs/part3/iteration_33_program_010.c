/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp tree-pretty-print-coverage.c -o coverage_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(array1, array2, result_array)
void vec_add(int n, double *a, double *b, double *c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_scan(int n, double *data, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) scan(inscan:prefix)
    for (int i = 0; i < n; i++) {
        double val = data[i];
        
        /* Inscan phase */
        #pragma omp scan inclusive(prefix)
        {
            sum += val;
            prefix[i] = sum;
        }
    }
    
    return sum;
}

/* Function with collapse clause - may generate _condtemp_ */
void nested_loop_collapse(int rows, int cols, double *matrix) {
    int bound = volatile_bound; /* Volatile prevents constant propagation */
    
    /* Collapsed loop with non-trivial bound */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            if (idx % 2 == 0) {
                matrix[idx] = i * j * 1.5;
            } else {
                matrix[idx] = i + j * 0.5;
            }
        }
    }
}

/* Complex reduction with nested parallelism */
double complex_reduction(int size, double *arr) {
    double total = 0.0;
    
    #pragma omp target teams distribute parallel for reduction(+:total) \
                     map(to: arr[0:size]) map(from: total)
    for (int i = 0; i < size; i++) {
        /* Data-dependent computation */
        double val = arr[i];
        if (omp_get_team_num() % 2 == 0) {
            val *= 1.1;
        }
        total += val * (i + 1);
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Dynamic sizes prevent compile-time optimization */
    int n1 = 100 + (rand() % 100);
    int n2 = 50 + (rand() % 50);
    int rows = 20 + (rand() % 30);
    int cols = 15 + (rand() % 25);
    
    printf("Running with n1=%d, n2=%d, rows=%d, cols=%d\n", n1, n2, rows, cols);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(n1 * sizeof(double));
    double *array2 = (double *)malloc(n1 * sizeof(double));
    double *result = (double *)malloc(n1 * sizeof(double));
    double *prefix_arr = (double *)malloc(n2 * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < n1; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.7;
    }
    
    for (int i = 0; i < n2; i++) {
        prefix_arr[i] = (i % 3 == 0) ? 2.0 : 1.0;
    }
    
    /* 1. Test reduction and scan (targets _reductemp_ and _scantemp_) */
    double scan_sum = compute_prefix_scan(n2, prefix_arr, prefix_arr);
    printf("Scan sum: %f\n", scan_sum);
    
    /* 2. Test nested collapse (may generate _condtemp_) */
    nested_loop_collapse(rows, cols, matrix);
    
    /* 3. Test declare target enter with to clause */
    #pragma omp target data map(to: array1[0:n1], array2[0:n1]) map(from: result[0:n1])
    {
        vec_add(n1, array1, array2, result);
    }
    
    /* 4. Test complex reduction in target region */
    double total = complex_reduction(n1, array1);
    printf("Total reduction: %f\n", total);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < n1 && i < 10; i++) {
        checksum += result[i];
    }
    for (int i = 0; i < rows * cols && i < 20; i += 3) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Conditional compilation for debugging */
    #ifdef DUMP_OMP
    /* Dummy function call that might hint the compiler */
    void dummy_omp_ref(void) {
        /* Reference various OpenMP constructs */
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            if (tid == 0) {
                printf("Thread 0 running\n");
            }
        }
    }
    dummy_omp_ref();
    #endif
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(prefix_arr);
    free(matrix);
    
    return 0;
}
