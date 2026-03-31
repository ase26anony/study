/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered OpenMP clause pretty-printing code
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o test_omp test.c
 * Or: gcc -O2 -fopenmp -fopenmp-simd -fdump-tree-all -foffload=disable -o test_omp test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global volatile variable to prevent optimization */
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
            scan(inscan:prefix_sum:sum) private(prefix_sum)
    for (int i = 0; i < n; i++) {
        double prefix_sum;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum + data[i];
        
        sum += data[i];
        prefix[i] = prefix_sum;
    }
    
    return sum;
}

/* Function with nested collapsed loops - may generate _condtemp_ */
void process_matrix(int rows, int cols, double *matrix) {
    int bound = g_volatile_bound;  /* Use volatile to prevent optimization */
    
    /* Collapsed loop with non-trivial bound - may generate condition temporaries */
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation */
            if ((i + j) % 2 == 0) {
                matrix[idx] *= 2.0;
            } else {
                matrix[idx] /= 2.0;
            }
        }
    }
}

/* Complex reduction with multiple operations */
double complex_reduction(double *arr, int n) {
    double total = 0.0;
    double product = 1.0;
    
    /* Combined reduction - may generate multiple _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:total) reduction(*:product) \
            private(local_total, local_product)
    for (int i = 0; i < n; i++) {
        double local_total = 0.0;
        double local_product = 1.0;
        
        /* Nested loop inside parallel region */
        for (int j = 0; j < 5; j++) {
            local_total += arr[i] * j;
            local_product *= (arr[i] + j);
        }
        
        total += local_total;
        product *= local_product;
    }
    
    return total + product;
}

/* Target region with data mapping */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for \
            map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            reduction(+:g_volatile_bound)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        /* Thread-dependent operation */
        if (omp_get_team_num() % 2 == 0) {
            c[i] += 1.0;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 1000 + (rand() % 100);
    int size2 = 500 + (rand() % 50);
    int rows = 100 + (rand() % 20);
    int cols = 100 + (rand() % 20);
    
    /* Allocate arrays */
    double *data1 = (double *)malloc(size1 * sizeof(double));
    double *data2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    double *prefix = (double *)malloc(size1 * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < size1; i++) {
        data1[i] = i * 0.1;
        data2[i] = i * 0.2;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (i % 100) * 0.01;
    }
    
    /* 1. Test reduction and scan - should generate _reductemp_ and _scantemp_ */
    printf("Computing prefix sum...\n");
    double sum1 = compute_prefix_sum(data1, size1, prefix);
    printf("Prefix sum total: %f\n", sum1);
    
    /* 2. Test collapsed loops - may generate _condtemp_ */
    printf("Processing matrix...\n");
    process_matrix(rows, cols, matrix);
    
    /* 3. Test complex reduction */
    printf("Complex reduction...\n");
    double result2 = complex_reduction(data1, size2);
    printf("Complex reduction result: %f\n", result2);
    
    /* 4. Test declare target enter with to clause */
    printf("Adding vectors...\n");
    #pragma omp target data map(to: data1[0:size1], data2[0:size1]) \
                            map(from: result[0:size1])
    {
        add_vectors(data1, data2, result, size1);
    }
    
    /* 5. Test target region with reduction */
    printf("Target computation...\n");
    target_computation(data1, data2, result, size1);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < size1; i += 10) {
        checksum += data1[i] + result[i] + prefix[i];
    }
    
    for (int i = 0; i < rows * cols; i += 20) {
        checksum += matrix[i];
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

/* Dummy function to hint compiler about OpenMP constructs
 * This might help keep the structures alive for pretty-printing
 */
#ifdef DUMP_OMP
void __attribute__((used)) dump_hint() {
    /* References to OpenMP clause types - may help keep them in IR */
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE__REDUCTEMP_]));
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE__CONDTEMP_]));
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE__SCANTEMP_]));
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE_ENTER]));
}
#endif
