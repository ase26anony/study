/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered pretty-printing code for:
 * - OMP_CLAUSE__REDUCTEMP_
 * - OMP_CLAUSE__CONDTEMP_
 * - OMP_CLAUSE__SCANTEMP_
 * - OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp tree-pretty-print-coverage.c -o coverage_test
 * Or with: gcc -O2 -fopenmp -fopenmp-simd -fdump-tree-all -foffload=disable tree-pretty-print-coverage.c -o coverage_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_bound = 100;
volatile int g_collapse_bound = 50;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(map_to_from:array)
void vec_add(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *data, int n, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum:sum) private(prefix_sum)
    for (int i = 0; i < n; i++) {
        double prefix_sum;
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum + data[i];
        sum = prefix_sum;
        prefix[i] = prefix_sum;
    }
    
    return sum;
}

/* Function with nested loops and collapse - may generate _condtemp_ */
void nested_collapse_loops(int *matrix, int rows, int cols) {
    int i, j;
    
    /* Use volatile variable in loop bound to prevent optimization */
    int bound = g_collapse_bound;
    
    /* This may generate condition temporaries for collapse logic */
    #pragma omp parallel for collapse(2) private(i, j) \
            if(bound > 10)  /* Additional condition to complicate lowering */
    for (i = 0; i < rows && i < bound; i++) {
        for (j = 0; j < cols && j < bound; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation to prevent optimization */
            if ((i + j) % 3 == 0) {
                matrix[idx] = i * j;
            } else if ((i + j) % 3 == 1) {
                matrix[idx] = i + j;
            } else {
                matrix[idx] = i - j;
            }
            
            /* Thread-dependent operation */
            if (omp_get_thread_num() % 2 == 0) {
                matrix[idx] += 1;
            }
        }
    }
}

/* Complex reduction with multiple temporaries */
double complex_reduction(double *data, int n) {
    double result = 0.0;
    double temp_sum = 0.0;
    
    /* Multiple reductions and complex loop may generate _reductemp_ */
    #pragma omp parallel for reduction(+:result, temp_sum) \
            schedule(dynamic, 4) \
            if(n > 100)  /* Conditional to create more temporaries */
    for (int i = 0; i < n; i++) {
        double val = data[i];
        
        /* Complex conditional operations */
        if (val > 0.5) {
            result += val * val;
            temp_sum += val;
        } else if (val < -0.5) {
            result -= val * val;
            temp_sum -= val;
        } else {
            result += 0.1;
        }
        
        /* Thread-specific operations */
        int tid = omp_get_thread_num();
        if (tid % 3 == 0) {
            result += 0.01;
        }
    }
    
    return result + temp_sum * 0.5;
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 500 + (rand() % 100);
    int size2 = 300 + (rand() % 100);
    int matrix_size = 100 + (rand() % 50);
    
    printf("Running OpenMP coverage test with sizes: %d, %d, %d\n", 
           size1, size2, matrix_size);
    
    /* Allocate arrays */
    double *data1 = (double *)malloc(size1 * sizeof(double));
    double *data2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    double *prefix = (double *)malloc(size1 * sizeof(double));
    int *matrix = (int *)malloc(matrix_size * matrix_size * sizeof(int));
    
    if (!data1 || !data2 || !result || !prefix || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        data1[i] = (double)i / size1;
        data2[i] = (double)(size1 - i) / size1;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = i % 100;
    }
    
    /* Test 1: Prefix sum with scan (should generate _scantemp_) */
    printf("Test 1: Computing prefix sum with scan...\n");
    double total_sum = compute_prefix_sum(data1, size1, prefix);
    printf("  Total sum: %f\n", total_sum);
    
    /* Test 2: Nested collapse loops (may generate _condtemp_) */
    printf("Test 2: Running nested collapse loops...\n");
    nested_collapse_loops(matrix, matrix_size, matrix_size);
    
    /* Test 3: Complex reduction (may generate _reductemp_) */
    printf("Test 3: Complex reduction...\n");
    double complex_result = complex_reduction(data1, size1);
    printf("  Complex reduction result: %f\n", complex_result);
    
    /* Test 4: Declare target enter with to clause */
    printf("Test 4: Using declare target enter...\n");
    
    /* The declare target directive above should generate ENTER clause */
    /* Now use target region */
    #pragma omp target map(tofrom: data1[0:size1], data2[0:size1]) \
                       map(from: result[0:size1])
    {
        vec_add(data1, data2, result, size1);
    }
    
    /* Test 5: Combined parallel for simd with reduction */
    printf("Test 5: Combined parallel for simd...\n");
    double final_sum = 0.0;
    #pragma omp parallel for simd reduction(+:final_sum) \
            if(size1 > 200)  /* Conditional to create temporaries */
    for (int i = 0; i < size1; i++) {
        final_sum += result[i] * prefix[i];
        
        /* Additional conditional to create more complex IR */
        if (i % 7 == 0) {
            final_sum += 0.001 * omp_get_thread_num();
        }
    }
    printf("  Final combined sum: %f\n", final_sum);
    
    /* Compute checksum to verify execution and prevent dead code elimination */
    double checksum = 0.0;
    checksum += total_sum;
    checksum += complex_result;
    checksum += final_sum;
    
    /* Add some matrix values */
    for (int i = 0; i < matrix_size; i += 10) {
        checksum += matrix[i * matrix_size + i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(result);
    free(prefix);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep clause information in the IR
 */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* This function doesn't need to do anything,
     * but its existence might affect code generation */
    volatile int x = 0;
    (void)x;
}
#endif
