/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -foffload=disable
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(map_to_from:result)
void add_vectors(double *a, double *b, double *result, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Another function for declare target with to clause */
#pragma omp declare target enter(scale_vector) to(map_to_from:data)
void scale_vector(double *data, double factor, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        data[i] *= factor;
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 100 + (rand() % 100);
    int size2 = 50 + (rand() % 50);
    int rows = 10 + (rand() % 10);
    int cols = 10 + (rand() % 10);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    double *scan_array = (double *)malloc(size2 * sizeof(double));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    
    if (!array1 || !array2 || !result || !scan_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.5;
    }
    
    for (int i = 0; i < size2; i++) {
        scan_array[i] = (i % 3) + 1.0;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = i;
    }
    
    double total_sum = 0.0;
    double prefix_sum = 0.0;
    
    /* 1. OpenMP SIMD with reduction and scan - targets _reductemp_ and _scantemp_ */
    printf("Starting reduction and scan...\n");
    #pragma omp parallel for simd reduction(+:total_sum) \
            scan(inscan:prefix_sum) if(size2 > 50)
    for (int i = 0; i < size2; i++) {
        double val = scan_array[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Data-dependent operation */
        if (prefix_sum > 10.0) {
            val *= 2.0;
        }
        
        total_sum += val;
        scan_array[i] = prefix_sum;
    }
    
    /* 2. Nested loop with collapse - may generate _condtemp_ */
    printf("Starting collapsed loops...\n");
    int local_sum = 0;
    volatile int vol_bound = g_volatile_bound;
    
    #pragma omp parallel for collapse(2) reduction(+:local_sum) \
            schedule(dynamic) num_threads(4)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Complex condition that may require condition temporary */
            int idx = i * cols + j;
            if (idx < vol_bound && idx % (omp_get_thread_num() + 1) == 0) {
                local_sum += matrix[idx];
            }
        }
    }
    
    /* 3. Use declare target enter function */
    printf("Using declare target functions...\n");
    
    /* Call the entered function */
    #pragma omp target map(to: array1[0:size1], array2[0:size1]) \
                       map(from: result[0:size1]) if(size1 > 50)
    {
        add_vectors(array1, array2, result, size1);
    }
    
    /* 4. Another target region with data mapping */
    double factor = 2.0;
    #pragma omp target data map(tofrom: result[0:size1])
    {
        #pragma omp target enter data map(to: result[0:size1])
        
        #pragma omp target map(to: factor) map(tofrom: result[0:size1])
        {
            scale_vector(result, factor, size1);
        }
    }
    
    /* 5. Target update directive */
    #pragma omp target update from(result[0:size1])
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1; i += 10) {
        checksum += result[i];
    }
    checksum += total_sum + local_sum;
    
    printf("Checksum: %f\n", checksum);
    printf("Total sum from reduction: %f\n", total_sum);
    printf("Local sum from collapsed loops: %d\n", local_sum);
    
    /* Conditional compilation for debugging */
    #ifdef DUMP_OMP
    /* Dummy function that hints at OpenMP clause usage */
    void dummy_omp_hint(int clause_type) {
        switch(clause_type) {
            case 0: printf("_reductemp_\n"); break;
            case 1: printf("_condtemp_\n"); break;
            case 2: printf("_scantemp_\n"); break;
            case 3: printf("enter to\n"); break;
        }
    }
    dummy_omp_hint(0);
    #endif
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(scan_array);
    free(matrix);
    
    return 0;
}
