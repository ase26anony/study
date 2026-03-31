/* tree-pretty-print-coverage.c
 * Targets uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -foffload=disable -o coverage_test tree-pretty-print-coverage.c
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
void process_matrix(int rows, int cols, double *matrix) {
    volatile int v_rows = rows;  /* Prevent optimization */
    volatile int v_cols = cols;
    
    /* This may generate _condtemp_ for collapse bounds */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < v_rows; i++) {
        for (int j = 0; j < v_cols; j++) {
            int idx = i * cols + j;
            if ((i + j) % 2 == 0) {
                matrix[idx] *= 2.0;
            } else {
                matrix[idx] /= 2.0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent optimization */
    int n = 1000 + (rand() % 1000);
    int rows = 50 + (rand() % 50);
    int cols = 50 + (rand() % 50);
    
    printf("Testing with n=%d, rows=%d, cols=%d\n", n, rows, cols);
    
    /* Allocate arrays */
    double *arr1 = (double *)malloc(n * sizeof(double));
    double *arr2 = (double *)malloc(n * sizeof(double));
    double *result = (double *)malloc(n * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    
    if (!arr1 || !arr2 || !result || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 0.5;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (double)(i % 100);
    }
    
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* SECTION 1: Reduction and Scan - targets _reductemp_ and _scantemp_ */
    printf("Running reduction with scan...\n");
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = arr1[i] + arr2[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        sum += val;
        
        /* Data-dependent operation */
        if (omp_get_thread_num() % 2 == 0) {
            result[i] = val * 1.1;
        } else {
            result[i] = val * 0.9;
        }
    }
    
    /* SECTION 2: Nested collapse - may generate _condtemp_ */
    printf("Processing matrix with collapse...\n");
    process_matrix(rows, cols, matrix);
    
    /* SECTION 3: Target region with entered function */
    printf("Running target region...\n");
    #pragma omp target map(to: n) map(tofrom: arr1[0:n], arr2[0:n], result[0:n])
    {
        /* Call the function that was entered via declare target */
        vec_add(n, arr1, arr2, result);
    }
    
    /* SECTION 4: Additional complex reduction in teams */
    printf("Running teams reduction...\n");
    double team_sum = 0.0;
    #pragma omp target teams distribute parallel for \
            reduction(+:team_sum) map(to: matrix[0:rows*cols]) \
            map(tofrom: team_sum)
    for (int i = 0; i < rows * cols; i++) {
        team_sum += matrix[i];
        
        /* Complex conditional to generate more temporaries */
        if (matrix[i] > 50.0 && omp_get_team_num() % 2 == 0) {
            matrix[i] -= 10.0;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < n; i += 10) {
        checksum += result[i];
    }
    for (int i = 0; i < rows * cols; i += 20) {
        checksum += matrix[i];
    }
    
    printf("Results: sum=%.2f, prefix_sum=%.2f, team_sum=%.2f, checksum=%.2f\n",
           sum, prefix_sum, team_sum, checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(result);
    free(matrix);
    
    return 0;
}
