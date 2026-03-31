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
    int tmp_array[512][512];
    int row_sums[512] = {0};
    int total_sum = 0;
    int partial_sums[16] = {0};
    
    /* Initialize arrays */
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = i * actual_M + j;
            dst[i][j] = 0;
            tmp_array[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:actual_N][0:actual_M], \
                          dst[0:actual_N][0:actual_M], \
                          tmp_array, row_sums, total_sum, partial_sums)
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_array) reduction(+:total_sum) \
                firstprivate(actual_N, actual_M)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Conditional data access pattern */
                int tmp;
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with conditional */
                    tmp = src[i-1][j] + src[i+1][j] + 
                          src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                } else if ((i + j) % 3 == 1) {
                    /* Different access pattern */
                    tmp = src[i][j] * 2;
                    dst[i][j] = tmp - src[i][j];
                } else {
                    /* Yet another pattern */
                    tmp = src[i][j] + src[j][i];
                    dst[i][j] = tmp / 2;
                }
                
                /* Reduction with private variable */
                total_sum += tmp;
                
                /* Access to private array */
                tmp_array[i][j] = tmp % 100;
            }
        }
        
        /* SECOND OPENACC REGION: Different partitioning - gang+vector only */
        #pragma acc kernels loop gang vector independent \
                private(row_sums) firstprivate(actual_N, actual_M)
        for (int i = 0; i < actual_N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < actual_M; j++) {
                /* Conditional update based on computed values */
                if (dst[i][j] > 100) {
                    row_sum += dst[i][j] % 100;
                } else {
                    row_sum += dst[i][j];
                }
            }
            row_sums[i] = row_sum;
            
            /* Access global device data */
            #pragma acc atomic update
            global_sum += row_sum;
        }
        
        /* THIRD OPENACC REGION: Worker+vector partitioning */
        /* Use argc to create dynamic condition preventing optimization */
        int worker_size = (argc > 2) ? 32 : 16;
        #pragma acc parallel loop worker vector num_workers(worker_size) \
                vector_length(32) reduction(+:partial_sums[:16])
        for (int i = 0; i < actual_N; i++) {
            int worker_id = 0;
            #pragma acc atomic capture
            worker_id = partial_sums[0]++;
            
            int idx = worker_id % 16;
            for (int j = 0; j < actual_M; j++) {
                /* Complex conditional with multiple array accesses */
                if ((i * j) % 7 == idx) {
                    partial_sums[idx] += src[i][j] * dst[i][j];
                }
            }
        }
    }
    
    /* MIXED OpenMP region outside OpenACC data region */
    /* This stresses the compiler's handling of multiple parallel models */
    #pragma omp parallel for schedule(dynamic) \
            private(tmp_array) shared(src, dst)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            /* Host-side computation mixing arrays from device */
            tmp_array[i][j] = src[i][j] + dst[i][j];
        }
    }
    
    /* FOURTH OPENACC REGION: Fully partitioned scenario */
    /* Using global declared data */
    #pragma acc parallel loop gang worker vector collapse(2) \
            firstprivate(actual_N, actual_M)
    for (int i = 0; i < actual_N && i < 256; i++) {
        for (int j = 0; j < actual_M && j < 256; j++) {
            /* Multiple operations on global device data */
            int val = src[i][j] + dst[i][j];
            global_matrix[i][j] = val;
            
            /* Nested conditional with atomic */
            if (val % 2 == 0) {
                #pragma acc atomic update
                global_sum += val;
            }
        }
    }
    
    /* Verification output to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Global sum: %d\n", global_sum);
    
    int final_check = 0;
    for (int i = 0; i < 16; i++) {
        final_check += partial_sums[i];
    }
    printf("Partial sums total: %d\n", final_check);
}

int main(int argc, char **argv) {
    /* Dynamic dimensions based on command line */
    int N = (argc > 1) ? atoi(argv[1]) : 512;
    int M = (argc > 2) ? atoi(argv[2]) : 512;
    
    /* Initialize global device data */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = i * 256 + j;
        }
    }
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, argc);
    
    /* Additional OpenMP region to mix pragmas */
    int host_array[1000];
    #pragma omp parallel for
    for (int i = 0; i < 1000; i++) {
        host_array[i] = i * omp_get_thread_num();
    }
    
    /* Final verification using global device data */
    int host_sum = 0;
    #pragma acc parallel loop reduction(+:host_sum) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            host_sum += global_matrix[i][j] % 100;
        }
    }
    
    printf("Final host sum from device: %d\n", host_sum);
    
    return 0;
}
