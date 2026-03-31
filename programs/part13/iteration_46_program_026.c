/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare analysis */
#pragma acc declare create(global_matrix, global_scalar)
static int global_matrix[256][256];
static int global_scalar = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void compute_kernel(int N, int M, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = (argc > 1) ? N : 512;
    volatile int dyn_M = (argc > 2) ? M : 512;
    int actual_N = dyn_N;
    int actual_M = dyn_M;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_buffer[512][512];
    int row_sums[512] = {0};
    int total_sum = 0;
    int partial_sums[8] = {0}; /* For nested reduction */
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = i * actual_M + j;
            dst[i][j] = 0;
            tmp_buffer[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src, dst) copyin(actual_N, actual_M) \
                copyout(row_sums) create(tmp_buffer) \
                copy(total_sum) reduction(+:global_scalar)
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                    private(tmp_buffer) firstprivate(actual_N, actual_M) \
                    reduction(+:total_sum, global_scalar)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Conditional data access to force broadcast analysis */
                int tmp;
                if ((i + j) % 3 == 0) {
                    /* Access with stride pattern */
                    tmp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                } else if ((i + j) % 3 == 1) {
                    /* Different access pattern */
                    tmp = src[i][j] * 2 - src[i-1][j-1];
                } else {
                    /* Yet another pattern */
                    tmp = src[i+1][j+1] / 2 + src[i][j];
                }
                
                /* Conditional store with reduction */
                if (tmp > 100) {
                    dst[i][j] = tmp;
                    total_sum += tmp;
                    global_scalar += 1;
                }
                
                /* Worker-private computation */
                int worker_tmp = 0;
                #pragma acc loop vector reduction(+:worker_tmp)
                for (int k = 0; k < 4; k++) {
                    worker_tmp += src[i][j] % (k+2);
                }
                tmp_buffer[i][j] = worker_tmp;
            }
        }
        
        /* SECOND OPENACC REGION: Different partitioning (gang+vector only) */
        /* This should trigger different partitioning type analysis */
        #pragma acc kernels loop gang vector collapse(2) \
                    private(partial_sums) firstprivate(actual_N, actual_M)
        for (int i = 0; i < actual_N; i++) {
            for (int j = 0; j < actual_M; j++) {
                /* Nested reduction pattern */
                int idx = j % 8;
                partial_sums[idx] += dst[i][j];
                
                /* Conditional update based on global matrix */
                if (global_matrix[i % 256][j % 256] > 0) {
                    dst[i][j] += global_matrix[i % 256][j % 256];
                }
                
                /* Vector partitioned operation */
                dst[i][j] = dst[i][j] % 1024;
            }
            
            /* Row-wise reduction outside inner loop */
            if (i % 16 == 0) {
                int row_sum = 0;
                #pragma acc loop vector reduction(+:row_sum)
                for (int j = 0; j < actual_M; j++) {
                    row_sum += dst[i][j];
                }
                row_sums[i/16] = row_sum;
            }
        }
        
        /* THIRD REGION: Worker-only partitioning scenario */
        #pragma acc parallel loop worker collapse(2) \
                    firstprivate(actual_N, actual_M)
        for (int i = 0; i < actual_N; i += 4) {
            for (int j = 0; j < actual_M; j += 4) {
                /* Block processing - each worker handles 4x4 block */
                int block_sum = 0;
                #pragma acc loop seq
                for (int bi = 0; bi < 4 && i+bi < actual_N; bi++) {
                    #pragma acc loop seq
                    for (int bj = 0; bj < 4 && j+bj < actual_M; bj++) {
                        block_sum += dst[i+bi][j+bj];
                    }
                }
                
                /* Store block sum with conditional */
                if (block_sum > 1000) {
                    #pragma acc atomic update
                    total_sum += block_sum;
                }
            }
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    int host_array[1024];
    #pragma omp parallel for private(host_array)
    for (int i = 0; i < 1024; i++) {
        host_array[i] = i * 2;
        /* Cross-thread pattern to prevent optimization */
        if (i % 2 == 0) {
            host_array[i] += omp_get_thread_num();
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Global scalar: %d\n", global_scalar);
    printf("Row sums[0]: %d\n", row_sums[0]);
    
    /* Verify computation */
    int verify_sum = 0;
    for (int i = 0; i < actual_N && i < 10; i++) {
        for (int j = 0; j < actual_M && j < 10; j++) {
            verify_sum += dst[i][j];
        }
    }
    printf("Verification checksum: %d\n", verify_sum);
}

/* Helper function with additional OpenACC constructs */
void nested_partitioning(int size) {
    int array[256][256];
    int result[256] = {0};
    
    #pragma acc data copy(array, result)
    {
        /* Fully partitioned scenario */
        #pragma acc parallel loop collapse(2) \
                    gang worker vector independent
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                array[i][j] = i * size + j;
                
                /* Nested parallel region inside */
                if (j % 32 == 0) {
                    #pragma acc atomic update
                    result[i % 256] += array[i][j];
                }
            }
        }
        
        /* Gang partitioned only */
        #pragma acc parallel loop gang independent
        for (int i = 0; i < size; i += 16) {
            int local_sum = 0;
            #pragma acc loop worker vector reduction(+:local_sum)
            for (int j = 0; j < size; j++) {
                local_sum += array[i % 256][j];
            }
            result[i/16] = local_sum;
        }
    }
    
    printf("Nested partitioning result[0]: %d\n", result[0]);
}

int main(int argc, char **argv) {
    /* Initialize global matrix */
    #pragma acc parallel loop collapse(2)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 97;
        }
    }
    
    /* Call compute kernel with dynamic sizes */
    compute_kernel(512, 512, argc);
    
    /* Call helper function */
    nested_partitioning(256);
    
    /* Additional OpenMP region to mix pragmas */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            int section_sum = 0;
            #pragma acc parallel loop reduction(+:section_sum)
            for (int i = 0; i < 100; i++) {
                section_sum += i;
            }
            printf("Section 1 sum: %d\n", section_sum);
        }
        
        #pragma omp section
        {
            /* Empty section to create control flow variety */
            volatile int dummy = argc;
            printf("Section 2 argc: %d\n", dummy);
        }
    }
    
    return 0;
}
