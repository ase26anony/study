/* test-omp-oacc-neuter-broadcast.c
 * Complex OpenACC program designed to trigger partitioning analysis
 * and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Persistent device arrays - may influence partitioning decisions */
#pragma acc declare create(global_matrix, global_stats)
static int global_matrix[256][256];
static float global_stats[256];

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void compute_kernels(int N, int M, int use_condition, int argc) {
    /* Dynamic sizing to prevent constant folding */
    volatile int dyn_N = (argc > 1) ? N : 512;
    volatile int dyn_M = (argc > 2) ? M : 512;
    
    int src[dyn_N][dyn_M];
    int dst[dyn_N][dyn_M];
    int partial_sums[dyn_N];
    int total_sum = 0;
    int tmp; /* Will be used with private clause */
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < dyn_N; i++) {
        for (int j = 0; j < dyn_M; j++) {
            src[i][j] = i * dyn_M + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src, dst, partial_sums, total_sum)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario
         * Multiple clauses to force partitioning analysis
         */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp) reduction(+:total_sum) \
                firstprivate(use_condition) \
                copyin(global_matrix[0:256][0:256])
        for (int i = 1; i < dyn_N-1; i++) {
            for (int j = 1; j < dyn_M-1; j++) {
                /* Conditional data access pattern */
                if (use_condition && (i % 2 == 0)) {
                    /* Different access pattern for even rows */
                    tmp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                } else {
                    /* Standard stencil */
                    tmp = src[i][j] + src[i-1][j-1] + src[i+1][j+1];
                }
                
                /* Mix with global matrix access */
                tmp += global_matrix[i % 256][j % 256];
                
                /* Conditional store */
                if (tmp > 0) {
                    dst[i][j] = tmp;
                } else {
                    dst[i][j] = -tmp;
                }
                
                /* Reduction with conditional */
                if (j % 3 == 0) {
                    total_sum += tmp;
                }
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach
         * Only gang and vector, no worker partitioning
         */
        #pragma acc kernels loop gang vector \
                private(tmp) reduction(+:partial_sums[0:dyn_N]) \
                copyout(global_stats[0:256])
        for (int i = 0; i < dyn_N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < dyn_M; j++) {
                tmp = dst[i][j];
                /* Another conditional pattern */
                if (i > j) {
                    row_sum += tmp * 2;
                } else {
                    row_sum += tmp / 2;
                }
                
                /* Update global stats */
                if (j % 64 == 0) {
                    #pragma acc atomic update
                    global_stats[i % 256] += tmp * 0.01f;
                }
            }
            partial_sums[i] = row_sum;
        }
        
        /* THIRD: Nested parallelism with manual gang/worker/vector separation */
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
                copy(global_matrix[0:128][0:128]) /* Partial copy */
        {
            #pragma acc loop gang
            for (int block = 0; block < 4; block++) {
                int start = block * (dyn_N / 4);
                int end = (block + 1) * (dyn_N / 4);
                
                #pragma acc loop worker
                for (int i = start; i < end; i++) {
                    int local_sum = 0;
                    
                    #pragma acc loop vector reduction(+:local_sum)
                    for (int j = 0; j < 128; j++) {
                        /* Access with stride pattern */
                        int idx = (i * 17 + j * 13) % 128;
                        local_sum += global_matrix[i % 128][idx];
                    }
                    
                    /* Worker-private computation */
                    for (int k = 0; k < 4; k++) {
                        partial_sums[i] += local_sum >> k;
                    }
                }
            }
        }
    }
    
    /* Final OpenMP section outside ACC data region */
    int host_verify = 0;
    #pragma omp parallel for reduction(+:host_verify) schedule(dynamic)
    for (int i = 0; i < dyn_N; i++) {
        for (int j = 0; j < dyn_M; j++) {
            host_verify += dst[i][j] % 100;
        }
    }
    
    printf("Checksum: total_sum=%d, host_verify=%d\n", total_sum, host_verify);
}

/* Helper function with another ACC region using declare directive */
void process_with_declared_data(int size) {
    int local_buf[size][size];
    
    /* Initialize using OpenMP */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            local_buf[i][j] = (i + j) % 255;
        }
    }
    
    /* Use declared global data */
    #pragma acc parallel loop gang worker vector collapse(2) \
            copy(local_buf[0:size][0:size]) \
            present(global_matrix, global_stats)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Complex access pattern mixing local and global */
            int val = local_buf[i][j];
            if (val > 128) {
                global_matrix[i % 256][j % 256] += val;
                #pragma acc atomic update
                global_stats[val % 256] += 1.0f;
            } else {
                global_matrix[i % 256][j % 256] -= val;
            }
            local_buf[i][j] = val * 2;
        }
    }
}

int main(int argc, char **argv) {
    printf("Starting complex OpenACC partitioning test...\n");
    
    /* Initialize global arrays */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = i * 256 + j;
        }
    }
    
    #pragma acc parallel loop present(global_stats)
    for (int i = 0; i < 256; i++) {
        global_stats[i] = 0.0f;
    }
    
    /* Call compute function with various parameters */
    compute_kernels(512, 512, argc > 1, argc);
    
    /* Process with different size */
    process_with_declared_data(128);
    
    /* Final verification */
    float final_stat = 0.0f;
    #pragma acc parallel loop reduction(+:final_stat) present(global_stats)
    for (int i = 0; i < 256; i++) {
        final_stat += global_stats[i];
    }
    
    printf("Final stat sum: %.2f\n", final_stat);
    printf("Test completed.\n");
    
    return 0;
}
