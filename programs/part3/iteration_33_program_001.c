/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(array1, array2, result)
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function for reduction complexity */
double complex_reduction(double *data, int n) {
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) safelen(8) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        /* Inscan phase */
        {
            double val = data[i];
            sum += val;
            prefix_sum += val;
        }
        
        /* Use thread-dependent condition to create complexity */
        if (omp_get_thread_num() % 2 == 0) {
            data[i] = prefix_sum * 0.5;
        } else {
            data[i] = prefix_sum * 1.5;
        }
    }
    
    return sum;
}

/* Function with nested loops and collapse - may generate _condtemp_ */
void nested_collapse(int rows, int cols, double *matrix) {
    int i, j;
    
    /* Use volatile variable in loop bound to prevent optimization */
    int bound = volatile_bound;
    
    /* This collapse clause with non-trivial bound may create condition temporaries */
    #pragma omp parallel for collapse(2) private(i, j) \
            if(rows > 50) num_threads(4)
    for (i = 0; i < rows && i < bound; i++) {
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation */
            if ((i + j) % 3 == 0) {
                matrix[idx] = matrix[idx] * 2.0;
            } else if ((i + j) % 3 == 1) {
                matrix[idx] = matrix[idx] / 2.0;
            }
            /* Add some branching complexity */
            matrix[idx] += (i > j) ? 1.0 : -1.0;
        }
    }
}

/* Target region with teams - complex OpenMP structure */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            reduction(+:a[0:n]) if(n > 1000)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        /* Thread-dependent operation */
        if (omp_get_team_num() % 2 == 0) {
            c[i] += omp_get_thread_num();
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to create variable but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Create data-dependent sizes to prevent optimization */
    int size1 = 500 + (rand() % 100);
    int size2 = 300 + (rand() % 100);
    int rows = 50 + (rand() % 50);
    int cols = 40 + (rand() % 40);
    
    printf("Running OpenMP coverage test with sizes: %d, %d, %dx%d\n", 
           size1, size2, rows, cols);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    double *target_a = (double *)malloc(size2 * sizeof(double));
    double *target_b = (double *)malloc(size2 * sizeof(double));
    double *target_c = (double *)malloc(size2 * sizeof(double));
    
    if (!array1 || !array2 || !result || !matrix || !target_a || !target_b || !target_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.5;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (i % 10) * 1.1;
    }
    
    for (int i = 0; i < size2; i++) {
        target_a[i] = i * 2.0;
        target_b[i] = i * 3.0;
    }
    
    /* 1. Test reduction with scan - should generate _reductemp_ and _scantemp_ */
    printf("Testing reduction with scan...\n");
    double total = complex_reduction(array1, size1);
    printf("Reduction total: %f\n", total);
    
    /* 2. Test nested collapse - may generate _condtemp_ */
    printf("Testing nested collapse...\n");
    nested_collapse(rows, cols, matrix);
    
    /* 3. Test declare target enter with to clause */
    printf("Testing declare target enter...\n");
    
    /* Call the function that was declared with enter */
    #pragma omp target map(to: array1[0:size1], array2[0:size1]) \
                       map(from: result[0:size1])
    {
        add_vectors(array1, array2, result, size1);
    }
    
    /* 4. Test target region with teams */
    printf("Testing target region...\n");
    target_computation(target_a, target_b, target_c, size2);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1; i += 10) {
        checksum += array1[i] + result[i];
    }
    for (int i = 0; i < rows * cols; i += 20) {
        checksum += matrix[i];
    }
    for (int i = 0; i < size2; i += 15) {
        checksum += target_c[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(matrix);
    free(target_a);
    free(target_b);
    free(target_c);
    
    return 0;
}
