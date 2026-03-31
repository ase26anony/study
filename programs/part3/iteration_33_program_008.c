/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o omp_coverage tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(array1, array2, result)
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction for reductemp generation */
double compute_sum(double *arr, int n) {
    double sum = 0.0;
    
    /* This parallel for simd with reduction may generate _reductemp_ */
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* Function with scan clause for scantemp generation */
void prefix_scan(double *input, double *output, int n) {
    double prefix_sum = 0.0;
    
    /* This should generate _scantemp_ for the scan operation */
    #pragma omp parallel for simd reduction(+:prefix_sum) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += input[i];
        output[i] = prefix_sum;
    }
}

/* Function with collapse for condtemp generation */
void nested_loop_computation(int *matrix, int rows, int cols) {
    int bound = g_volatile_bound; /* Volatile prevents compile-time optimization */
    
    /* Collapsed loop with non-trivial bound may generate _condtemp_ */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation to prevent optimization */
            if (omp_get_thread_num() % 2 == 0) {
                matrix[idx] = i + j;
            } else {
                matrix[idx] = i * j;
            }
        }
    }
}

/* Combined construct with multiple features */
void combined_features(double *data, int size) {
    double sum = 0.0;
    double scan_temp = 0.0;
    
    /* Combined parallel for simd with reduction and scan */
    #pragma omp parallel for simd reduction(+:sum) scan(inscan:scan_temp)
    for (int i = 0; i < size; i++) {
        /* Complex operation to ensure temporary generation */
        double val = data[i];
        if (val > 0.5) {
            val *= 2.0;
        }
        sum += val;
        
        #pragma omp scan inclusive(scan_temp)
        scan_temp += val;
        data[i] = scan_temp;
    }
}

/* Target region with declare target enter */
void target_computation(double *a, double *b, double *c, int n) {
    /* Enter clause with to mapper should trigger OMP_CLAUSE_ENTER_TO */
    #pragma omp target enter data map(to: a[0:n], b[0:n]) map(alloc: c[0:n])
    
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
    
    #pragma omp target exit data map(from: c[0:n])
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes prevent compile-time optimization */
    int size1 = 100 + (rand() % 100);
    int size2 = 50 + (rand() % 50);
    int rows = 20 + (rand() % 20);
    int cols = 10 + (rand() % 10);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    double *scan_input = (double *)malloc(size2 * sizeof(double));
    double *scan_output = (double *)malloc(size2 * sizeof(double));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 0.1;
        array2[i] = (size1 - i) * 0.1;
    }
    
    for (int i = 0; i < size2; i++) {
        scan_input[i] = (i % 10) * 0.5;
    }
    
    /* 1. Generate _reductemp_ through reduction */
    double sum = compute_sum(array1, size1);
    printf("Reduction sum: %f\n", sum);
    
    /* 2. Generate _scantemp_ through scan operation */
    prefix_scan(scan_input, scan_output, size2);
    printf("Scan result[%d] = %f\n", size2-1, scan_output[size2-1]);
    
    /* 3. Generate _condtemp_ through collapsed loop */
    nested_loop_computation(matrix, rows, cols);
    printf("Matrix[0][0] = %d\n", matrix[0]);
    
    /* 4. Combined features for multiple temporaries */
    combined_features(array1, size1);
    printf("Combined result[0] = %f\n", array1[0]);
    
    /* 5. Trigger ENTER clause with to mapper */
    /* The declare target directive above should generate this */
    add_vectors(array1, array2, result, size1);
    printf("Vector add result[0] = %f\n", result[0]);
    
    /* 6. Target computation */
    target_computation(array1, array2, result, size1);
    printf("Target computation result[0] = %f\n", result[0]);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 10; i++) {
        checksum += result[i] + array1[i];
    }
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(scan_input);
    free(scan_output);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep the structures alive for pretty-printing
 */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* References to OpenMP clause types that might keep them in IR */
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE__REDUCTEMP_]));
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE__CONDTEMP_]));
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE__SCANTEMP_]));
    asm volatile("" : : "r"(&omp_clause_code_name[OMP_CLAUSE_ENTER]));
}
#endif
