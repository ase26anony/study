/* test-omp-oacc-neuter-broadcast.c
 * Complex OpenACC program designed to trigger partitioning analysis
 * and potentially execute the uncovered switch statement in
 * omp-oacc-neuter-broadcast.cc (lines 335-343)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use acc declare for persistent device data management */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void compute_kernels(int N, int M, int use_condition, int argc) {
    int i, j, k;
    volatile int dynamic_bound = N; /* Prevent constant folding */
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp[512];
    int total_sum = 0;
    int partial_sums[16] = {0};
    
    /* Initialize arrays with non-trivial pattern */
    #pragma omp parallel for collapse(2) /* Mix OpenMP with OpenACC */
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = (i * j) % 100;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) \
                     copyin(tmp[0:M]) copy(total_sum)
    {
        /* First parallel region: complex partitioning with gang+worker+vector */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:total_sum) /* Multiple clauses */
        for (i = 1; i < N-1; i++) {
            for (j = 1; j < M-1; j++) {
                int temp = 0; /* private variable */
                
                /* Conditional data access pattern */
                if (use_condition || (i % 2 == 0)) {
                    /* Stencil operation with data dependencies */
                    temp += src[i-1][j] + src[i+1][j] + 
                            src[i][j-1] + src[i][j+1];
                    
                    /* Nested conditional */
                    if (j % 3 == 0) {
                        temp *= 2;
                    }
                    
                    dst[i][j] = temp / 4;
                } else {
                    dst[i][j] = src[i][j];
                }
                
                /* Reduction with complex expression */
                total_sum += dst[i][j] % 7;
            }
        }
        
        /* Second kernels region: different partitioning (gang+vector only) */
        #pragma acc kernels loop gang vector independent
        for (i = 0; i < N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                /* Conditional store with array access */
                if (dst[i][j] > 50) {
                    row_sum += dst[i][j];
                    tmp[j] = dst[i][j]; /* Use copyin array */
                }
            }
            /* Store to global declared array */
            if (i < 256 && M >= 256) {
                global_matrix[i][i] = row_sum;
            }
        }
        
        /* Third region: worker-level partitioning */
        #pragma acc parallel loop gang worker num_workers(4) \
                firstprivate(partial_sums) /* firstprivate clause */
        for (k = 0; k < 16; k++) {
            int worker_sum = 0;
            int start = k * (N/16);
            int end = (k+1) * (N/16);
            
            #pragma acc loop vector reduction(+:worker_sum)
            for (i = start; i < end && i < N; i++) {
                for (j = 0; j < M; j += 4) {
                    worker_sum += src[i][j];
                }
            }
            
            partial_sums[k] = worker_sum;
        }
    }
    
    /* Process partial sums on host (mixed OpenMP) */
    int final_sum = total_sum;
    #pragma omp parallel for reduction(+:final_sum)
    for (k = 0; k < 16; k++) {
        final_sum += partial_sums[k];
    }
    
    /* Update global sum with acc declare variable */
    #pragma acc update host(global_sum)
    global_sum += final_sum;
    #pragma acc update device(global_sum)
    
    printf("Intermediate checksum: %d\n", final_sum);
}

/* Another function with different partitioning pattern */
void nested_partitioning(int size) {
    int i, j;
    int matrix[128][128];
    int vector[128];
    int reduction_var = 0;
    
    /* Initialize */
    for (i = 0; i < 128; i++) {
        for (j = 0; j < 128; j++) {
            matrix[i][j] = (i + j) % 64;
        }
        vector[i] = i;
    }
    
    /* Fully partitioned scenario (all levels) */
    #pragma acc parallel loop collapse(2) gang worker vector \
            private(j) reduction(+:reduction_var) \
            copy(matrix, vector, reduction_var)
    for (i = 0; i < size && i < 128; i++) {
        for (j = 0; j < size && j < 128; j++) {
            /* Complex access pattern */
            matrix[i][j] += vector[i] * vector[j];
            
            /* Multiple conditions */
            if (matrix[i][j] % 2 == 0) {
                reduction_var += matrix[i][j];
            } else if (matrix[i][j] % 3 == 0) {
                reduction_var -= matrix[i][j] / 2;
            }
        }
    }
    
    printf("Nested partitioning result: %d\n", reduction_var);
}

int main(int argc, char **argv) {
    int N, M;
    
    /* Dynamic bounds based on arguments to prevent constant folding */
    N = (argc > 1) ? atoi(argv[1]) : 512;
    M = (argc > 2) ? atoi(argv[2]) : 512;
    
    if (N <= 0) N = 512;
    if (M <= 0) M = 512;
    
    printf("Testing OpenACC partitioning with N=%d, M=%d\n", N, M);
    
    /* Initialize global declared array */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i * 256 + j) % 1000;
        }
    }
    
    /* Call functions with different partitioning scenarios */
    compute_kernels(N, M, argc % 2, argc);
    nested_partitioning(N % 128);
    
    /* Final host OpenMP section */
    int host_array[1000];
    #pragma omp parallel for
    for (int i = 0; i < 1000; i++) {
        host_array[i] = i * i;
    }
    
    /* Final verification */
    #pragma acc update host(global_sum)
    printf("Final global sum: %d\n", global_sum);
    
    return 0;
}
