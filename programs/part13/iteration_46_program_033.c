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
void process_data(int N, int M, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = N;
    volatile int dyn_M = M;
    int actual_N = dyn_N;
    int actual_M = dyn_M;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_matrix[256][256];
    
    /* Reduction variables with different scopes */
    int total_sum = 0;
    int row_sums[512] = {0};
    int tile_sum = 0;
    
    /* Initialize source data with pattern */
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = (i * 3 + j * 7) % 97;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region for host-device transfers */
    #pragma acc data copy(src, dst) copyout(row_sums[0:actual_N]) \
                   create(tmp_matrix) copyin(actual_N, actual_M)
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_matrix) reduction(+:total_sum) \
                firstprivate(actual_N, actual_M)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Local private array - forces partitioning decisions */
                int local_tmp[4][4];
                
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil computation with different access patterns */
                    int tmp = src[i-1][j] + src[i+1][j] + 
                             src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                    
                    /* Fill local array with conditional pattern */
                    for (int ti = 0; ti < 4; ti++) {
                        for (int tj = 0; tj < 4; tj++) {
                            local_tmp[ti][tj] = (ti == tj) ? src[i][j] : tmp;
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
                global_matrix[i % 256][j % 256] += dst[i][j] % 17;
            }
        }
        
        /* SECOND OPENACC REGION: Different partitioning strategy */
        /* Uses only gang+vector partitioning (case 5) */
        #pragma acc kernels loop gang vector independent \
                reduction(+:tile_sum) private(tmp_matrix)
        for (int i = 0; i < actual_N; i += 16) {
            int tile_size = (i + 16 > actual_N) ? actual_N - i : 16;
            
            #pragma acc loop vector
            for (int j = 0; j < actual_M; j++) {
                int row_acc = 0;
                
                /* Worker-like computation within vector loop */
                for (int ti = 0; ti < tile_size; ti++) {
                    int idx = i + ti;
                    /* Conditional with data-dependent access */
                    if (idx < actual_N && (idx + j) % 2 == 0) {
                        row_acc += src[idx][j] * dst[idx][j];
                    }
                }
                
                /* Store to array with atomic for potential conflicts */
                #pragma acc atomic
                row_sums[i/16] += row_acc;
                
                tile_sum += row_acc % 13;
            }
        }
        
        /* THIRD OPENACC REGION: Nested parallelism with explicit clauses */
        /* Should trigger gang+worker partitioned analysis (case 3) */
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
                copy(tile_sum) present(src, dst)
        {
            #pragma acc loop gang
            for (int block = 0; block < 8; block++) {
                int start_row = block * (actual_N / 8);
                int end_row = (block == 7) ? actual_N : (block + 1) * (actual_N / 8);
                
                #pragma acc loop worker
                for (int w = 0; w < 4; w++) {
                    int worker_sum = 0;
                    
                    #pragma acc loop vector reduction(+:worker_sum)
                    for (int i = start_row; i < end_row; i++) {
                        for (int j = w * (actual_M / 4); j < (w + 1) * (actual_M / 4); j++) {
                            /* Complex conditional chain */
                            if (i % 2 == 0) {
                                if (j % 3 == 0) {
                                    worker_sum += src[i][j] * 2;
                                } else {
                                    worker_sum += dst[i][j] / 3;
                                }
                            } else if (j % 4 == 0) {
                                worker_sum += (src[i][j] + dst[i][j]) % 19;
                            }
                        }
                    }
                    
                    #pragma acc atomic
                    tile_sum += worker_sum;
                }
            }
        }
    }
    
    /* MIX OpenMP with OpenACC in same function */
    /* This stresses compiler's multi-runtime handling */
    #pragma omp parallel for reduction(+:global_sum) schedule(dynamic)
    for (int i = 0; i < actual_N; i++) {
        int local_sum = 0;
        for (int j = 0; j < actual_M; j++) {
            local_sum += dst[i][j] - src[i][j];
        }
        global_sum += local_sum;
        
        /* Also access OpenACC-declared data from OpenMP region */
        if (i < 256) {
            #pragma omp atomic
            global_matrix[i][i % 256] += local_sum % 23;
        }
    }
    
    /* Final verification print to prevent dead code elimination */
    printf("Partitioning test results:\n");
    printf("  Total sum from first region: %d\n", total_sum);
    printf("  Tile sum from second region: %d\n", tile_sum);
    printf("  Global sum from OpenMP: %d\n", global_sum);
    
    /* Sample check of row sums */
    int row_check = 0;
    for (int i = 0; i < actual_N/16 + 1; i++) {
        row_check += row_sums[i] % 7;
    }
    printf("  Row sums checksum: %d\n", row_check);
}

int main(int argc, char *argv[]) {
    /* Dynamic dimensions based on command line */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    if (N < 64) N = 64;
    if (M < 64) M = 64;
    
    /* Initialize global declared data */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j * 11) % 31;
        }
    }
    
    /* Copy global data to device via declare directive */
    #pragma acc update device(global_matrix)
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, argc);
    
    /* Final update and check */
    #pragma acc update host(global_matrix, global_sum)
    
    int final_check = 0;
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            final_check = (final_check + global_matrix[i][j]) % 10007;
        }
    }
    printf("Final global matrix checksum: %d\n", final_check);
    
    return 0;
}
