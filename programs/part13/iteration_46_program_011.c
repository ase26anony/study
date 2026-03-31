/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare directive partitioning */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void process_data(int N, int M, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = N;
    volatile int dyn_M = M;
    int i, j, k;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_matrix[256][256];
    int total_sum = 0;
    int row_sums[512] = {0};
    
    /* Initialize source arrays with pattern */
    for (i = 0; i < dyn_N; i++) {
        for (j = 0; j < dyn_M; j++) {
            src[i][j] = (i * j) % 100;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:dyn_N][0:dyn_M], dst[0:dyn_N][0:dyn_M]) \
                     copyin(tmp_matrix[0:256][0:256]) \
                     copy(total_sum, row_sums[0:dyn_N])
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario */
        /* Uses gang+worker+vector with private and reduction clauses */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:total_sum) \
                firstprivate(dyn_N, dyn_M)
        for (i = 1; i < dyn_N-1; i++) {
            for (j = 1; j < dyn_M-1; j++) {
                int temp = 0;  /* private variable */
                
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with data-dependent access */
                    for (k = -1; k <= 1; k++) {
                        temp += src[i+k][j] + src[i][j+k];
                    }
                    dst[i][j] = temp / 5;
                } else if ((i * j) % 7 == 0) {
                    /* Different access pattern */
                    temp = src[i-1][j-1] + src[i+1][j+1];
                    dst[i][j] = temp / 2;
                } else {
                    dst[i][j] = src[i][j];
                }
                
                /* Reduction with conditional */
                if (dst[i][j] > 50) {
                    total_sum += dst[i][j];
                }
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach */
        /* Only gang+vector partitioning (no worker) */
        #pragma acc kernels loop gang vector \
                private(j) reduction(+:global_sum)
        for (i = 0; i < 256; i++) {
            int row_temp = 0;
            #pragma acc loop vector
            for (j = 0; j < 256; j++) {
                /* Access declared device data */
                global_matrix[i][j] = src[i%512][j%512] * 2;
                row_temp += global_matrix[i][j];
                
                /* Conditional update */
                if (global_matrix[i][j] % 2 == 0) {
                    tmp_matrix[i][j] = global_matrix[i][j] / 2;
                }
            }
            global_sum += row_temp;
        }
        
        /* THIRD: Worker-partitioned loop inside parallel region */
        #pragma acc parallel loop gang worker num_gangs(4) num_workers(2) \
                private(i, j) firstprivate(dyn_N)
        for (int tile = 0; tile < 4; tile++) {
            int start = tile * (dyn_N / 4);
            int end = start + (dyn_N / 4);
            
            #pragma acc loop worker
            for (i = start; i < end; i++) {
                int row_sum = 0;
                #pragma acc loop vector
                for (j = 0; j < dyn_M; j++) {
                    row_sum += dst[i][j];
                }
                row_sums[i] = row_sum;
            }
        }
    }
    
    /* MIX OpenMP with OpenACC in same file */
    /* Pure OpenMP parallel region outside OpenACC data region */
    int omp_array[1000];
    #pragma omp parallel for private(i)
    for (i = 0; i < 1000; i++) {
        omp_array[i] = i * (argc > 1 ? atoi("1") : 2);
    }
    
    /* Another OpenACC region with fully partitioned scenario */
    int final_check = 0;
    #pragma acc parallel loop gang worker vector collapse(2) \
            reduction(+:final_check) copy(final_check)
    for (i = 0; i < 128; i++) {
        for (j = 0; j < 128; j++) {
            /* Access multiple data structures */
            int val = src[i*4][j*4] + dst[i*4][j*4] + 
                     tmp_matrix[i][j%256] + omp_array[(i+j)%1000];
            final_check += (val % 10);
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Global sum: %d\n", global_sum);
    printf("Final check: %d\n", final_check);
    
    /* Verify row sums */
    int verify_sum = 0;
    for (i = 0; i < dyn_N; i++) {
        verify_sum += row_sums[i];
    }
    printf("Row sums total: %d\n", verify_sum);
}

int main(int argc, char *argv[]) {
    /* Dynamic dimensions based on command line */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    
    /* Initialize declared device data */
    #pragma acc parallel loop gang vector collapse(2)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 100;
        }
    }
    
    /* Process with complex OpenACC constructs */
    process_data(N, M, argc);
    
    return 0;
}
