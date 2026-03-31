/* Complex OpenACC program designed to trigger execution of the partitioning
   type string lookup function in omp-oacc-neuter-broadcast.cc (lines 335-343).
   This program uses multiple OpenACC constructs with explicit data clauses
   and partitioning scenarios to force the compiler to analyze and represent
   various data partitioning strategies. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to test declare directive */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with complex nested loops and mixed OpenACC/OpenMP */
void process_data(int N, int M, int argc) {
    /* Use argc to prevent constant folding */
    volatile int dynamic_bound = argc;
    int actual_N = N + (dynamic_bound % 2);
    int actual_M = M + (dynamic_bound % 3);
    
    /* Multi-dimensional arrays */
    int src[512][512];
    int dst[512][512];
    int tmp_2d[256][256];
    
    /* Reduction variables */
    int total_sum = 0;
    int row_sums[512] = {0};
    int col_sums[512] = {0};
    
    /* Initialize arrays */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = i * actual_M + j;
            dst[i][j] = 0;
            if (i < 256 && j < 256) {
                tmp_2d[i][j] = (i + j) % 7;
            }
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:actual_N][0:actual_M], \
                         dst[0:actual_N][0:actual_M], \
                         tmp_2d) \
                     copyin(row_sums[0:actual_N]) \
                     copy(total_sum)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning */
        /* This region uses gang+worker+vector with private and reduction
           to potentially trigger multiple partitioning scenarios */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_2d) reduction(+:total_sum) \
                firstprivate(actual_N, actual_M)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Conditional data access pattern */
                int temp = 0;
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with conditional */
                    temp = src[i-1][j] + src[i+1][j] + 
                           src[i][j-1] + src[i][j+1];
                } else if ((i + j) % 3 == 1) {
                    temp = src[i][j] * 2;
                } else {
                    temp = src[i][j] / 2;
                }
                
                /* More complex conditional with nested if */
                if (i % 2 == 0) {
                    dst[i][j] = temp + src[i][j];
                    if (j % 3 == 0) {
                        total_sum += dst[i][j];
                    }
                } else {
                    dst[i][j] = temp - src[i][j];
                }
                
                /* Access to global declared data */
                if (i < 256 && j < 256) {
                    global_matrix[i][j] += temp % 100;
                }
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning */
        /* Uses only gang+vector partitioning (no worker) */
        #pragma acc kernels loop gang vector collapse(2) \
                private(row_sums, col_sums)
        for (int i = 0; i < actual_N; i++) {
            for (int j = 0; j < actual_M; j++) {
                /* Row-wise and column-wise reductions */
                row_sums[i] += dst[i][j];
                col_sums[j] += dst[i][j];
                
                /* Another conditional pattern */
                if (dst[i][j] > 1000) {
                    dst[i][j] = 1000;
                } else if (dst[i][j] < -1000) {
                    dst[i][j] = -1000;
                }
            }
        }
        
        /* THIRD: Worker-only partitioning scenario */
        #pragma acc parallel loop worker collapse(2) \
                firstprivate(actual_N, actual_M)
        for (int i = 0; i < actual_N; i += 2) {
            for (int j = 0; j < actual_M; j += 2) {
                /* Block operation */
                int block_sum = 0;
                for (int bi = 0; bi < 2 && i+bi < actual_N; bi++) {
                    for (int bj = 0; bj < 2 && j+bj < actual_M; bj++) {
                        block_sum += dst[i+bi][j+bj];
                    }
                }
                tmp_2d[i/2][j/2] = block_sum;
            }
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    #pragma omp parallel for reduction(+:global_sum)
    for (int i = 0; i < actual_N; i++) {
        int local_sum = 0;
        for (int j = 0; j < actual_M; j++) {
            local_sum += dst[i][j];
        }
        #pragma omp atomic
        global_sum += local_sum;
    }
    
    /* Verification output */
    printf("Total sum from reductions: %d\n", total_sum);
    printf("Global sum from OpenMP: %d\n", global_sum);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < (actual_N < 256 ? actual_N : 256); i++) {
        for (int j = 0; j < (actual_M < 256 ? actual_M : 256); j++) {
            checksum = (checksum + tmp_2d[i][j]) % 1000000;
        }
    }
    printf("Final checksum: %d\n", checksum);
}

int main(int argc, char **argv) {
    /* Dynamic bounds based on arguments to prevent optimization */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    if (N < 64) N = 64;
    if (M < 64) M = 64;
    
    /* Initialize global declared data */
    #pragma acc parallel loop collapse(2)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = i * j;
        }
    }
    
    /* Process data with complex OpenACC patterns */
    process_data(N, M, argc);
    
    /* Additional OpenACC region with vector-only partitioning */
    int vec_array[1024];
    #pragma acc parallel loop vector
    for (int i = 0; i < 1024; i++) {
        vec_array[i] = i * 2;
    }
    
    /* Final mixed OpenMP region */
    #pragma omp parallel for
    for (int i = 0; i < 1024; i++) {
        vec_array[i] += omp_get_thread_num();
    }
    
    return 0;
}
