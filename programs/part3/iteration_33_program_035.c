/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp tree-pretty-print-coverage.c -o coverage_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int volatile_bound = 100;

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
            scan(inscan:prefix_sum:sum) private(prefix_sum)
    for (int i = 0; i < n; i++) {
        double prefix_sum;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum + data[i];
        
        prefix[i] = prefix_sum;
        sum += data[i];
    }
    
    return sum;
}

/* Function with nested loops and collapse - may generate _condtemp_ */
void nested_loop_computation(int m, int n, double *matrix) {
    int bound = volatile_bound; /* Use volatile to prevent constant propagation */
    
    /* Complex loop bound may generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(bound > 50)  /* Additional condition */
    for (int i = 0; i < m && i < bound; i++) {
        for (int j = 0; j < n && j < bound; j++) {
            int idx = i * n + j;
            /* Data-dependent computation */
            if ((i + j) % 2 == 0) {
                matrix[idx] = matrix[idx] * 2.0;
            } else {
                matrix[idx] = matrix[idx] / 2.0;
            }
        }
    }
}

/* Target region with teams - complex enough for internal temporaries */
void target_computation(int n, double *a, double *b, double *c) {
    #pragma omp target teams distribute parallel for \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            reduction(+:n)  /* May generate reduction temporaries */
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        /* Thread-dependent operation */
        if (omp_get_team_num() % 2 == 0) {
            c[i] += 0.5;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 100 + (rand() % 100);
    int size2 = 50 + (rand() % 50);
    int matrix_rows = 30 + (rand() % 20);
    int matrix_cols = 40 + (rand() % 20);
    
    printf("Running OpenMP coverage test with sizes: %d, %d, %dx%d\n",
           size1, size2, matrix_rows, matrix_cols);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *array3 = (double *)malloc(size1 * sizeof(double));
    double *prefix = (double *)malloc(size1 * sizeof(double));
    double *matrix = (double *)malloc(matrix_rows * matrix_cols * sizeof(double));
    
    if (!array1 || !array2 || !array3 || !prefix || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.5;
        array3[i] = 0.0;
    }
    
    for (int i = 0; i < matrix_rows * matrix_cols; i++) {
        matrix[i] = (double)(i % 100);
    }
    
    /* 1. Test reduction and scan - should generate _reductemp_ and _scantemp_ */
    printf("Testing reduction and scan...\n");
    double total_sum = compute_prefix_sum(size1, array1, prefix);
    printf("Total sum from prefix scan: %f\n", total_sum);
    
    /* 2. Test nested loops with collapse - may generate _condtemp_ */
    printf("Testing nested loops with collapse...\n");
    nested_loop_computation(matrix_rows, matrix_cols, matrix);
    
    /* 3. Test declare target enter with to clause */
    printf("Testing declare target enter...\n");
    
    /* The declare target directive above should generate ENTER clause */
    /* Now use the function in a target region */
    #pragma omp target map(to: array1[0:size2], array2[0:size2]) \
                       map(from: array3[0:size2])
    {
        vec_add(size2, array1, array2, array3);
    }
    
    /* 4. Additional target computation with teams */
    printf("Testing target teams with reduction...\n");
    target_computation(size2, array1, array2, array3);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 10; i++) {
        checksum += prefix[i];
    }
    for (int i = 0; i < matrix_rows * matrix_cols && i < 100; i += 10) {
        checksum += matrix[i];
    }
    for (int i = 0; i < size2 && i < 10; i++) {
        checksum += array3[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(prefix);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep clause information in the AST
 */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* References to force inclusion of OpenMP constructs */
    volatile int dummy = 0;
    
    #pragma omp parallel reduction(+:dummy)
    {
        dummy += omp_get_thread_num();
    }
}
#endif
