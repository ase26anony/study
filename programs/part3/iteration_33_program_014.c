/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force runtime values to prevent optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(array_a, array_b, array_c)
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
            simdlen(4) safelen(8) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = data[i] + (i % (seed + 1)) * 0.1;
        
        /* Prefix scan phase */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        total += val;
        
        /* Conditional operation that might need condition temporaries */
        if (prefix_sum > 100.0) {
            data[i] = prefix_sum / (i + 1);
        }
    }
    
    return total;
}

/* Nested loop with collapse - may generate _condtemp_ */
void process_matrix(int rows, int cols, double *matrix) {
    int bound = g_volatile_bound;
    
    /* Complex loop bound to force condition temporary */
    #pragma omp parallel for collapse(2) \
            if(rows * cols > 1000) num_threads(4)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols && j < bound; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation */
            if ((i + j) % 2 == 0) {
                matrix[idx] *= 1.1;
            } else {
                matrix[idx] *= 0.9;
            }
            
            /* Thread-specific adjustment */
            if (omp_get_thread_num() % 3 == 0) {
                matrix[idx] += 0.01;
            }
        }
    }
}

/* Target region with data mapping */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target map(tofrom: a[0:n], b[0:n]) map(from: c[0:n]) \
            device(0) if(n > 500)
    {
        #pragma omp teams distribute parallel for simd \
                reduction(+:a[:n]) num_teams(2)
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * b[i];
            c[i] = a[i] + b[i];
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for reproducible but variable sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 500 + (rand() % 100);
    int size2 = 300 + (rand() % 50);
    int rows = 50 + (seed % 30);
    int cols = 40 + (seed % 20);
    
    printf("Running with seed=%d, sizes: %d, %d, matrix: %dx%d\n", 
           seed, size1, size2, rows, cols);
    
    /* Allocate arrays with runtime sizes */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size2 * sizeof(double));
    double *array3 = (double *)malloc(size1 * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    
    if (!array1 || !array2 || !array3 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 0.5 + (i % 7);
    }
    for (int i = 0; i < size2; i++) {
        array2[i] = i * 0.3 + (i % 5);
    }
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (i % 100) * 0.1;
    }
    
    /* SECTION 1: Test reduction and scan (for _reductemp_ and _scantemp_) */
    printf("Computing prefix sum...\n");
    double total1 = compute_prefix_sum(array1, size1, seed);
    
    /* SECTION 2: Test nested collapse (for _condtemp_) */
    printf("Processing matrix...\n");
    process_matrix(rows, cols, matrix);
    
    /* SECTION 3: Test declare target enter with to clause */
    printf("Performing target computation...\n");
    
    /* First update the data to target */
    #pragma omp target update to(array1[0:size1], array2[0:size2])
    
    /* Call the function that was entered via declare target */
    #pragma omp target teams distribute parallel for \
            map(to: array1[0:size1], array2[0:size2]) \
            map(from: array3[0:size1]) \
            num_teams(2) thread_limit(64)
    for (int i = 0; i < size1; i++) {
        if (i < size2) {
            array3[i] = array1[i] + array2[i];
        } else {
            array3[i] = array1[i];
        }
    }
    
    /* SECTION 4: Combined construct with multiple clauses */
    printf("Running combined construct...\n");
    {
        double sum = 0.0;
        double scan_sum = 0.0;
        
        #pragma omp parallel for simd \
                reduction(+:sum) \
                simdlen(4) linear(i:1) \
                scan(inscan:scan_sum) \
                if(size1 > 200)
        for (int i = 0; i < size1; i++) {
            double val = array1[i] * 0.01;
            #pragma omp scan exclusive(scan_sum)
            array1[i] = scan_sum;
            scan_sum += val;
            sum += val;
            
            /* Complex condition that might generate temporaries */
            if ((i * seed) % 13 == 0 && scan_sum > sum) {
                array1[i] += 1.0;
            }
        }
        printf("Combined sum: %f\n", sum);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 100; i++) {
        checksum += array1[i] + array3[i];
    }
    for (int i = 0; i < rows * cols && i < 100; i++) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Total from prefix: %f\n", total1);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(matrix);
    
    return 0;
}
