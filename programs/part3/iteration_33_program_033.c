/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global volatile variable to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(p1, p2, p3)
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *arr, int n, int seed) {
    double prefix_sum = 0.0;
    double total = 0.0;
    
    /* This construct should generate _reductemp_ and _scantemp_ */
    #pragma omp parallel for simd reduction(+:total) \
            simdlen(4) safelen(8) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = arr[i] * (seed + i % 7);
        
        /* Prefix scan phase */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Reduction */
        total += val;
        
        /* Store result back */
        arr[i] = prefix_sum;
    }
    
    return total;
}

/* Function with collapse clause - may generate _condtemp_ */
void nested_loop_computation(int *matrix, int rows, int cols, int base) {
    int bound = g_volatile_bound; /* Use volatile to prevent optimization */
    
    /* Collapsed loop with non-trivial bound - may generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(rows * cols > 1000) \
            num_threads(omp_get_max_threads() > 4 ? 4 : omp_get_max_threads())
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols && j < bound; j++) {
            int idx = i * cols + j;
            /* Data-dependent computation */
            if ((i + j) % 3 == 0) {
                matrix[idx] = (i * base + j) * (omp_get_thread_num() + 1);
            } else {
                matrix[idx] = (i * base - j) / ((omp_get_thread_num() % 3) + 1);
            }
        }
    }
}

/* Function with complex reduction */
double complex_reduction(double *data, int size, int iter) {
    double result = 0.0;
    
    /* Parallel region with reduction - may generate multiple _reductemp_ */
    #pragma omp parallel reduction(+:result)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        int chunk = size / nthreads;
        int start = tid * chunk;
        int end = (tid == nthreads - 1) ? size : start + chunk;
        
        /* Nested loop inside parallel region */
        for (int k = 0; k < iter; k++) {
            double local_sum = 0.0;
            #pragma omp simd reduction(+:local_sum)
            for (int i = start; i < end; i++) {
                local_sum += data[i] * (k + 1) * (tid + 1);
            }
            result += local_sum;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent optimization */
    int size1 = 500 + (rand() % 100);
    int size2 = 300 + (rand() % 100);
    int rows = 50 + (rand() % 20);
    int cols = 40 + (rand() % 20);
    
    printf("Running OpenMP coverage test with sizes: %d, %d, %dx%d\n", 
           size1, size2, rows, cols);
    
    /* Allocate arrays */
    double *arr1 = (double *)malloc(size1 * sizeof(double));
    double *arr2 = (double *)malloc(size1 * sizeof(double));
    double *arr3 = (double *)malloc(size1 * sizeof(double));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    double *data = (double *)malloc(size2 * sizeof(double));
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = i * 0.3;
    }
    
    for (int i = 0; i < size2; i++) {
        data[i] = (i % 10) * 0.1;
    }
    
    /* Test 1: Prefix sum with reduction and scan */
    printf("Test 1: Prefix sum with reduction and scan\n");
    double total1 = compute_prefix_sum(arr1, size1, seed);
    printf("  Total from prefix sum: %f\n", total1);
    
    /* Test 2: Nested loop with collapse */
    printf("Test 2: Nested loop with collapse\n");
    nested_loop_computation(matrix, rows, cols, seed);
    
    /* Compute checksum from matrix */
    int matrix_sum = 0;
    #pragma omp parallel for reduction(+:matrix_sum)
    for (int i = 0; i < rows * cols; i++) {
        matrix_sum += matrix[i] % 1000;
    }
    printf("  Matrix checksum: %d\n", matrix_sum);
    
    /* Test 3: Complex reduction */
    printf("Test 3: Complex reduction\n");
    double total2 = complex_reduction(data, size2, 3);
    printf("  Complex reduction result: %f\n", total2);
    
    /* Test 4: Declare target enter with to clause */
    printf("Test 4: Declare target enter\n");
    
    /* Additional declare target to ensure enter clause is processed */
    #pragma omp declare target enter(add_vectors) to(p1:length(size1))
    
    /* Target region using the entered function */
    #pragma omp target map(to: arr1[0:size1], arr2[0:size1]) \
                       map(from: arr3[0:size1]) \
                       if(size1 > 100)
    {
        add_vectors(arr1, arr2, arr3, size1);
    }
    
    /* Verify target computation */
    double target_sum = 0.0;
    for (int i = 0; i < size1 && i < 10; i++) {
        target_sum += arr3[i];
    }
    printf("  Target computation partial sum: %f\n", target_sum);
    
    /* Test 5: Teams distribute with reduction */
    printf("Test 5: Teams distribute\n");
    double teams_result = 0.0;
    #pragma omp target teams distribute parallel for \
                     reduction(+:teams_result) \
                     map(tofrom: teams_result) \
                     num_teams(2) thread_limit(64)
    for (int i = 0; i < size2; i++) {
        teams_result += data[i] * (i % 5 + 1);
    }
    printf("  Teams reduction result: %f\n", teams_result);
    
    /* Final checksum to verify execution */
    double final_checksum = total1 + total2 + teams_result + target_sum + matrix_sum;
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(matrix);
    free(data);
    
    return 0;
}
