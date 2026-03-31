/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered OpenMP clause pretty-printing code
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp tree-pretty-print-coverage.c -o coverage_test
 * Or: gcc -O2 -fopenmp -fopenmp-simd -fdump-tree-all -foffload=disable tree-pretty-print-coverage.c -o coverage_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(array_a, array_b, array_c)
void vec_add(int n, double *a, double *b, double *c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_scan(int n, double *data, double *prefix) {
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) safelen(8) \
            scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        /* Exclusive scan phase */
        #pragma omp scan exclusive(prefix_sum)
        {
            prefix[i] = prefix_sum;
            prefix_sum += data[i];
        }
        sum += data[i];
    }
    
    return sum;
}

/* Function with nested collapse - may generate _condtemp_ */
void nested_collapse_loop(int rows, int cols, double *matrix) {
    int i, j;
    
    /* Use volatile variable to prevent optimization of loop bounds */
    volatile int local_rows = rows;
    volatile int local_cols = cols;
    
    /* This collapse with volatile bounds may generate condition temporaries */
    #pragma omp parallel for collapse(2) private(i, j) \
            schedule(dynamic, 4)
    for (i = 0; i < local_rows; i++) {
        for (j = 0; j < local_cols; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation to prevent optimization */
            if ((i + j) % 3 == 0) {
                matrix[idx] = matrix[idx] * 1.5;
            } else if ((i + j) % 3 == 1) {
                matrix[idx] = matrix[idx] / 1.3;
            } else {
                matrix[idx] = matrix[idx] + omp_get_thread_num();
            }
        }
    }
}

/* Complex reduction with multiple clauses */
double complex_reduction(int size, double *arr1, double *arr2) {
    double total = 0.0;
    double max_val = -1e30;
    double min_val = 1e30;
    
    /* Combined construct with multiple reductions */
    #pragma omp target teams distribute parallel for \
            reduction(+:total) reduction(max:max_val) reduction(min:min_val) \
            map(to: arr1[0:size], arr2[0:size]) map(from: total, max_val, min_val)
    for (int i = 0; i < size; i++) {
        double val = arr1[i] * arr2[i];
        total += val;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
        
        /* Thread-dependent operation */
        if (omp_get_team_num() % 2 == 0) {
            val *= 1.1;
        }
    }
    
    return total + max_val - min_val;
}

int main(int argc, char *argv[]) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int n1 = 1000 + (rand() % 100);
    int n2 = 500 + (rand() % 50);
    int rows = 50 + (rand() % 20);
    int cols = 40 + (rand() % 20);
    
    printf("Running OpenMP coverage test with sizes: n1=%d, n2=%d, rows=%d, cols=%d\n",
           n1, n2, rows, cols);
    
    /* Allocate arrays */
    double *data = (double *)malloc(n1 * sizeof(double));
    double *prefix = (double *)malloc(n1 * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    double *arr1 = (double *)malloc(n2 * sizeof(double));
    double *arr2 = (double *)malloc(n2 * sizeof(double));
    double *arr3 = (double *)malloc(n2 * sizeof(double));
    
    if (!data || !prefix || !matrix || !arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < n1; i++) {
        data[i] = (i % 10) * 0.1;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (i % 100) * 0.01;
    }
    
    for (int i = 0; i < n2; i++) {
        arr1[i] = (i % 20) * 0.05;
        arr2[i] = (i % 15) * 0.07;
    }
    
    /* Test 1: Reduction and scan (for _reductemp_ and _scantemp_) */
    printf("Test 1: Running reduction with scan...\n");
    double sum1 = compute_prefix_scan(n1, data, prefix);
    
    /* Test 2: Nested collapse (for _condtemp_) */
    printf("Test 2: Running nested collapse loop...\n");
    nested_collapse_loop(rows, cols, matrix);
    
    /* Test 3: Declare target enter (for ENTER clause) */
    printf("Test 3: Running declare target enter...\n");
    
    /* Explicitly map arrays for target region */
    #pragma omp target enter data map(to: arr1[0:n2], arr2[0:n2]) map(alloc: arr3[0:n2])
    
    /* Call the function that was entered via declare target */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n2; i++) {
        arr3[i] = arr1[i] + arr2[i] + omp_get_team_num() * 0.001;
    }
    
    #pragma omp target exit data map(from: arr3[0:n2]) map(release: arr1[0:n2], arr2[0:n2])
    
    /* Test 4: Complex reduction in target region */
    printf("Test 4: Running complex reduction...\n");
    double result = complex_reduction(n2, arr1, arr2);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    
    /* Use only a subset for checksum to keep it fast */
    for (int i = 0; i < 100 && i < n1; i++) {
        checksum += prefix[i];
    }
    
    for (int i = 0; i < 100 && i < rows * cols; i++) {
        checksum += matrix[i];
    }
    
    for (int i = 0; i < 100 && i < n2; i++) {
        checksum += arr3[i];
    }
    
    checksum += sum1 + result;
    
    printf("Final checksum: %f\n", checksum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(data);
    free(prefix);
    free(matrix);
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clauses */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* These declarations hint the compiler about OpenMP clause types */
    asm volatile ("/* OMP_CLAUSE__REDUCTEMP_ */");
    asm volatile ("/* OMP_CLAUSE__CONDTEMP_ */");
    asm volatile ("/* OMP_CLAUSE__SCANTEMP_ */");
    asm volatile ("/* OMP_CLAUSE_ENTER */");
}
#endif
