/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(array1, array2, result)
void add_vectors(double *a, double *b, double *c, int n) {
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
            scan(inscan:prefix_sum:sum) if(n > 1000)
    for (int i = 0; i < n; i++) {
        double val = data[i];
        
        /* Inclusive scan */
        #pragma omp scan inclusive(prefix_sum)
        {
            sum += val;
            prefix[i] = sum;
        }
        
        /* Conditional operation based on thread ID - may generate _condtemp_ */
        if (omp_get_thread_num() % 2 == 0) {
            prefix[i] *= 1.01;
        }
    }
    
    return sum;
}

/* Function with nested collapsed loops - may generate _condtemp_ */
void process_matrix(double **matrix, int rows, int cols) {
    int i, j;
    
    /* Use volatile variable in loop bound to prevent optimization */
    int bound = volatile_bound;
    
    /* Collapsed loop with non-trivial bound - may generate condition temporaries */
    #pragma omp parallel for collapse(2) private(i, j) \
            if(rows * cols > 500) num_threads(4)
    for (i = 0; i < rows && i < bound; i++) {
        for (j = 0; j < cols; j++) {
            /* Complex condition that might require temporaries */
            if ((i * cols + j) % (omp_get_thread_num() + 1) == 0) {
                matrix[i][j] *= 2.0;
            }
        }
    }
}

/* Function with target region using entered function */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            device(0) if(n > 100)
    {
        add_vectors(a, b, c, n);
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Create variable sizes to prevent optimization */
    int size1 = 1000 + (rand() % 1000);
    int size2 = 500 + (rand() % 500);
    
    printf("Running OpenMP coverage test with sizes: %d, %d\n", size1, size2);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    double *prefix = (double *)malloc(size1 * sizeof(double));
    
    /* Allocate matrix */
    double **matrix = (double **)malloc(size2 * sizeof(double *));
    for (int i = 0; i < size2; i++) {
        matrix[i] = (double *)malloc(size2 * sizeof(double));
    }
    
    /* Initialize data with simple patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.5;
    }
    
    for (int i = 0; i < size2; i++) {
        for (int j = 0; j < size2; j++) {
            matrix[i][j] = i * size2 + j;
        }
    }
    
    /* Execute OpenMP constructs to generate internal clauses */
    
    /* 1. Test reduction and scan - should generate _reductemp_ and _scantemp_ */
    printf("Computing prefix sum...\n");
    double total_sum = compute_prefix_sum(array1, size1, prefix);
    
    /* 2. Test collapsed loops - may generate _condtemp_ */
    printf("Processing matrix...\n");
    process_matrix(matrix, size2, size2);
    
    /* 3. Test declare target enter with to clause */
    printf("Running target computation...\n");
    target_computation(array1, array2, result, size1);
    
    /* 4. Additional test with teams distribute */
    {
        int teams_size = 200 + (rand() % 300);
        double *team_array = (double *)malloc(teams_size * sizeof(double));
        
        for (int i = 0; i < teams_size; i++) {
            team_array[i] = i * 0.1;
        }
        
        double team_sum = 0.0;
        #pragma omp target teams distribute parallel for \
                reduction(+:team_sum) map(to: team_array[0:teams_size]) \
                map(tofrom: team_sum) num_teams(4) thread_limit(128)
        for (int i = 0; i < teams_size; i++) {
            team_sum += team_array[i] * (i % 10);
        }
        
        printf("Teams reduction sum: %f\n", team_sum);
        free(team_array);
    }
    
    /* 5. Test with taskloop reduction */
    {
        double task_sum = 0.0;
        #pragma omp parallel master
        {
            #pragma omp taskloop reduction(+:task_sum) grainsize(50) \
                    if(size1 > 800)
            for (int i = 0; i < size1; i++) {
                task_sum += prefix[i] * 0.01;
            }
        }
        printf("Taskloop sum: %f\n", task_sum);
    }
    
    /* Compute checksum to verify execution and prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 100; i++) {
        checksum += result[i] + prefix[i];
    }
    
    for (int i = 0; i < size2 && i < 50; i++) {
        checksum += matrix[i][i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Total prefix sum: %f\n", total_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(prefix);
    
    for (int i = 0; i < size2; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This won't directly trigger the pretty-printer but helps keep
 * OpenMP structures in the IR */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* References to force inclusion of OpenMP constructs */
    asm volatile("" : : "r"(&omp_get_thread_num));
}
#endif
