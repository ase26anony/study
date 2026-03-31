/* test-omp-oacc-neuter-broadcast.c
 * Designed to trigger the partitioning type string lookup function
 * in omp-oacc-neuter-broadcast.cc (lines 335-343)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Persistent device data - may trigger partitioning analysis during declare */
#pragma acc declare create(global_matrix)
static int global_matrix[256][256];

/* Function with mixed OpenACC/OpenMP usage */
void process_data(int N, int M, int use_conditional) {
    int i, j;
    int local_sum = 0;
    volatile int dynamic_bound = N; /* Prevent constant folding */
    
    /* 2D arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    
    /* Initialize arrays */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = i * M + j;
            dst[i][j] = 0;
            global_matrix[i % 256][j % 256] = (i + j) % 100;
        }
    }
    
    /* OpenACC data region with complex partitioning */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) copyin(global_matrix)
    {
        int tmp; /* Will be made private */
        
        /* FIRST COMPUTE REGION: Complex partitioning with all levels */
        /* This should trigger analysis of gang+worker+vector partitioning */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp) reduction(+:local_sum) \
                present(src, dst, global_matrix)
        for (i = 1; i < dynamic_bound-1; i++) {
            for (j = 1; j < M-1; j++) {
                /* Conditional data access - forces different broadcast strategies */
                if (use_conditional && (i % 3 == 0)) {
                    /* Different access pattern for some rows */
                    tmp = src[i-1][j] + src[i+1][j];
                } else {
                    /* Stencil computation */
                    tmp = src[i-1][j] + src[i][j-1] + 
                          src[i][j+1] + src[i+1][j];
                }
                
                /* Mix in global matrix access */
                tmp += global_matrix[i % 256][j % 256] % 10;
                
                dst[i][j] = tmp;
                local_sum += tmp;
                
                /* Additional conditional that might affect partitioning */
                if (j % 7 == 0) {
                    dst[i][j] *= 2;
                }
            }
        }
        
        /* SECOND COMPUTE REGION: Different partitioning scheme */
        /* Should trigger gang+vector partitioning analysis */
        int row_sums[512] = {0};
        
        #pragma acc parallel loop gang vector \
                firstprivate(row_sums) private(tmp) \
                present(src, dst)
        for (i = 0; i < dynamic_bound; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                row_sum += src[i][j];
                /* Conditional store */
                if (src[i][j] > 1000) {
                    tmp = src[i][j] / 2;
                    dst[i][j] += tmp;
                }
            }
            row_sums[i] = row_sum;
        }
        
        /* THIRD: Worker-only style partitioning */
        /* Using kernels directive for different optimization path */
        #pragma acc kernels loop worker collapse(2) \
                present(src, dst)
        for (i = 0; i < dynamic_bound; i += 2) {
            for (j = 0; j < M; j += 2) {
                /* Block operation - might trigger worker partitioning */
                dst[i][j] += src[i][j];
                if (i+1 < dynamic_bound) dst[i+1][j] += src[i+1][j];
                if (j+1 < M) dst[i][j+1] += src[i][j+1];
                if (i+1 < dynamic_bound && j+1 < M) 
                    dst[i+1][j+1] += src[i+1][j+1];
            }
        }
    }
    
    /* Additional OpenMP region outside ACC data */
    #pragma omp parallel for private(i, j) reduction(+:local_sum)
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            /* Verify computation */
            if (dst[i][j] > 1000000) {
                local_sum += 1;
            }
        }
    }
    
    printf("Intermediate checksum: %d\n", local_sum);
}

/* Function with nested parallelism */
void nested_parallelism(int size) {
    int i, j, k;
    int cube[64][64][64];
    int total = 0;
    
    /* Initialize 3D array */
    #pragma acc parallel loop collapse(3) gang worker vector \
            private(i, j, k) copy(cube) reduction(+:total)
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            for (k = 0; k < size; k++) {
                cube[i][j][k] = i + j * 2 + k * 3;
                total += cube[i][j][k];
            }
        }
    }
    
    /* Process with different loop nesting */
    #pragma acc parallel loop gang collapse(2) \
            copy(cube) reduction(+:total)
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            int row_total = 0;
            #pragma acc loop vector reduction(+:row_total)
            for (k = 0; k < size; k++) {
                row_total += cube[i][j][k];
            }
            total += row_total;
        }
    }
}

int main(int argc, char **argv) {
    int N, M;
    
    /* Dynamic bounds to prevent constant folding */
    N = (argc > 1) ? atoi(argv[1]) : 128;
    M = (argc > 2) ? atoi(argv[2]) : 128;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    
    printf("Testing OpenACC partitioning with N=%d, M=%d\n", N, M);
    
    /* First test: with conditional access */
    process_data(N, M, 1);
    
    /* Second test: without conditional access */
    process_data(N/2, M/2, 0);
    
    /* Third test: 3D nested parallelism */
    nested_parallelism(32);
    
    /* Final verification output */
    printf("Test completed successfully.\n");
    
    return 0;
}
