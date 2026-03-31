/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -foffload=disable
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(a,b,result)
void add_vectors(double *a, double *b, double *result, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
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
        double val = data[i] * (seed + i % 7);
        
        /* Prefix scan phase */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Reduction */
        total += val;
        
        /* Store prefix sum */
        data[i] = prefix_sum;
    }
    
    return total;
}

/* Function with nested collapse - may generate _condtemp_ */
void nested_collapse_loop(int *matrix, int rows, int cols) {
    int volatile bound = g_volatile_bound;
    
    /* Complex loop bound that may require condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(omp_get_num_threads() > 1)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols && j < (bound - i); j++) {
            int idx = i * cols + j;
            
            /* Data-dependent operation */
            if ((i + j) % 3 == omp_get_thread_num() % 3) {
                matrix[idx] = i * j;
            } else {
                matrix[idx] = i + j;
            }
        }
    }
}

/* Target region with complex data mapping */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            reduction(+:c[0:n]) num_teams(2) thread_limit(128)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        
        /* Thread-dependent operation */
        if (omp_get_team_num() % 2 == 0) {
            c[i] += 0.5;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 100 + (rand() % 100);
    int size2 = 50 + (rand() % 50);
    int matrix_size = 30 + (rand() % 20);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    int *matrix = (int *)malloc(matrix_size * matrix_size * sizeof(int));
    
    if (!array1 || !array2 || !result || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 0.1;
        array2[i] = (size1 - i) * 0.05;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = 0;
    }
    
    /* 1. Test reduction and scan - should generate _reductemp_ and _scantemp_ */
    printf("Testing reduction and scan...\n");
    double total = compute_prefix_sum(array1, size1, seed);
    printf("Total from prefix sum: %f\n", total);
    
    /* 2. Test nested collapse - may generate _condtemp_ */
    printf("Testing nested collapse...\n");
    nested_collapse_loop(matrix, matrix_size, matrix_size);
    
    /* 3. Test declare target enter with to clause */
    printf("Testing declare target enter...\n");
    
    /* The declare target directive above should generate ENTER clause with TO */
    
    /* 4. Test target region */
    printf("Testing target region...\n");
    target_computation(array1, array2, result, size1);
    
    /* 5. Additional complex OpenMP construct with multiple clauses */
    printf("Testing combined construct...\n");
    {
        double sum = 0.0;
        double prefix = 0.0;
        volatile int v_bound = size2;
        
        #pragma omp parallel for simd reduction(+:sum) \
                collapse(2) scan(inscan:prefix) \
                if(omp_in_parallel())
        for (int i = 0; i < v_bound && i < size1; i++) {
            for (int j = 0; j < 3; j++) {
                double val = array1[i] * (j + 1);
                
                #pragma omp scan inclusive(prefix)
                prefix += val;
                
                sum += val;
                
                /* Conditional operation */
                if ((i + j + omp_get_thread_num()) % 5 == 0) {
                    result[i] += prefix;
                }
            }
        }
        
        printf("Combined construct sum: %f\n", sum);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 10; i++) {
        checksum += array1[i] + result[i];
    }
    
    int matrix_checksum = 0;
    for (int i = 0; i < matrix_size && i < 5; i++) {
        for (int j = 0; j < matrix_size && j < 5; j++) {
            matrix_checksum += matrix[i * matrix_size + j];
        }
    }
    
    printf("Final checksums: array=%.3f, matrix=%d\n", checksum, matrix_checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep clause information in the IR
 */
#ifdef DUMP_OMP
void __attribute__((noinline)) 
dummy_omp_clause_hint(int clause_type) {
    /* This function doesn't do anything useful, but its existence
     * might affect how the compiler handles OpenMP clause information
     */
    volatile int x = clause_type;
    (void)x;
}
#endif
