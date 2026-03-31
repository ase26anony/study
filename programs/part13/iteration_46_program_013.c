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
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_matrix[256][256];
    int total_sum = 0;
    int row_sums[512] = {0};
    
    /* Initialize source arrays with pattern */
    for (int i = 0; i < dyn_N; i++) {
        for (int j = 0; j < dyn_M; j++) {
            src[i][j] = (i * j) % 100;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region for host-device transfers */
    #pragma acc data copy(src[0:dyn_N][0:dyn_M], dst[0:dyn_N][0:dyn_M]) \
                     copyin(tmp_matrix[0:dyn_N/2][0:dyn_M/2]) \
                     copy(total_sum, row_sums[0:dyn_N])
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario */
        /* Uses gang+worker+vector with private and reduction clauses */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_matrix) reduction(+:total_sum) \
                firstprivate(dyn_N, dyn_M) if(use_condition)
        for (int i = 1; i < dyn_N-1; i++) {
            for (int j = 1; j < dyn_M-1; j++) {
                /* Local private array - forces partitioning analysis */
                int local_tmp[4][4];
                
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with data-dependent access */
                    int tmp = src[i-1][j] + src[i+1][j] + 
                              src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                    
                    /* Fill private array with conditional pattern */
                    for (int ti = 0; ti < 4; ti++) {
                        for (int tj = 0; tj < 4; tj++) {
                            local_tmp[ti][tj] = (ti * tj < (i+j) % 8) ? 
                                                src[i][j] : dst[i][j];
                        }
                    }
                    
                    /* Reduction with conditional */
                    if (dst[i][j] > 50) {
                        total_sum += dst[i][j];
                    }
                } else {
                    dst[i][j] = src[i][j];
                }
                
                /* Access global declared data */
                #pragma acc atomic update
                global_matrix[i % 256][j % 256] += dst[i][j] % 10;
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach */
        /* Uses only gang+vector partitioning (no worker) */
        #pragma acc kernels loop gang vector \
                private(row_sums) firstprivate(dyn_N, dyn_M)
        for (int i = 0; i < dyn_N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < dyn_M; j++) {
                /* Different computation pattern */
                row_sum += dst[i][j];
                
                /* Another conditional pattern */
                if (argc > 2 && (i * j) % 7 == 0) {
                    dst[i][j] = (dst[i][j] * 3) / 2;
                }
            }
            row_sums[i] = row_sum;
            
            /* Update global sum with atomic */
            #pragma acc atomic
            global_sum += row_sum % 1000;
        }
        
        /* THIRD: Nested parallelism with manual gang/worker/vector clauses */
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
                copy(total_sum) firstprivate(dyn_N, dyn_M)
        {
            #pragma acc loop gang
            for (int block = 0; block < dyn_N; block += 64) {
                #pragma acc loop worker
                for (int w = 0; w < 4; w++) {
                    #pragma acc loop vector
                    for (int i = block + w*16; i < block + (w+1)*16 && i < dyn_N; i++) {
                        int local_sum = 0;
                        #pragma acc loop seq
                        for (int j = 0; j < dyn_M; j++) {
                            local_sum += dst[i][j] % 17;
                        }
                        #pragma acc atomic
                        total_sum += local_sum;
                    }
                }
            }
        }
    }
    
    /* PURE OPENMP REGION: Mix pragma types to stress compiler */
    #pragma omp parallel for schedule(dynamic) if(dyn_N > 100)
    for (int i = 0; i < dyn_N; i++) {
        int omp_sum = 0;
        for (int j = 0; j < dyn_M; j++) {
            omp_sum += src[i][j] % 13;
        }
        #pragma omp atomic
        total_sum += omp_sum;
    }
    
    /* Verification output to prevent dead code elimination */
    printf("Checksum: %d (N=%d, M=%d, argc=%d)\n", 
           total_sum % 1000000, dyn_N, dyn_M, argc);
}

/* Helper function with another OpenACC region */
void helper_function(int size) {
    int array[1024];
    int sum = 0;
    
    #pragma acc parallel loop gang worker vector \
            private(array) reduction(+:sum) if(size > 256)
    for (int i = 0; i < size; i++) {
        array[i % 1024] = i * 2;
        if (i % 5 == 0) {
            sum += array[i % 1024];
        }
    }
    
    printf("Helper sum: %d\n", sum);
}

int main(int argc, char **argv) {
    /* Dynamic bounds based on command line to prevent optimization */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    int use_condition = argc > 3 ? atoi(argv[3]) : 1;
    
    /* Initialize global declared data */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 19;
        }
    }
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, use_condition, argc);
    
    /* Call helper with different size */
    helper_function(N / 2);
    
    /* Final verification using global data */
    int final_check = 0;
    #pragma acc parallel loop reduction(+:final_check) \
            present(global_matrix, global_sum)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            final_check += global_matrix[i][j];
        }
    }
    final_check += global_sum;
    
    printf("Final verification: %d\n", final_check % 1000000);
    
    return 0;
}
