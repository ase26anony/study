/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(a, b, c)
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *data, int n, int seed) {
    double prefix_sum = 0.0;
    double total = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:total) \
            simdlen(4) safelen(8) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = data[i] + (i % (seed + 1)) * 0.1;
        
        /* Prefix scan phase */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Reduction */
        total += val;
        
        /* Store modified value */
        data[i] = prefix_sum;
    }
    
    return total;
}

/* Function with collapse clause that may generate _condtemp_ */
void nested_loop_computation(int *matrix, int rows, int cols) {
    int local_bound = g_volatile_bound;
    
    /* Complex loop bound may generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(rows * cols > 1000)  /* Conditional clause */
    for (int i = 0; i < rows && i < local_bound; i++) {
        for (int j = 0; j < cols && j < (local_bound - i); j++) {
            int idx = i * cols + j;
            
            /* Data-dependent operation */
            if ((i + j) % (omp_get_thread_num() + 2) == 0) {
                matrix[idx] *= 2;
            } else {
                matrix[idx] += 1;
            }
        }
    }
}

/* Target region with declare target enter */
void target_computation(double *a, double *b, double *c, int n) {
    /* Use the function that was entered via declare target */
    #pragma omp target map(tofrom: a[0:n], b[0:n]) map(from: c[0:n]) \
            device(0) if(n > 500)
    {
        add_vectors(a, b, c, n);
    }
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 500 + (rand() % 500);
    int size2 = 300 + (rand() % 300);
    int rows = 50 + (seed % 50);
    int cols = 40 + (seed % 40);
    
    printf("Running with seed=%d, size1=%d, size2=%d\n", seed, size1, size2);
    
    /* Allocate arrays */
    double *data1 = (double *)malloc(size1 * sizeof(double));
    double *data2 = (double *)malloc(size2 * sizeof(double));
    double *result = (double *)malloc(size2 * sizeof(double));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    
    if (!data1 || !data2 || !result || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size1; i++) {
        data1[i] = i * 0.5 + (i % 3);
    }
    
    for (int i = 0; i < size2; i++) {
        data2[i] = i * 0.3 + (i % 5);
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = i;
    }
    
    /* 1. Trigger _reductemp_ and _scantemp_ clauses */
    printf("Computing prefix sum...\n");
    double total1 = compute_prefix_sum(data1, size1, seed);
    
    /* 2. Trigger _condtemp_ clause with nested collapsed loops */
    printf("Running nested loop computation...\n");
    nested_loop_computation(matrix, rows, cols);
    
    /* 3. Trigger ENTER clause with to() mapper */
    printf("Running target computation...\n");
    target_computation(data2, data2, result, size2);
    
    /* Compute checksums to prevent dead code elimination */
    double checksum = 0.0;
    int int_checksum = 0;
    
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < size1; i += 10) {
        checksum += data1[i];
    }
    
    #pragma omp parallel for reduction(+:int_checksum)
    for (int i = 0; i < rows * cols; i += 7) {
        int_checksum += matrix[i];
    }
    
    printf("Checksums: double=%.2f, int=%d\n", checksum, int_checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(result);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep clause information in the AST
 */
#ifdef DUMP_OMP
void __attribute__((used)) 
dump_omp_clause_hint(int clause_type) {
    /* This function doesn't need to do anything meaningful.
     * It's just to prevent optimization of OpenMP constructs.
     */
    volatile int x = clause_type;
    (void)x;
}
#endif
