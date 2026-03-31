/* test-omp-oacc-neuter-broadcast.c
 * Designed to trigger partitioning analysis in GCC's omp-oacc-neuter-broadcast.cc
 * Specifically targets the switch statement mapping integer codes to partitioning descriptions
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare analysis */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with complex OpenACC compute regions */
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
    int total_sum = 0;
    int row_sums[512] = {0};
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = i * actual_M + j;
            dst[i][j] = 0;
            tmp_array[i][j] = 0;
        }
    }
    
    /* OpenACC data region for host-device transfers */
    #pragma acc data copy(src[0:actual_N][0:actual_M], \
                          dst[0:actual_N][0:actual_M], \
                          total_sum) \
                   copyin(tmp_array[0:actual_N][0:actual_M])
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario */
        /* This should trigger analysis for multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_array) reduction(+:total_sum) \
                firstprivate(actual_N, actual_M) \
                async(1)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Conditional data access to force broadcasting analysis */
                int temp = 0;
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with conditional */
                    temp = src[i-1][j] + src[i+1][j] + 
                           src[i][j-1] + src[i][j+1];
                    dst[i][j] = temp / 4;
                } else if ((i + j) % 3 == 1) {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                    dst[i][j] = temp;
                } else {
                    /* Yet another pattern */
                    temp = src[i][j] + src[i][actual_M-1-j];
                    dst[i][j] = temp / 2;
                }
                
                /* Reduction with conditional */
                if (temp > 100) {
                    total_sum += temp;
                }
                
                /* Worker-private temporary array access */
                tmp_array[i][j] = temp % 256;
            }
        }
        
        /* Wait for first kernel to complete */
        #pragma acc wait(1)
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach */
        /* Uses only gang and vector partitioning (no worker) */
        #pragma acc kernels loop gang vector \
                private(row_sums) copy(row_sums[0:actual_N]) \
                async(2)
        for (int i = 0; i < actual_N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < actual_M; j++) {
                /* Conditional update based on previous kernel's result */
                if (dst[i][j] > 0) {
                    row_sum += dst[i][j];
                } else {
                    row_sum -= src[i][j];
                }
            }
            row_sums[i] = row_sum;
            
            /* Access global device data */
            #pragma acc atomic update
            global_sum += row_sum;
        }
        
        /* Wait for second kernel */
        #pragma acc wait(2)
        
        /* THIRD REGION: Nested parallelism with explicit clauses */
        /* This creates gang+worker+vector partitioning scenario */
        #pragma acc parallel loop gang worker num_gangs(8) \
                firstprivate(actual_N, actual_M) \
                present(global_matrix)
        for (int i = 0; i < actual_N; i += 4) {
            /* Worker-level loop */
            #pragma acc loop worker
            for (int w = 0; w < 4 && (i+w) < actual_N; w++) {
                int idx = i + w;
                /* Vector-level loop */
                #pragma acc loop vector vector_length(32)
                for (int j = 0; j < actual_M; j++) {
                    /* Complex update with multiple array accesses */
                    int val = src[idx][j] + dst[idx][j] + tmp_array[idx][j];
                    if (val % 7 == 0) {
                        global_matrix[idx][j] = val;
                    } else {
                        global_matrix[idx][j] = val * 2;
                    }
                }
            }
        }
    }
    
    /* MIX OpenMP and OpenACC: Pure OpenMP region outside ACC data */
    /* This stresses compiler's multi-runtime handling */
    #pragma omp parallel for reduction(+:total_sum)
    for (int i = 0; i < actual_N; i++) {
        int local_sum = 0;
        for (int j = 0; j < actual_M; j++) {
            local_sum += dst[i][j];
        }
        total_sum += local_sum;
    }
    
    /* Verification output to prevent dead code elimination */
    printf("Checksum: %d (N=%d, M=%d)\n", total_sum, actual_N, actual_M);
}

/* Helper function with another ACC region */
void additional_computation(int size) {
    int array[256][256];
    int partial_sums[16] = {0};
    
    #pragma acc data copy(array, partial_sums)
    {
        /* Region with gang partitioned reduction */
        #pragma acc parallel loop gang collapse(2) \
                reduction(+:partial_sums[:16])
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                array[i][j] = i * j;
                int gang_id = (i + j) % 16;
                partial_sums[gang_id] += array[i][j];
            }
        }
        
        /* Fully partitioned scenario */
        #pragma acc parallel loop gang worker vector collapse(2) \
                firstprivate(size)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (array[i][j] > 1000) {
                    array[i][j] /= 2;
                } else {
                    array[i][j] *= 2;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Dynamic bounds based on arguments */
    int N = argc > 1 ? atoi(argv[1]) : 128;
    int M = argc > 2 ? atoi(argv[2]) : 128;
    
    /* Initialize global device data */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = i + j;
        }
    }
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, argc);
    
    /* Additional computation with different patterns */
    additional_computation(N < 256 ? N : 256);
    
    /* Final verification using global device data */
    int final_check = 0;
    #pragma acc parallel loop reduction(+:final_check) \
            present(global_matrix, global_sum)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            final_check += global_matrix[i][j];
        }
    }
    
    printf("Global check: %d (global_sum=%d)\n", final_check, global_sum);
    
    return 0;
}
