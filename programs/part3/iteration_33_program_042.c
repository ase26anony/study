/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -foffload=disable
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(map_to_from:arr1, arr2, result)
void vec_add(int n, double *a, double *b, double *c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function for nested collapse */
void process_matrix(int rows, int cols, double *matrix, volatile int bound) {
    /* Use volatile to prevent optimization of loop bounds */
    int limit = bound;
    
    /* This may generate _condtemp_ for collapse clause */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols && j < limit; j++) {
            matrix[i * cols + j] *= (i + j + 1);
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Create runtime-dependent sizes to prevent optimization */
    int n = 100 + (rand() % 100);
    int m = 50 + (rand() % 50);
    int p = 30 + (rand() % 30);
    
    /* Allocate arrays */
    double *arr1 = (double *)malloc(n * sizeof(double));
    double *arr2 = (double *)malloc(n * sizeof(double));
    double *result = (double *)malloc(n * sizeof(double));
    double *matrix = (double *)malloc(m * p * sizeof(double));
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < n; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = i * 0.3;
    }
    
    for (int i = 0; i < m * p; i++) {
        matrix[i] = (i % 10) * 0.1;
    }
    
    /* SECTION 1: Generate _reductemp_ and _scantemp_ clauses */
    /* Use reduction with scan to generate both temporary types */
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = arr1[i] + arr2[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        sum += val;
        
        /* Data-dependent operation */
        if (i % (omp_get_thread_num() + 2) == 0) {
            result[i] = prefix_sum;
        } else {
            result[i] = val;
        }
    }
    
    /* SECTION 2: Generate _condtemp_ clause */
    /* Use volatile variable for loop bound to create condition temporaries */
    volatile int dynamic_bound = p - (rand() % 10);
    process_matrix(m, p, matrix, dynamic_bound);
    
    /* SECTION 3: Trigger ENTER clause with to() */
    /* The declare target directive above should generate ENTER clause */
    /* Now use target region with the entered function */
    #pragma omp target map(tofrom: arr1[0:n], arr2[0:n], result[0:n])
    {
        vec_add(n, arr1, arr2, result);
    }
    
    /* SECTION 4: Complex nested construct with multiple clauses */
    /* Combined teams distribute parallel for with reduction */
    double final_sum = 0.0;
    
    #pragma omp target teams distribute parallel for \
            reduction(+:final_sum) map(tofrom: final_sum) \
            map(to: matrix[0:m*p])
    for (int idx = 0; idx < m * p; idx++) {
        /* Complex operation that might generate internal temporaries */
        double temp = matrix[idx];
        for (int k = 0; k < 3; k++) {
            temp = temp * 0.9 + 0.1;
        }
        final_sum += temp;
        
        /* Thread-dependent operation */
        if (omp_get_team_num() % 2 == 0) {
            matrix[idx] = temp;
        }
    }
    
    /* SECTION 5: Additional scan directive for more scantemp coverage */
    double scan_array[100];
    for (int i = 0; i < 100; i++) {
        scan_array[i] = i * 0.01;
    }
    
    double scan_sum = 0.0;
    #pragma omp parallel for simd reduction(+:scan_sum) \
            scan(inscan:scan_sum)
    for (int i = 0; i < 100; i++) {
        #pragma omp scan exclusive(scan_sum)
        double old = scan_sum;
        scan_sum += scan_array[i];
        scan_array[i] = old;
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < n; i += 10) {
        checksum += result[i];
    }
    for (int i = 0; i < m * p; i += 15) {
        checksum += matrix[i];
    }
    
    checksum += sum + final_sum + scan_sum;
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(result);
    free(matrix);
    
    return 0;
}
