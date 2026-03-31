/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered OpenMP clause pretty-printing code
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o omp_coverage tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime evaluation to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(array1, array2, result)
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function for declare target with link */
#pragma omp declare target link(global_counter)
int global_counter = 0;

/* Function that uses scan directive - triggers _scantemp_ */
void scan_example(int *data, int n, int *prefix_sum) {
    int sum = 0;
    
    /* This should generate _scantemp_ and potentially _reductemp_ */
    #pragma omp parallel for simd reduction(+:sum) scan(inscan:prefix_sum[:n])
    for (int i = 0; i < n; i++) {
        sum += data[i];
        
        #pragma omp scan inclusive(prefix_sum[i])
        prefix_sum[i] = sum;
    }
}

/* Function with collapse clause - may generate _condtemp_ */
void collapse_example(double *matrix, int rows, int cols) {
    volatile int bound = g_volatile_bound % 50 + 10; /* Prevent optimization */
    
    /* Complex collapse with runtime bounds may generate condition temporaries */
    #pragma omp parallel for collapse(2) if(rows > 10 && cols > 10)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols && j < bound; j++) {
            int idx = i * cols + j;
            if (omp_get_thread_num() % 2 == 0) {
                matrix[idx] *= 1.1;
            } else {
                matrix[idx] *= 0.9;
            }
        }
    }
}

/* Function with reduction on complex expression - may generate _reductemp_ */
double complex_reduction_example(double *data, int n) {
    double sum = 0.0;
    double product = 1.0;
    
    /* Complex reduction that might need temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            private(product) /* Force private copies */
    for (int i = 0; i < n; i++) {
        double val = data[i];
        sum += val;
        
        /* Thread-dependent computation */
        if (omp_get_thread_num() % 3 == 0) {
            product *= (val + 1.0);
        } else if (omp_get_thread_num() % 3 == 1) {
            product *= (val * 0.5);
        } else {
            product *= (val - 0.1);
        }
    }
    
    return sum + product;
}

/* Target region with data mapping */
void target_example(double *host_data, int n) {
    #pragma omp target enter data map(to: host_data[:n])
    
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < n; i++) {
        host_data[i] = host_data[i] * 2.0 + (double)i / n;
    }
    
    #pragma omp target exit data map(from: host_data[:n])
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 100 + (rand() % 100);
    int size2 = 50 + (rand() % 50);
    int matrix_size = 30 + (rand() % 20);
    
    printf("Running OpenMP coverage test with sizes: %d, %d, %d\n", 
           size1, size2, matrix_size);
    
    /* Allocate arrays */
    int *data1 = (int*)malloc(size1 * sizeof(int));
    int *prefix = (int*)malloc(size1 * sizeof(int));
    double *array1 = (double*)malloc(size2 * sizeof(double));
    double *array2 = (double*)malloc(size2 * sizeof(double));
    double *result = (double*)malloc(size2 * sizeof(double));
    double *matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < size1; i++) {
        data1[i] = i % 10;
        prefix[i] = 0;
    }
    
    for (int i = 0; i < size2; i++) {
        array1[i] = i * 0.5;
        array2[i] = i * 0.3;
        result[i] = 0.0;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (i % 10) * 0.1;
    }
    
    /* Execute various OpenMP constructs to trigger different clauses */
    
    /* 1. Scan example - should trigger _scantemp_ */
    printf("Running scan example...\n");
    scan_example(data1, size1, prefix);
    
    /* 2. Collapse example - may trigger _condtemp_ */
    printf("Running collapse example...\n");
    collapse_example(matrix, matrix_size, matrix_size);
    
    /* 3. Complex reduction - may trigger _reductemp_ */
    printf("Running complex reduction...\n");
    double red_result = complex_reduction_example(array1, size2);
    
    /* 4. Use declare target enter function */
    printf("Running declare target function...\n");
    #pragma omp target map(tofrom: array1[:size2], array2[:size2], result[:size2])
    {
        add_vectors(array1, array2, result, size2);
    }
    
    /* 5. Target region with update */
    printf("Running target example...\n");
    target_example(array1, size2);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    double checksum2 = 0.0;
    
    #pragma omp parallel for reduction(+:checksum1)
    for (int i = 0; i < size1; i++) {
        checksum1 += prefix[i] + data1[i];
    }
    
    #pragma omp parallel for reduction(+:checksum2)
    for (int i = 0; i < size2; i++) {
        checksum2 += result[i] + array1[i];
    }
    
    printf("Checksums: int=%d, double=%.2f, reduction_result=%.2f\n", 
           checksum1, checksum2, red_result);
    
    /* Cleanup */
    free(data1);
    free(prefix);
    free(array1);
    free(array2);
    free(result);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP constructs
 * This won't directly trigger pretty-printing but helps keep constructs alive */
#ifdef DUMP_OMP
void __attribute__((used)) dump_hint() {
    /* Reference various OpenMP clause types */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Empty but keeps structure */
        }
    }
}
#endif
