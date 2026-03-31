/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp tree-pretty-print-coverage.c -o test
 * Or with: gcc -O2 -fopenmp -fopenmp-simd -fdump-tree-all -foffload=disable tree-pretty-print-coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_bound = 100;
volatile int g_seed = 42;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(map_to_from:g_volatile_bound)
void vec_add(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *arr, int n) {
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum) if(n > 1000)
    for (int i = 0; i < n; i++) {
        double val = arr[i];
        
        /* Inscan phase */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Reduction */
        sum += val;
        
        /* Store prefix sum back */
        arr[i] = prefix_sum;
    }
    
    return sum;
}

/* Function with nested collapse - may generate _condtemp_ */
void nested_collapse_loop(int *matrix, int rows, int cols) {
    int local_bound = g_volatile_bound; /* Use volatile to prevent optimization */
    
    /* Complex loop bound that may require condition temporaries */
    #pragma omp parallel for collapse(2) \
            firstprivate(local_bound) \
            if(rows * cols > local_bound)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Data-dependent operation */
            if ((i + j) % 2 == 0) {
                matrix[idx] = i * j;
            } else {
                matrix[idx] = i + j;
            }
            
            /* Thread-specific operation */
            if (omp_get_thread_num() % 3 == 0) {
                matrix[idx] += 1;
            }
        }
    }
}

/* Function with target region and declare target */
void target_region_computation(double *a, double *b, double *c, int n) {
    /* Use declare target with enter clause and to mapper */
    #pragma omp target enter data map(to: a[0:n], b[0:n]) \
            map(alloc: c[0:n]) depend(inout: a, b)
    
    #pragma omp target teams distribute parallel for \
            reduction(+:g_seed) if(n > 500)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        g_seed += (int)c[i]; /* Use volatile global */
    }
    
    #pragma omp target exit data map(from: c[0:n]) \
            map(release: a[0:n], b[0:n])
}

int main(int argc, char **argv) {
    /* Use argc to create variable but reproducible sizes */
    int base_size = 1000;
    if (argc > 1) {
        base_size = atoi(argv[1]) % 500 + 500;
    }
    
    int rows = base_size / 10;
    int cols = base_size / 20;
    int total_elements = rows * cols;
    
    /* Allocate arrays with runtime-determined sizes */
    double *array1 = (double *)malloc(base_size * sizeof(double));
    double *array2 = (double *)malloc(base_size * sizeof(double));
    double *array3 = (double *)malloc(base_size * sizeof(double));
    int *matrix = (int *)malloc(total_elements * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < base_size; i++) {
        array1[i] = i * 0.5;
        array2[i] = i * 0.3;
        array3[i] = 0.0;
    }
    
    printf("Starting OpenMP computations...\n");
    
    /* 1. Trigger _reductemp_ and _scantemp_ clauses */
    double total_sum = compute_prefix_sum(array1, base_size);
    printf("Prefix sum total: %f\n", total_sum);
    
    /* 2. Trigger _condtemp_ clause with nested collapse */
    nested_collapse_loop(matrix, rows, cols);
    
    /* 3. Use declare target with enter clause */
    #pragma omp declare target enter(vec_add) \
            to(map_to_from: array2, array3)
    
    /* Call the function that was entered */
    vec_add(array1, array2, array3, base_size);
    
    /* 4. Additional target region with complex clauses */
    target_region_computation(array1, array2, array3, base_size);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    int matrix_sum = 0;
    
    #pragma omp parallel for reduction(+:checksum, matrix_sum) \
            schedule(dynamic, 16)
    for (int i = 0; i < base_size; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        if (i < total_elements) {
            matrix_sum += matrix[i];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Matrix sum: %d\n", matrix_sum);
    printf("Volatile seed: %d\n", g_seed);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep clause information in the AST
 */
#ifdef DUMP_OMP
void __attribute__((noinline)) 
dummy_omp_clause_hint(int clause_type) {
    /* This function doesn't do anything useful,
     * but might prevent optimization of OpenMP structures */
    volatile int hint = clause_type;
    (void)hint;
}
#endif
