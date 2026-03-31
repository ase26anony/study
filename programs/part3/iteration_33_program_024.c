/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(map_to_from:result)
void add_vectors(double *a, double *b, double *result, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *data, int n, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum:sum) if(n > 1000)
    for (int i = 0; i < n; i++) {
        double prefix_sum;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum + data[i];
        
        if (i % 2 == 0) {
            prefix[i] = prefix_sum * 0.5;
        } else {
            prefix[i] = prefix_sum * 2.0;
        }
        
        sum += data[i];
    }
    
    return sum;
}

/* Function with nested collapse - may generate _condtemp_ */
void nested_collapse_loop(int *matrix, int rows, int cols) {
    int bound = g_volatile_bound; /* Volatile prevents constant propagation */
    
    /* Complex loop bound may generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(rows * cols > bound) \
            shared(matrix) private(bound)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Data-dependent operation */
            if ((i + j) % (omp_get_thread_num() + 1) == 0) {
                matrix[i * cols + j] = i * j;
            } else {
                matrix[i * cols + j] = i + j;
            }
        }
    }
}

/* Function using declare target with enter clause */
void target_region_computation(double *a, double *b, double *c, int n) {
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                     device(0) if(n > 500)
    {
        add_vectors(a, b, c, n);
        
        /* Additional computation inside target */
        #pragma omp parallel for reduction(+:c[:n])
        for (int i = 0; i < n; i++) {
            c[i] = c[i] * (i % 10 + 1);
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes prevent compile-time optimization */
    int size1 = 1000 + (rand() % 1000);
    int size2 = 500 + (rand() % 500);
    int rows = 50 + (rand() % 50);
    int cols = 30 + (rand() % 30);
    
    printf("Running with sizes: %d, %d, %dx%d\n", 
           size1, size2, rows, cols);
    
    /* Allocate arrays */
    double *data1 = (double *)malloc(size1 * sizeof(double));
    double *data2 = (double *)malloc(size2 * sizeof(double));
    double *prefix = (double *)malloc(size1 * sizeof(double));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    double *result = (double *)malloc(size2 * sizeof(double));
    
    if (!data1 || !data2 || !prefix || !matrix || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        data1[i] = i * 0.1 + (i % 3) * 0.5;
    }
    
    for (int i = 0; i < size2; i++) {
        data2[i] = i * 0.2 - (i % 5) * 0.3;
    }
    
    /* 1. Test reduction and scan - should generate _reductemp_ and _scantemp_ */
    double total_sum = compute_prefix_sum(data1, size1, prefix);
    printf("Prefix sum total: %f\n", total_sum);
    
    /* 2. Test nested collapse - may generate _condtemp_ */
    nested_collapse_loop(matrix, rows, cols);
    
    /* Compute checksum from matrix */
    int matrix_sum = 0;
    #pragma omp parallel for reduction(+:matrix_sum)
    for (int i = 0; i < rows * cols; i++) {
        matrix_sum += matrix[i];
    }
    printf("Matrix checksum: %d\n", matrix_sum);
    
    /* 3. Test declare target with enter clause */
    target_region_computation(data1, data2, result, 
                             (size1 < size2) ? size1 : size2);
    
    /* Verify target computation */
    double result_sum = 0.0;
    for (int i = 0; i < ((size1 < size2) ? size1 : size2); i++) {
        result_sum += result[i];
    }
    printf("Target result sum: %f\n", result_sum);
    
    /* Additional test: combined construct with multiple clauses */
    {
        double combined_array[100];
        for (int i = 0; i < 100; i++) {
            combined_array[i] = i * 0.01;
        }
        
        /* Combined parallel for simd with reduction */
        #pragma omp parallel for simd reduction(+:combined_array[:100]) \
                schedule(dynamic, 10) if(size1 > 800)
        for (int i = 0; i < 100; i++) {
            combined_array[i] += omp_get_thread_num() * 0.001;
        }
        
        /* Force usage to prevent dead code elimination */
        double check = 0;
        for (int i = 0; i < 100; i++) {
            check += combined_array[i];
        }
        printf("Combined array check: %f\n", check);
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(prefix);
    free(matrix);
    free(result);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep clause information in the AST
 */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* These don't execute, but their presence might affect AST generation */
    #pragma omp declare target enter(hint_omp_clauses)
    
    volatile int hint = 0;
    #pragma omp parallel for reduction(+:hint)
    for (int i = 0; i < 10; i++) {
        hint += i;
    }
}
#endif
