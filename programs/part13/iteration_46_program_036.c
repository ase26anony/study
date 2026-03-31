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
void process_data(int N, int M, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = N;
    volatile int dyn_M = M;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_matrix[256][256];
    
    /* Reduction variables with different scopes */
    int total_sum = 0;
    int row_sums[512] = {0};
    int tile_sum = 0;
    
    /* Initialize source data with pattern */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < dyn_N; i++) {
        for (int j = 0; j < dyn_M; j++) {
            src[i][j] = i * 1000 + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:dyn_N][0:dyn_M], dst[0:dyn_N][0:dyn_M]) \
                     copyin(tmp_matrix[0:256][0:256]) \
                     copy(total_sum, row_sums[0:dyn_N])
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_matrix) reduction(+:total_sum) \
                firstprivate(dyn_N, dyn_M)
        for (int i = 1; i < dyn_N-1; i++) {
            for (int j = 1; j < dyn_M-1; j++) {
                /* Local private array to force partitioning decisions */
                int local_tmp[4][4];
                
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with different access patterns */
                    int sum = 0;
                    #pragma acc loop vector reduction(+:sum)
                    for (int di = -1; di <= 1; di++) {
                        for (int dj = -1; dj <= 1; dj++) {
                            sum += src[i+di][j+dj];
                        }
                    }
                    dst[i][j] = sum / 9;
                    
                    /* Fill local array with conditional pattern */
                    for (int ti = 0; ti < 4; ti++) {
                        for (int tj = 0; tj < 4; tj++) {
                            local_tmp[ti][tj] = (ti * tj < (i+j) % 8) ? 
                                                src[i][j] : dst[i][j];
                        }
                    }
                    
                    /* Reduction with conditional */
                    if (dst[i][j] > 1000) {
                        total_sum += dst[i][j];
                    }
                } else {
                    /* Alternative computation path */
                    dst[i][j] = src[i][j] * 2;
                    #pragma acc atomic update
                    total_sum += src[i][j];
                }
                
                /* Access global declared data */
                if (i < 256 && j < 256) {
                    global_matrix[i][j] = dst[i][j];
                }
            }
        }
        
        /* SECOND OPENACC REGION: Different partitioning strategy */
        /* gang+vector only, no worker partitioning */
        #pragma acc parallel loop gang vector collapse(2) \
                reduction(+:tile_sum) private(row_sums)
        for (int i = 0; i < dyn_N; i += 16) {
            for (int j = 0; j < dyn_M; j += 16) {
                /* Tile-based computation */
                int tile_total = 0;
                #pragma acc loop worker vector collapse(2)
                for (int ti = 0; ti < 16 && (i+ti) < dyn_N; ti++) {
                    for (int tj = 0; tj < 16 && (j+tj) < dyn_M; tj++) {
                        tile_total += dst[i+ti][j+tj];
                        
                        /* Conditional update based on argc */
                        if (argc > 2 && (ti*tj) % 5 == 0) {
                            dst[i+ti][j+tj] += tile_total % 100;
                        }
                    }
                }
                tile_sum += tile_total;
            }
        }
        
        /* THIRD OPENACC REGION: worker+vector partitioning */
        /* Using kernels directive for different optimization path */
        #pragma acc kernels copyout(row_sums[0:dyn_N])
        {
            #pragma acc loop gang
            for (int i = 0; i < dyn_N; i++) {
                int row_sum = 0;
                #pragma acc loop worker vector reduction(+:row_sum)
                for (int j = 0; j < dyn_M; j++) {
                    /* Complex conditional access */
                    if (j % 2 == 0) {
                        row_sum += src[i][j];
                    } else {
                        row_sum += dst[i][j] * (i % 3 + 1);
                    }
                    
                    /* Data-dependent array update */
                    if (row_sum > 1000000) {
                        dst[i][j] = row_sum % 256;
                    }
                }
                row_sums[i] = row_sum;
                
                /* Update global sum with atomic */
                #pragma acc atomic
                global_sum += row_sum;
            }
        }
    }
    
    /* Mixed OpenMP region outside OpenACC data region */
    int verify_sum = 0;
    #pragma omp parallel for reduction(+:verify_sum) schedule(dynamic)
    for (int i = 0; i < dyn_N; i++) {
        for (int j = 0; j < dyn_M; j++) {
            verify_sum += dst[i][j];
        }
    }
    
    printf("Total sum from OpenACC: %d\n", total_sum);
    printf("Tile sum: %d\n", tile_sum);
    printf("Verification sum: %d\n", verify_sum);
    printf("Global sum: %d\n", global_sum);
}

/* Helper function with nested OpenACC regions */
void nested_partitioning(int size) {
    int matrix[128][128];
    int vector[128];
    
    /* Initialize */
    #pragma acc parallel loop collapse(2) gang vector
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Fully partitioned scenario */
    #pragma acc parallel loop gang worker vector
    for (int i = 0; i < size; i++) {
        int row_acc = 0;
        #pragma acc loop vector reduction(+:row_acc)
        for (int j = 0; j < size; j++) {
            row_acc += matrix[i][j];
        }
        vector[i] = row_acc;
        
        /* Nested parallel region inside */
        #pragma acc parallel loop worker vector private(row_acc)
        for (int k = 0; k < i; k++) {
            matrix[i][k] += vector[k];
        }
    }
}

int main(int argc, char **argv) {
    /* Dynamic sizes based on command line */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    
    printf("Running with N=%d, M=%d\n", N, M);
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, argc);
    
    /* Additional partitioning patterns */
    nested_partitioning(128);
    
    /* Final OpenMP region to mix pragmas */
    int final_check = 0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < 1000; i++) {
        final_check += i * (argc + 1);
    }
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
