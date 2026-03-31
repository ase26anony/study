/* test-omp-oacc-neuter-broadcast.c
 * Complex OpenACC program designed to trigger partitioning analysis
 * and potentially execute the uncovered switch statement in
 * omp-oacc-neuter-broadcast.cc lines 335-343.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Persistent device data to stress declare directive partitioning */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void compute_kernel(int N, int M, int use_condition, int argc) {
    int i, j, k;
    volatile int dynamic_bound = N; /* Prevent constant folding */
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_3d[64][64][64];
    
    /* Reduction variables with different scopes */
    int total_sum = 0;
    int row_sums[512] = {0};
    int tile_sum = 0;
    
    /* Initialize source arrays with pattern */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = (i * M + j) % 100;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) \
                     copyin(tmp_3d[0:64][0:64][0:64]) \
                     copy(total_sum, row_sums[0:N])
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning */
        /* Gang+worker+vector with private and reduction */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:total_sum, tile_sum) \
                firstprivate(use_condition)
        for (i = 1; i < dynamic_bound-1; i++) {
            for (j = 1; j < M-1; j++) {
                /* Private variable forcing partitioning analysis */
                int temp = 0;
                
                /* Conditional data access to force broadcast analysis */
                if (use_condition && (i % 2 == 0)) {
                    /* Stencil operation with neighbor access */
                    temp += src[i-1][j] + src[i+1][j] + 
                            src[i][j-1] + src[i][j+1];
                    
                    /* Nested loop inside parallel region */
                    for (k = 0; k < 4; k++) {
                        temp += src[i][j] * k;
                    }
                } else {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                }
                
                dst[i][j] = temp;
                total_sum += temp;
                
                /* Additional reduction on tile */
                if (i % 16 == 0 && j % 16 == 0) {
                    tile_sum += temp;
                }
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning */
        /* Only gang+vector partitioning (no worker) */
        #pragma acc kernels loop gang vector collapse(2) \
                private(j) reduction(+:row_sums[:N])
        for (i = 0; i < dynamic_bound; i++) {
            for (j = 0; j < M; j++) {
                row_sums[i] += dst[i][j];
                
                /* Access global declared data */
                global_matrix[i % 256][j % 256] += dst[i][j];
            }
        }
        
        /* THIRD: Worker-only partitioning scenario */
        #pragma acc parallel loop worker collapse(3) \
                private(i, j, k) reduction(+:global_sum)
        for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
                for (k = 0; k < 64; k++) {
                    tmp_3d[i][j][k] = i + j * 2 + k * 3;
                    global_sum += tmp_3d[i][j][k];
                    
                    /* Conditional with data-dependent access */
                    if (tmp_3d[i][j][k] % 7 == (argc % 7)) {
                        global_matrix[i % 256][j % 256] += tmp_3d[i][j][k];
                    }
                }
            }
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    int host_array[1024];
    #pragma omp parallel for private(i) schedule(dynamic)
    for (i = 0; i < 1024; i++) {
        host_array[i] = i * argc;
    }
    
    /* Verification output to prevent dead code elimination */
    printf("Total sum: %d, Tile sum: %d, Global sum: %d\n", 
           total_sum, tile_sum, global_sum);
    printf("Row 0 sum: %d, Row %d sum: %d\n", 
           row_sums[0], N-1, row_sums[N-1]);
}

/* Main function with dynamic bounds based on arguments */
int main(int argc, char **argv) {
    int N = 512;
    int M = 512;
    int use_condition = 1;
    
    /* Dynamic bounds to prevent constant folding */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N > 512) N = 512;
        if (N < 64) N = 64;
    }
    if (argc > 2) {
        M = atoi(argv[2]);
        if (M > 512) M = 512;
        if (M < 64) M = 64;
    }
    if (argc > 3) {
        use_condition = atoi(argv[3]) & 1;
    }
    
    /* Initialize global declared data */
    #pragma acc parallel loop collapse(2) gang vector
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = 0;
        }
    }
    
    /* Call compute kernel with complex partitioning scenarios */
    compute_kernel(N, M, use_condition, argc);
    
    /* Additional OpenACC region with fully partitioned scenario */
    int final_array[128][128];
    #pragma acc parallel loop collapse(2) independent \
            gang worker vector private(int)
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            final_array[i][j] = i * j + global_matrix[i % 256][j % 256];
        }
    }
    
    /* Final checksum */
    int checksum = 0;
    #pragma acc parallel loop reduction(+:checksum) gang
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            checksum += final_array[i][j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
