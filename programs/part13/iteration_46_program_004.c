/* test-omp-oacc-neuter-broadcast.c
 * Complex OpenACC program designed to trigger partitioning analysis
 * and potentially execute the uncovered switch statement in
 * omp-oacc-neuter-broadcast.cc lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use acc declare for persistent device data management */
#pragma acc declare create(global_matrix, global_sum)

#define SIZE 256
static int global_matrix[SIZE][SIZE];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void process_data(int argc, char *argv[]) {
    int i, j, k;
    volatile int dynamic_bound; /* Prevent constant folding */
    
    /* Use argc to set dynamic bounds */
    dynamic_bound = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (dynamic_bound > SIZE) dynamic_bound = SIZE;
    
    int src[SIZE][SIZE], dst[SIZE][SIZE];
    int temp_sum = 0, row_sums[SIZE] = {0};
    
    /* Initialize arrays */
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < dynamic_bound; j++) {
            src[i][j] = i * dynamic_bound + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:dynamic_bound][0:dynamic_bound], \
                          dst[0:dynamic_bound][0:dynamic_bound]) \
                     copyin(global_matrix[0:dynamic_bound][0:dynamic_bound]) \
                     copy(temp_sum, row_sums[0:dynamic_bound])
    {
        /* First OpenACC parallel region with complex partitioning */
        /* gang+worker+vector partitioning scenario */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) firstprivate(dynamic_bound) \
                reduction(+:temp_sum) \
                copy(global_sum)
        for (i = 1; i < dynamic_bound-1; i++) {
            for (j = 1; j < dynamic_bound-1; j++) {
                int tmp; /* private variable */
                
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with data-dependent access */
                    tmp = src[i-1][j] + src[i+1][j] + 
                          src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                    
                    /* Access global declared data */
                    #pragma acc atomic update
                    global_matrix[i][j] += dst[i][j];
                } else if ((i + j) % 3 == 1) {
                    /* Different access pattern */
                    tmp = src[i][j] * 2;
                    dst[i][j] = tmp;
                    
                    /* Conditional reduction */
                    if (tmp > 100) {
                        temp_sum += tmp;
                    }
                } else {
                    /* Third access pattern */
                    for (k = 0; k < 3; k++) {
                        dst[i][j] += src[i][j] / (k+1);
                    }
                }
                
                /* Additional reduction on global variable */
                #pragma acc atomic
                global_sum += (i * j) % 7;
            }
        }
        
        /* Second OpenACC kernels region with different partitioning */
        /* gang+vector partitioning (no worker) */
        #pragma acc kernels loop gang vector \
                private(j) firstprivate(dynamic_bound) \
                reduction(+:row_sums[:dynamic_bound])
        for (i = 0; i < dynamic_bound; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < dynamic_bound; j++) {
                /* Conditional update */
                if (dst[i][j] > 0) {
                    row_sum += dst[i][j];
                } else {
                    row_sum -= src[i][j];
                }
            }
            row_sums[i] = row_sum;
            
            /* Update global matrix */
            #pragma acc loop vector
            for (j = 0; j < dynamic_bound; j++) {
                if (j % 2 == 0) {
                    global_matrix[i][j] += row_sum;
                }
            }
        }
        
        /* Third region: worker+vector partitioning scenario */
        #pragma acc parallel loop worker vector collapse(2) \
                firstprivate(dynamic_bound)
        for (i = 0; i < dynamic_bound; i += 2) {
            for (j = 0; j < dynamic_bound; j += 2) {
                /* Block operation */
                int block_sum = 0;
                #pragma acc loop vector reduction(+:block_sum)
                for (int bi = 0; bi < 2 && i+bi < dynamic_bound; bi++) {
                    for (int bj = 0; bj < 2 && j+bj < dynamic_bound; bj++) {
                        block_sum += dst[i+bi][j+bj];
                    }
                }
                /* Store block sum in all elements */
                #pragma acc loop vector
                for (int bi = 0; bi < 2 && i+bi < dynamic_bound; bi++) {
                    for (int bj = 0; bj < 2 && j+bj < dynamic_bound; bj++) {
                        dst[i+bi][j+bj] = block_sum;
                    }
                }
            }
        }
    }
    
    /* Pure OpenMP region outside OpenACC data region */
    /* Mixing pragma types to stress compiler */
    int host_array[SIZE];
    #pragma omp parallel for private(i)
    for (i = 0; i < dynamic_bound; i++) {
        host_array[i] = row_sums[i] * 2;
    }
    
    /* Final verification */
    int final_check = 0;
    for (i = 0; i < dynamic_bound; i++) {
        final_check += host_array[i];
    }
    
    printf("Checksum: %d, Global sum: %d, Final check: %d\n", 
           temp_sum, global_sum, final_check);
}

/* Another function with different partitioning pattern */
void nested_partitioning(int n) {
    int i, j;
    int matrix[n][n];
    int vector[n];
    
    /* Initialize */
    for (i = 0; i < n; i++) {
        vector[i] = i;
        for (j = 0; j < n; j++) {
            matrix[i][j] = i * n + j;
        }
    }
    
    /* Fully partitioned scenario (gang+worker+vector) */
    #pragma acc parallel loop gang worker vector collapse(2) \
            copy(matrix[0:n][0:n], vector[0:n])
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            /* Complex conditional with private variable */
            int private_val = matrix[i][j];
            if (i > j) {
                private_val *= vector[i];
            } else if (i < j) {
                private_val += vector[j];
            } else {
                private_val = 0;
            }
            
            /* Nested conditional */
            if (private_val % 2 == 0) {
                matrix[i][j] = private_val / 2;
            } else {
                matrix[i][j] = private_val * 3 + 1;
            }
        }
    }
    
    /* Worker partitioned scenario */
    int worker_results[n];
    #pragma acc parallel loop worker copy(worker_results[0:n])
    for (i = 0; i < n; i++) {
        int sum = 0;
        #pragma acc loop vector reduction(+:sum)
        for (j = 0; j < n; j++) {
            sum += matrix[i][j];
        }
        worker_results[i] = sum;
    }
    
    printf("Worker results computed for size %d\n", n);
}

int main(int argc, char *argv[]) {
    printf("Starting complex OpenACC partitioning test...\n");
    
    /* Initialize global matrix */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            global_matrix[i][j] = 1;
        }
    }
    
    /* Process data with complex partitioning */
    process_data(argc, argv);
    
    /* Additional test with different size */
    int small_size = (argc > 2) ? atoi(argv[2]) : 64;
    if (small_size > 0 && small_size <= SIZE) {
        nested_partitioning(small_size);
    }
    
    /* Final host-side computation mixing OpenMP */
    int host_final = 0;
    #pragma omp parallel for reduction(+:host_final)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            host_final += global_matrix[i][j];
        }
    }
    
    printf("Final host sum: %d\n", host_final);
    printf("Test completed.\n");
    
    return 0;
}
