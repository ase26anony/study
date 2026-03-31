/* Complex OpenACC program designed to trigger execution of the
   partitioning type string lookup function in omp-oacc-neuter-broadcast.cc
   Lines 335-343: case 0: return "gang redundant"; ... case 7: return "fully partitioned";
   
   This program creates multiple OpenACC compute regions with different
   data partitioning scenarios to force the compiler to analyze and
   represent various partitioning strategies during compilation. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to test declare directive */
#pragma acc declare create(global_matrix)
static int global_matrix[256][256];

/* Function with complex OpenACC compute regions */
void compute_regions(int N, int M, int use_condition) {
    int i, j, k;
    volatile int dynamic_bound = N; /* Prevent constant folding */
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp[512][512];
    int row_sums[512] = {0};
    int col_sums[512] = {0};
    int total_sum = 0;
    int partial_sum = 0;
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = (i * M + j) % 97;
            dst[i][j] = 0;
            tmp[i][j] = 0;
        }
    }
    
    /* OpenACC data region for host-device data management */
    #pragma acc data copy(src, dst, tmp, row_sums, col_sums) \
                     copyin(total_sum, partial_sum) \
                     copyout(global_matrix)
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector
           This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) firstprivate(use_condition) reduction(+:total_sum)
        for (i = 1; i < dynamic_bound-1; i++) {
            for (j = 1; j < M-1; j++) {
                int temp = 0; /* private scalar */
                
                /* Conditional data access pattern */
                if (use_condition && (i % 3 == 0)) {
                    /* Complex stencil operation with data-dependent access */
                    temp = src[i-1][j] + src[i+1][j] + 
                           src[i][j-1] + src[i][j+1];
                    
                    /* Nested loop inside parallel region */
                    for (k = 0; k < 3; k++) {
                        temp += src[i-1+k][j-1+k];
                    }
                } else {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                }
                
                /* Conditional store */
                if (temp > 100) {
                    dst[i][j] = temp / 2;
                } else {
                    dst[i][j] = temp * 2;
                }
                
                /* Reduction with conditional */
                if ((i + j) % 2 == 0) {
                    total_sum += dst[i][j];
                }
                
                /* Access to global declared data */
                global_matrix[i % 256][j % 256] += dst[i][j];
            }
        }
        
        /* SECOND OPENACC REGION: Different partitioning - only gang+vector */
        #pragma acc kernels loop gang vector collapse(2) \
                private(i, j) reduction(+:partial_sum)
        for (i = 0; i < dynamic_bound; i += 2) {
            for (j = 0; j < M; j += 2) {
                int local_sum = 0;
                
                /* Block reduction pattern */
                for (int di = 0; di < 2 && i+di < dynamic_bound; di++) {
                    for (int dj = 0; dj < 2 && j+dj < M; dj++) {
                        local_sum += src[i+di][j+dj];
                    }
                }
                
                tmp[i/2][j/2] = local_sum;
                partial_sum += local_sum;
                
                /* Row-wise accumulation with conditional */
                if (local_sum > 50) {
                    #pragma acc atomic update
                    row_sums[i/2] += local_sum;
                }
            }
        }
        
        /* THIRD OPENACC REGION: Worker-level partitioning */
        #pragma acc parallel loop worker collapse(2) \
                firstprivate(dynamic_bound)
        for (i = 0; i < dynamic_bound; i++) {
            for (j = 0; j < M; j++) {
                /* Transform with data-dependent condition */
                if (src[i][j] > dst[i][j]) {
                    tmp[i][j] = src[i][j] - dst[i][j];
                } else {
                    tmp[i][j] = dst[i][j] - src[i][j];
                }
                
                /* Column sums with atomic */
                #pragma acc atomic update
                col_sums[j] += tmp[i][j];
            }
        }
        
        /* FOURTH OPENACC REGION: Nested parallelism with collapse */
        #pragma acc parallel loop gang(32) worker(4) vector_length(32) \
                collapse(3) private(i, j, k)
        for (i = 0; i < dynamic_bound/4; i++) {
            for (j = 0; j < M/4; j++) {
                for (k = 0; k < 4; k++) {
                    int idx_i = i*4 + k;
                    int idx_j = j*4 + k;
                    if (idx_i < dynamic_bound && idx_j < M) {
                        /* Complex update with multiple array accesses */
                        dst[idx_i][idx_j] = 
                            (src[idx_i][idx_j] + 
                             tmp[idx_i/2][idx_j/2] + 
                             global_matrix[idx_i % 256][idx_j % 256]) / 3;
                    }
                }
            }
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    #pragma omp parallel for private(i, j) reduction(+:total_sum)
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            /* Final verification computation on host */
            total_sum += dst[i][j] % 17;
        }
    }
    
    printf("Total checksum: %d, Partial sum: %d\n", total_sum, partial_sum);
}

int main(int argc, char **argv) {
    /* Dynamic bounds to prevent constant folding */
    int N = (argc > 1) ? atoi(argv[1]) : 256;
    int M = (argc > 2) ? atoi(argv[2]) : 256;
    int use_condition = (argc > 3) ? atoi(argv[3]) : 1;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    
    /* Initialize global matrix */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 31;
        }
    }
    
    /* Execute complex compute regions */
    compute_regions(N, M, use_condition);
    
    /* Additional OpenACC region with different clause combination */
    int small_arr[64][64];
    int small_sum = 0;
    
    #pragma acc parallel loop gang worker vector collapse(2) \
            copy(small_arr) reduction(+:small_sum)
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            small_arr[i][j] = (i * 64 + j) % 19;
            small_sum += small_arr[i][j];
            
            /* Nested conditional */
            if (small_arr[i][j] > 10) {
                small_arr[i][j] -= 5;
            } else {
                small_arr[i][j] += 5;
            }
        }
    }
    
    printf("Small array sum: %d\n", small_sum);
    
    return 0;
}
