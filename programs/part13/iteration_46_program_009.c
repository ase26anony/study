/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare analysis */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void process_data(int N, int M, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = N;
    volatile int dyn_M = M;
    int actual_N = dyn_N;
    int actual_M = dyn_M;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_matrix[256][256];
    
    /* Reduction variables with different scopes */
    int total_sum = 0;
    int row_sums[512] = {0};
    int partial_sums[32] = {0};
    
    /* Initialize source data */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = (i * 3 + j * 7) % 97;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:actual_N][0:actual_M], \
                         dst[0:actual_N][0:actual_M], \
                         total_sum) \
                   copyin(row_sums[0:actual_N]) \
                   create(tmp_matrix[0:actual_N/2][0:actual_M/2])
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_matrix) firstprivate(actual_N, actual_M) \
                reduction(+:total_sum) \
                async(1)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Conditional data access pattern */
                int tmp;
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with private variable */
                    tmp = src[i-1][j] + src[i+1][j] + 
                          src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                } else if ((i * j) % 5 == 0) {
                    /* Different access pattern */
                    tmp = src[i][j] * 2;
                    dst[i][j] = tmp - src[i][j];
                } else {
                    tmp = src[i][j];
                    dst[i][j] = tmp;
                }
                
                /* Reduction with conditional */
                if (tmp > 50) {
                    total_sum += tmp;
                }
                
                /* Access to smaller private matrix */
                int local_i = i % (actual_N/2);
                int local_j = j % (actual_M/2);
                tmp_matrix[local_i][local_j] = tmp % 100;
            }
        }
        
        /* Wait for first kernel */
        #pragma acc wait(1)
        
        /* SECOND OPENACC REGION: Different partitioning - gang+vector only */
        #pragma acc kernels loop gang vector collapse(2) \
                private(partial_sums) \
                copy(row_sums[0:actual_N])
        for (int i = 0; i < actual_N; i++) {
            for (int j = 0; j < actual_M; j++) {
                /* Row-wise reduction with gang partitioning */
                row_sums[i] += dst[i][j];
                
                /* Worker-like partitioning simulated with manual decomposition */
                int worker_id = j % 32;
                partial_sums[worker_id] += dst[i][j] % 17;
            }
        }
        
        /* THIRD REGION: Access declared device data with vector partitioning */
        #pragma acc parallel loop vector \
                copy(global_sum) \
                present(global_matrix)
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                /* Conditional update of global matrix */
                if (i > j) {
                    global_matrix[i][j] = src[i % actual_N][j % actual_M];
                } else {
                    global_matrix[i][j] = dst[i % actual_N][j % actual_M];
                }
                
                /* Atomic operation for global reduction */
                #pragma acc atomic update
                global_sum += global_matrix[i][j] % 23;
            }
        }
        
        /* FOURTH REGION: Fully partitioned scenario with all levels */
        #pragma acc parallel loop gang worker vector collapse(2) \
                firstprivate(actual_N, actual_M) \
                reduction(+:total_sum)
        for (int i = 0; i < actual_N; i += 2) {
            for (int j = 0; j < actual_M; j += 2) {
                /* Block processing - each gang/worker/vector handles a block */
                int block_sum = 0;
                #pragma acc loop seq
                for (int bi = 0; bi < 2 && i+bi < actual_N; bi++) {
                    #pragma acc loop seq
                    for (int bj = 0; bj < 2 && j+bj < actual_M; bj++) {
                        block_sum += dst[i+bi][j+bj];
                    }
                }
                total_sum += block_sum;
            }
        }
    }
    
    /* Host-side OpenMP region mixing with OpenACC */
    #pragma omp parallel for reduction(+:total_sum)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            /* Cross-verify OpenACC results */
            if (dst[i][j] != src[i][j]) {
                total_sum += (dst[i][j] - src[i][j]);
            }
        }
    }
    
    /* Use argc to create control flow diversity */
    if (argc > 2) {
        /* Additional OpenACC region with different parameters */
        int extra_sum = 0;
        #pragma acc parallel loop reduction(+:extra_sum) \
                copy(extra_sum) num_gangs(8) num_workers(4) vector_length(32)
        for (int i = 0; i < 1000; i++) {
            extra_sum += i % 19;
        }
        total_sum += extra_sum;
    }
}

int main(int argc, char **argv) {
    /* Dynamic problem size based on arguments */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    /* Initialize global matrix on device */
    #pragma acc parallel loop collapse(2) copy(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 101;
        }
    }
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, argc);
    
    /* Final verification output */
    printf("Processing complete. N=%d, M=%d\n", N, M);
    printf("Global sum: %d\n", global_sum);
    
    /* Force device synchronization */
    #pragma acc wait
    
    return 0;
}
