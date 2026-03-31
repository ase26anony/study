/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare directive analysis */
#pragma acc declare create(global_matrix)
static int global_matrix[256][256];

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void compute_kernels(int N, int M, int argc) {
    /* Use argc to prevent constant propagation */
    volatile int dyn_size = argc;
    int actual_N = N + (dyn_size % 16);
    
    /* Multi-dimensional arrays with different access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_buffer[512][512];
    int row_sums[512] = {0};
    int total_sum = 0;
    int partial_sums[32] = {0};
    
    /* Initialize arrays */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < M; j++) {
            src[i][j] = (i * M + j) % 97;
            dst[i][j] = 0;
            tmp_buffer[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:actual_N][0:M], dst[0:actual_N][0:M]) \
                     copyin(tmp_buffer[0:actual_N][0:M]) \
                     copyout(row_sums[0:actual_N]) \
                     create(partial_sums[0:32])
    {
        /* FIRST PARALLEL REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_buffer) reduction(+:total_sum)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < M-1; j++) {
                /* Conditional data access to force broadcast analysis */
                int tmp;
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with private variable */
                    tmp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                } else if ((i + j) % 3 == 1) {
                    /* Different access pattern */
                    tmp = src[i][j] * 2 - src[i][j-1];
                    dst[i][j] = tmp;
                } else {
                    /* Yet another pattern */
                    tmp = src[i][j];
                    dst[i][j] = tmp + 1;
                }
                
                /* Reduction with conditional */
                if (tmp > 100) {
                    total_sum += tmp;
                }
                
                /* Access to global declared data */
                global_matrix[i % 256][j % 256] += tmp % 10;
            }
        }
        
        /* SECOND KERNELS REGION: Different partitioning strategy */
        /* gang+vector without worker partitioning */
        #pragma acc kernels loop gang vector collapse(2) \
                firstprivate(actual_N) private(partial_sums)
        for (int i = 0; i < actual_N; i++) {
            for (int j = 0; j < M; j++) {
                /* Worker-like computation without worker clause */
                int idx = (i * 13 + j * 7) % 32;
                #pragma acc atomic
                partial_sums[idx] += src[i][j] + dst[i][j];
                
                /* Conditional store to tmp_buffer */
                if (j % 8 == 0) {
                    tmp_buffer[i][j] = partial_sums[idx] % 1000;
                }
            }
        }
        
        /* THIRD REGION: worker+vector partitioning scenario */
        /* Using nested loops with manual partitioning */
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32)
        {
            #pragma acc loop gang
            for (int block = 0; block < actual_N; block += 64) {
                #pragma acc loop worker vector collapse(2)
                for (int i = block; i < block + 64 && i < actual_N; i++) {
                    for (int j = 0; j < M; j++) {
                        /* Complex reduction pattern */
                        int local_sum = 0;
                        #pragma acc loop vector reduction(+:local_sum)
                        for (int k = 0; k < 4; k++) {
                            local_sum += src[i][(j + k) % M];
                        }
                        row_sums[i] += local_sum;
                    }
                }
            }
        }
        
        /* FOURTH REGION: Fully partitioned scenario with reduction array */
        #pragma acc parallel loop gang worker vector collapse(2) \
                reduction(+:partial_sums[0:32])
        for (int i = 0; i < actual_N; i++) {
            for (int j = 0; j < M; j++) {
                int idx = (i + j * 7) % 32;
                partial_sums[idx] += (src[i][j] * 2 - dst[i][j]);
            }
        }
    }
    
    /* Host-side OpenMP region to mix pragma types */
    int host_array[1000];
    #pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < 1000; i++) {
        host_array[i] = i * i;
        /* Access global matrix from host */
        if (i < 256) {
            global_matrix[i][0] += host_array[i] % 100;
        }
    }
    
    /* Verification output */
    printf("Total sum: %d\n", total_sum);
    printf("Row sums[0], [100], [200]: %d, %d, %d\n", 
           row_sums[0], row_sums[100], row_sums[200]);
    
    int final_check = 0;
    for (int i = 0; i < 32; i++) {
        final_check += partial_sums[i];
    }
    printf("Partial sums total: %d\n", final_check);
}

/* Second function with different partitioning patterns */
void nested_partitioning(int size) {
    int matrix[128][128];
    int result[128][128];
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = (i * 17 + j * 13) % 89;
        }
    }
    
    /* Gang redundant partitioning scenario */
    #pragma acc parallel loop gang collapse(2) copy(matrix, result)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Each gang works on entire rows */
            int sum = 0;
            #pragma acc loop worker vector reduction(+:sum)
            for (int k = 0; k < size; k++) {
                sum += matrix[i][k] * matrix[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    /* Worker partitioned scenario */
    int worker_results[16] = {0};
    #pragma acc parallel num_gangs(2) copyin(matrix) copyout(worker_results)
    {
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            #pragma acc loop worker reduction(+:worker_results[0:16])
            for (int w = 0; w < 8; w++) {
                int worker_id = g * 8 + w;
                int local_sum = 0;
                #pragma acc loop vector
                for (int i = 0; i < 64; i++) {
                    for (int j = 0; j < 64; j++) {
                        local_sum += matrix[i + g * 64][j + w * 8];
                    }
                }
                worker_results[worker_id] = local_sum;
            }
        }
    }
    
    printf("Worker results[0], [8], [15]: %d, %d, %d\n",
           worker_results[0], worker_results[8], worker_results[15]);
}

int main(int argc, char **argv) {
    /* Dynamic bounds based on arguments to prevent constant folding */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    
    /* Initialize global matrix */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 50;
        }
    }
    
    printf("Starting OpenACC partitioning tests...\n");
    printf("Matrix size: %d x %d\n", N, M);
    
    /* Call functions with complex partitioning scenarios */
    compute_kernels(N, M, argc);
    nested_partitioning(128);
    
    /* Final mixed OpenMP/OpenACC region */
    int final_matrix[64][64];
    #pragma omp parallel
    {
        #pragma omp for collapse(2)
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                final_matrix[i][j] = 0;
            }
        }
        
        #pragma omp single
        {
            #pragma acc parallel loop gang vector collapse(2) \
                    copyin(global_matrix) copy(final_matrix)
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    /* Access global matrix from device */
                    final_matrix[i][j] = global_matrix[i*4][j*4] * 2;
                }
            }
        }
    }
    
    /* Verification sum */
    int check = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            check += final_matrix[i][j];
        }
    }
    printf("Final checksum: %d\n", check);
    
    return 0;
}
