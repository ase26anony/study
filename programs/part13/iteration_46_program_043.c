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
void process_data(int N, int M, int use_condition, int argc) {
    int i, j, k;
    volatile int dynamic_bound = N; /* Prevent constant folding */
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_buffer[512][512];
    int row_sums[512] = {0};
    int total_sum = 0;
    int partial_sums[8] = {0}; /* For nested reductions */
    
    /* Initialize source data with pattern */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = (i * M + j) % 100;
            dst[i][j] = 0;
            tmp_buffer[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) \
                     copyin(tmp_buffer[0:N][0:M]) \
                     copyout(row_sums[0:N]) \
                     copy(total_sum) \
                     create(partial_sums)
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:total_sum) \
                firstprivate(use_condition) \
                async(1)
        for (i = 1; i < N-1; i++) {
            for (j = 1; j < M-1; j++) {
                int tmp = 0;
                
                /* Conditional data access pattern */
                if (use_condition && (i % 2 == 0)) {
                    /* Stencil operation with temporary */
                    #pragma acc loop vector reduction(+:tmp)
                    for (k = -1; k <= 1; k++) {
                        tmp += src[i+k][j] + src[i][j+k];
                    }
                    dst[i][j] = tmp / 5;
                } else {
                    /* Different access pattern */
                    tmp = src[i][j] * 2;
                    if (j % 3 == 0) {
                        tmp += src[i-1][j];
                    }
                    dst[i][j] = tmp;
                }
                
                /* Reduction with conditional */
                if (dst[i][j] > 50) {
                    total_sum += dst[i][j];
                }
            }
        }
        
        #pragma acc wait(1)
        
        /* SECOND OPENACC REGION: Different partitioning - gang+vector only */
        /* Should trigger different partitioning code path */
        #pragma acc kernels loop gang vector collapse(2) \
                private(tmp) reduction(+:partial_sums[:8]) \
                copy(global_matrix) /* Reference declared data */
        for (i = 0; i < N; i += 64) {
            for (j = 0; j < M; j += 64) {
                int tmp = 0;
                int tile_i, tile_j;
                
                /* Nested tile processing */
                #pragma acc loop worker vector collapse(2) reduction(+:tmp)
                for (tile_i = i; tile_i < i+64 && tile_i < N; tile_i++) {
                    for (tile_j = j; tile_j < j+64 && tile_j < M; tile_j++) {
                        /* Mixed access to different arrays */
                        tmp += dst[tile_i][tile_j] - src[tile_i][tile_j];
                        global_matrix[tile_i % 256][tile_j % 256] += 
                            (tile_i + tile_j) % 10;
                    }
                }
                
                /* Array reduction */
                partial_sums[(i/64) % 8] += tmp;
            }
        }
        
        /* THIRD OPENACC REGION: Worker+vector partitioning scenario */
        /* Using kernels construct for different lowering path */
        #pragma acc kernels loop worker vector
        for (i = 0; i < N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                /* Conditional store with temporary */
                int temp = dst[i][j];
                if (temp > 0) {
                    row_sum += temp;
                    tmp_buffer[i][j] = temp * 2;
                } else {
                    tmp_buffer[i][j] = temp / 2;
                }
            }
            row_sums[i] = row_sum;
            
            /* Access to global declared data */
            if (i < 256 && argc > 2) {
                #pragma acc atomic update
                global_sum += row_sum % 100;
            }
        }
        
    } /* End OpenACC data region */
    
    /* Mixed OpenMP region outside OpenACC */
    #pragma omp parallel for private(i, j) reduction(+:total_sum) schedule(dynamic)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Cross-verify OpenACC results */
            if (dst[i][j] != tmp_buffer[i][j]) {
                total_sum += 1;
            }
        }
    }
    
    printf("Verification checksum: %d\n", total_sum);
    printf("Row sums[0], [100], [200]: %d, %d, %d\n", 
           row_sums[0], row_sums[100], row_sums[200]);
}

/* Helper with another OpenACC region using declare directive */
void process_global_data(int size) {
    int i, j;
    
    #pragma acc parallel loop collapse(2) copy(global_matrix) \
                gang worker vector /* Fully partitioned */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            /* Complex update with multiple temporaries */
            int tmp1 = global_matrix[i][j];
            int tmp2 = (i * j) % 100;
            int tmp3 = tmp1 + tmp2;
            
            if ((i + j) % 4 == 0) {
                global_matrix[i][j] = tmp3 * 2;
            } else if ((i + j) % 4 == 1) {
                global_matrix[i][j] = tmp3 / 2;
            } else {
                global_matrix[i][j] = tmp3;
            }
        }
    }
}

int main(int argc, char **argv) {
    int N = 512, M = 512;
    
    /* Dynamic bounds to prevent compile-time optimization */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0 || N > 512) N = 512;
    }
    if (argc > 2) {
        M = atoi(argv[2]);
        if (M <= 0 || M > 512) M = 512;
    }
    
    printf("Starting OpenACC partitioning test with N=%d, M=%d\n", N, M);
    
    /* Initialize global declared data */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 50;
        }
    }
    
    /* Process with conditional flag based on argc */
    int use_condition = (argc % 2 == 0);
    process_data(N, M, use_condition, argc);
    
    /* Second function with different partitioning needs */
    process_global_data(128);
    
    /* Final OpenMP region for mixing */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp critical
        printf("Thread %d completed\n", tid);
    }
    
    printf("Test completed successfully.\n");
    return 0;
}
