/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare directive analysis */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void process_data(int N, int M, int use_condition, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = N;
    volatile int dyn_M = M;
    int actual_N = dyn_N;
    int actual_M = dyn_M;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_buffer[512][512];
    int row_sums[512] = {0};
    int total_sum = 0;
    int partial_sums[16] = {0};
    
    /* Initialize source data */
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = (i * 3 + j * 7) % 100;
            dst[i][j] = 0;
            tmp_buffer[i][j] = 0;
        }
    }
    
    /* OpenACC data region for host-device transfers */
    #pragma acc data copy(src, dst) copyin(actual_N, actual_M) \
                create(tmp_buffer, row_sums) copy(total_sum)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario */
        /* This should trigger analysis of gang+worker+vector partitioning */
        #pragma acc parallel loop gang worker vector collapse(2) \
                    private(tmp_buffer) firstprivate(actual_N, actual_M) \
                    reduction(+:total_sum)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Local private variable */
                int temp = 0;
                
                /* Conditional data access pattern - forces different 
                   broadcasting/neutering strategies */
                if (use_condition && (i % 2 == 0 || j % 3 == 0)) {
                    /* Stencil operation with conditional */
                    temp = src[i-1][j] + src[i+1][j] + 
                           src[i][j-1] + src[i][j+1];
                    dst[i][j] = temp / 4;
                    
                    /* Reduction with conditional */
                    if (temp > 50) total_sum += temp;
                } else {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                    dst[i][j] = temp;
                    if (temp < 100) total_sum += temp;
                }
                
                /* Use private array */
                tmp_buffer[i][j] = temp % 10;
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach */
        /* This should trigger gang+vector partitioning analysis */
        #pragma acc kernels loop gang vector independent \
                    private(partial_sums) firstprivate(actual_N, actual_M)
        for (int i = 0; i < actual_N; i++) {
            /* Worker-level private array */
            int worker_private[32] = {0};
            int row_sum = 0;
            
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < actual_M; j++) {
                /* Complex conditional with data-dependent access */
                if ((i + j + argc) % 4 == 0) {
                    row_sum += src[i][j] * 2;
                    worker_private[j % 32] += dst[i][j];
                } else if ((i * j) % 7 == 0) {
                    row_sum += dst[i][j] / 2;
                    worker_private[j % 32] -= src[i][j];
                }
                
                /* Update global matrix (persistent device data) */
                if (i < 256 && j < 256) {
                    global_matrix[i][j] += src[i][j] + dst[i][j];
                }
            }
            
            row_sums[i] = row_sum;
            
            /* Nested reduction into partial_sums */
            int gang_idx = i % 16;
            #pragma acc atomic
            partial_sums[gang_idx] += row_sum;
        }
        
        /* THIRD: Mixed parallel/kernels with explicit gang/worker clauses */
        /* This should trigger worker+vector partitioning analysis */
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32)
        {
            #pragma acc loop gang
            for (int block = 0; block < 8; block++) {
                int block_start = block * (actual_N / 8);
                int block_end = (block + 1) * (actual_N / 8);
                
                #pragma acc loop worker
                for (int i = block_start; i < block_end; i++) {
                    int local_accum = 0;
                    
                    #pragma acc loop vector reduction(+:local_accum)
                    for (int j = 0; j < actual_M; j++) {
                        /* Fully partitioned access pattern */
                        local_accum += tmp_buffer[i][j] * (i + j);
                    }
                    
                    /* Conditional update with atomic */
                    if (local_accum > 0) {
                        #pragma acc atomic
                        global_sum += local_accum;
                    }
                }
            }
        }
    }
    
    /* PURE OPENMP REGION: Mix pragma types to stress compiler */
    /* This may trigger additional internal validation code */
    #pragma omp parallel for schedule(dynamic) private(total_sum)
    for (int i = 0; i < actual_N; i++) {
        int thread_sum = 0;
        for (int j = 0; j < actual_M; j++) {
            thread_sum += dst[i][j];
        }
        /* Use OpenMP atomic to prevent optimization */
        #pragma omp atomic
        total_sum += thread_sum;
    }
    
    /* Verification output to prevent dead code elimination */
    printf("Checksum: total_sum = %d, row_sums[100] = %d, global_sum = %d\n", 
           total_sum, row_sums[100], global_sum);
}

/* Helper function with another OpenACC region */
void helper_function(int size) {
    int array[256][256];
    int result = 0;
    
    #pragma acc parallel loop collapse(2) gang worker \
                reduction(+:result) copy(array) copyout(result)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            array[i][j] = i * j;
            if ((i + j) % 2 == 0) {
                result += array[i][j];
            } else {
                result -= array[i][j] / 2;
            }
        }
    }
    
    printf("Helper result: %d\n", result);
}

int main(int argc, char *argv[]) {
    /* Dynamic bounds based on command line */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    int use_condition = argc > 3 ? atoi(argv[3]) : 1;
    
    /* Initialize global matrix on device */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = 0;
        }
    }
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, use_condition, argc);
    
    /* Call helper to increase coverage */
    helper_function(N % 256);
    
    /* Final OpenACC region with different clause combination */
    int final_array[128][128];
    int final_sum = 0;
    
    #pragma acc parallel loop gang vector collapse(2) \
                private(final_array) reduction(+:final_sum) \
                copyin(N, M)
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            final_array[i][j] = (i * M + j * N) % 97;
            final_sum += final_array[i][j];
            
            /* Access global matrix */
            if (i < 256 && j < 256) {
                final_sum += global_matrix[i][j] % 10;
            }
        }
    }
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
