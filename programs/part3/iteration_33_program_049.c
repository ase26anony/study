/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be entered via declare target */
#pragma omp declare target enter(add_vectors) to(p_device)
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
            linear(prefix_sum) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = data[i] * (seed + i % 7);
        
        /* Inscan phase */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        total += val;
        
        /* Conditional operation based on thread ID - may generate _condtemp_ */
        if (omp_get_thread_num() % 3 == 0) {
            data[i] = prefix_sum * 0.5;
        } else {
            data[i] = prefix_sum;
        }
    }
    
    return total;
}

/* Nested loops with collapse - may generate _condtemp_ for loop control */
void process_matrix(double **matrix, int rows, int cols, int seed) {
    int bound = g_volatile_bound + seed % 50; /* Non-trivial bound */
    
    #pragma omp parallel for collapse(2) \
            schedule(dynamic, 4) \
            firstprivate(seed)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols && j < bound; j++) {
            /* Complex condition that might need temporaries */
            matrix[i][j] = (i * seed + j) / (1.0 + (i % 10));
            
            /* Data-dependent control flow */
            if ((i * j + seed) % 100 > 50) {
                matrix[i][j] *= 2.0;
            }
        }
    }
}

/* Target region with device memory */
void target_computation(double *h_a, double *h_b, double *h_c, int n) {
    double *p_device = NULL;
    
    /* Allocate device memory */
    #pragma omp target enter data map(to: h_a[0:n], h_b[0:n]) \
            map(alloc: p_device[0:n])
    
    /* Call the entered function on device */
    #pragma omp target teams distribute parallel for \
            is_device_ptr(p_device)
    for (int i = 0; i < n; i++) {
        p_device[i] = h_a[i] + h_b[i];
    }
    
    /* Update host with results */
    #pragma omp target update from(p_device[0:n])
    
    /* Copy back to host array */
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        h_c[i] = p_device[i];
    }
    
    #pragma omp target exit data map(delete: h_a[0:n], h_b[0:n], p_device[0:n])
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int n1 = 500 + (seed % 100);
    int n2 = 300 + (seed % 50);
    int rows = 50 + (seed % 30);
    int cols = 40 + (seed % 20);
    
    printf("Starting OpenMP coverage test with seed=%d\n", seed);
    
    /* Allocate and initialize arrays */
    double *data1 = (double *)malloc(n1 * sizeof(double));
    double *data2 = (double *)malloc(n2 * sizeof(double));
    double *data3 = (double *)malloc(n2 * sizeof(double));
    double **matrix = (double **)malloc(rows * sizeof(double *));
    
    for (int i = 0; i < rows; i++) {
        matrix[i] = (double *)malloc(cols * sizeof(double));
    }
    
    /* Initialize with simple patterns */
    for (int i = 0; i < n1; i++) {
        data1[i] = (i + seed) * 0.1;
    }
    for (int i = 0; i < n2; i++) {
        data2[i] = (i * 2 + seed) * 0.05;
        data3[i] = 0.0;
    }
    
    /* Test 1: Reduction and scan (for _reductemp_ and _scantemp_) */
    printf("Test 1: Computing prefix sum with reduction and scan...\n");
    double total1 = compute_prefix_sum(data1, n1, seed);
    printf("  Total from prefix sum: %f\n", total1);
    
    /* Test 2: Nested loops with collapse (for _condtemp_) */
    printf("Test 2: Processing matrix with nested loops...\n");
    process_matrix(matrix, rows, cols, seed);
    
    /* Test 3: Target region with declare target enter (for ENTER clause) */
    printf("Test 3: Target computation with declare target...\n");
    
    /* Explicit declare target with enter and to clauses */
    #pragma omp declare target enter(add_vectors) \
            to(data2, data3) depend(inout: data2, data3)
    
    /* Call the entered function */
    add_vectors(data2, data2, data3, n2);
    
    /* Also test target region */
    target_computation(data2, data2, data3, n2);
    
    /* Test 4: Combined parallel for simd with multiple clauses */
    printf("Test 4: Combined construct testing...\n");
    double checksum = 0.0;
    volatile int dynamic_bound = n1 / 2 + seed % 10;
    
    #pragma omp parallel for simd reduction(+:checksum) \
            schedule(nonmonotonic:dynamic) \
            lastprivate(dynamic_bound) \
            linear(i:1)
    for (int i = 0; i < n1 && i < dynamic_bound; i++) {
        checksum += data1[i];
        if (i % 7 == (seed % 7)) {
            checksum *= 1.01;
        }
    }
    
    /* Compute final verification checksum */
    double final_checksum = total1 + checksum;
    for (int i = 0; i < rows && i < 10; i++) {
        for (int j = 0; j < cols && j < 10; j++) {
            final_checksum += matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
