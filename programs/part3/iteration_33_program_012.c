/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -foffload=disable
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(map_to_from:arr1, arr2, result)
void add_vectors(double *arr1, double *arr2, double *result, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        result[i] = arr1[i] + arr2[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *data, int n, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix[:n]) if(n > 1000)
    for (int i = 0; i < n; i++) {
        double val = data[i];
        
        /* Data-dependent operation */
        if (val > 0.5) {
            val *= 2.0;
        }
        
        sum += val;
        
        #pragma omp scan inclusive(prefix[i])
        prefix[i] = sum;
    }
    
    return sum;
}

/* Function with nested collapsed loops - may generate _condtemp_ */
void process_matrix(int rows, int cols, double *matrix) {
    int bound = g_volatile_bound;  /* Volatile prevents constant propagation */
    
    /* Collapsed loop with non-trivial bound - may generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(rows * cols > 1000) \
            schedule(dynamic, 4)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Thread-dependent operation */
            if (omp_get_thread_num() % 2 == 0) {
                matrix[idx] *= 1.1;
            } else {
                matrix[idx] *= 0.9;
            }
            
            /* Complex condition that might need temporaries */
            if (i > j && (i + j) % 3 == 0) {
                matrix[idx] += 0.01 * (i - j);
            }
        }
    }
}

/* Target region with declare target enter */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target teams distribute parallel for \
            map(to: a[0:n], b[0:n]) \
            map(from: c[0:n]) \
            reduction(+:a[0:n]) \
            if(n > 500)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        a[i] += 0.5;  /* Modification in reduction */
    }
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
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
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    double *result = (double *)malloc(size2 * sizeof(double));
    
    if (!data1 || !data2 || !prefix || !matrix || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        data1[i] = (double)(i % 100) / 100.0;
    }
    
    for (int i = 0; i < size2; i++) {
        data2[i] = (double)(i % 50) / 50.0;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (double)(i % 10) / 10.0;
    }
    
    /* 1. Test reduction and scan - should generate _reductemp_ and _scantemp_ */
    printf("Computing prefix sum...\n");
    double total = compute_prefix_sum(data1, size1, prefix);
    printf("Total sum: %f\n", total);
    
    /* 2. Test collapsed loops - may generate _condtemp_ */
    printf("Processing matrix...\n");
    process_matrix(rows, cols, matrix);
    
    /* 3. Test declare target enter with to clause */
    printf("Running target computation...\n");
    
    /* Update volatile bound to affect loop conditions */
    g_volatile_bound = rows / 2;
    
    /* Call the function that was declared with enter */
    #pragma omp target data map(tofrom: data2[0:size2]) \
                            map(to: result[0:size2])
    {
        add_vectors(data2, data2, result, size2);
    }
    
    /* 4. Another target region with complex clauses */
    target_computation(data1, prefix, result, size1 < size2 ? size1 : size2);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < size1 && i < 100; i++) {
        checksum += data1[i] + prefix[i];
    }
    
    for (int i = 0; i < rows * cols && i < 100; i++) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(prefix);
    free(matrix);
    free(result);
    
    return 0;
}
