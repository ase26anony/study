/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   Lines 335-343: partitioning type string lookup function */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force partitioning decisions */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void process_data(int N, int M, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = N;
    volatile int dyn_M = M;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_matrix[256][256];
    int total_sum = 0;
    int partial_sums[16] = {0};
    
    /* Initialize arrays */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < dyn_N; i++) {
        for (int j = 0; j < dyn_M; j++) {
            src[i][j] = i * j + argc;  /* Use argc for dynamic behavior */
            dst[i][j] = 0;
            if (i < 256 && j < 256) {
                global_matrix[i][j] = (i + j) % 100;
                tmp_matrix[i][j] = 0;
            }
        }
    }
    
    /* OpenACC data region with explicit data management */
    #pragma acc data copy(src[0:dyn_N][0:dyn_M], dst[0:dyn_N][0:dyn_M]) \
                     copyin(tmp_matrix) copy(global_matrix) \
                     copy(total_sum) copy(partial_sums)
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_matrix) reduction(+:total_sum) \
                firstprivate(dyn_N, dyn_M)
        for (int i = 1; i < dyn_N-1; i++) {
            for (int j = 1; j < dyn_M-1; j++) {
                /* Conditional data access to force broadcast analysis */
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
                    temp = src[i][j] + global_matrix[i%256][j%256];
                    dst[i][j] = temp;
                }
                
                /* Reduction with private variable */
                total_sum += temp;
                
                /* Access to declared device data */
                if (i < 256 && j < 256) {
                    tmp_matrix[i][j] = temp % 100;
                }
            }
        }
        
        /* SECOND OPENACC REGION: Different partitioning scheme */
        /* gang+vector without worker partitioning */
        #pragma acc kernels loop gang vector collapse(2) \
                private(partial_sums) firstprivate(dyn_N)
        for (int i = 0; i < dyn_N; i += 32) {
            for (int j = 0; j < dyn_M; j += 32) {
                /* Local reduction in tile */
                int tile_sum = 0;
                #pragma acc loop seq
                for (int ti = i; ti < i+32 && ti < dyn_N; ti++) {
                    for (int tj = j; tj < j+32 && tj < dyn_M; tj++) {
                        /* Conditional update based on position */
                        if (ti % 2 == 0) {
                            tile_sum += dst[ti][tj];
                        } else {
                            tile_sum -= dst[ti][tj] / 2;
                        }
                    }
                }
                /* Store partial sum with gang-specific index */
                int gang_idx = (i/32) % 16;
                partial_sums[gang_idx] += tile_sum;
            }
        }
        
        /* THIRD OPENACC REGION: worker+vector partitioning */
        /* Using kernels construct instead of parallel */
        #pragma acc kernels loop worker vector collapse(2)
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                /* Update global matrix with conditional */
                if (global_matrix[i][j] > 50) {
                    global_matrix[i][j] = tmp_matrix[i][j] + 
                                         (i * j) % 100;
                } else {
                    global_matrix[i][j] = tmp_matrix[i][j] - 
                                         (i + j) % 100;
                }
                
                /* Nested conditional with reduction-like operation */
                #pragma acc atomic update
                global_sum += global_matrix[i][j] % 10;
            }
        }
    }
    
    /* OpenMP region outside OpenACC - mixing pragmas */
    #pragma omp parallel for reduction(+:total_sum)
    for (int i = 0; i < dyn_N; i++) {
        for (int j = 0; j < dyn_M; j++) {
            /* Verify results from device computation */
            if (dst[i][j] != src[i][j]) {
                total_sum += dst[i][j];
            }
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Checksum: total_sum = %d, global_sum = %d\n", 
           total_sum, global_sum);
    
    /* Additional computation with different array shapes */
    int arr_3d[64][64][4];
    #pragma acc parallel loop gang collapse(2) \
            firstprivate(dyn_N) if(dyn_N > 256)
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            #pragma acc loop vector
            for (int k = 0; k < 4; k++) {
                arr_3d[i][j][k] = (i * j * k + total_sum) % 1000;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Dynamic problem size based on arguments */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N <= 0) N = 512;
    if (M <= 0) M = 512;
    
    printf("Running with N=%d, M=%d\n", N, M);
    
    /* Call function with complex OpenACC patterns */
    process_data(N, M, argc);
    
    /* Additional test with smaller size for different partitioning */
    if (argc > 3) {
        int small_N = atoi(argv[3]);
        #pragma acc parallel loop gang worker vector \
                private(global_sum) reduction(max:small_N)
        for (int i = 0; i < small_N; i++) {
            int val = i * i;
            if (val % 7 == 0) {
                #pragma acc atomic update
                global_sum += val;
            }
        }
    }
    
    return 0;
}
