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
void process_data(int N, int M, int use_cond, int argc) {
    int i, j, k;
    volatile int dynamic_bound = N; /* Prevent constant folding */
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp[512][512];
    int total_sum = 0;
    int row_sums[512] = {0};
    
    /* Initialize arrays with pattern */
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = (i * M + j) % 100;
            dst[i][j] = 0;
            tmp[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) \
                     copyin(tmp[0:N][0:M]) copy(total_sum)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning */
        /* Multiple clauses to force partitioning analysis */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) firstprivate(N, M) reduction(+:total_sum) \
                present(src, dst, tmp)
        for (i = 1; i < N-1; i++) {
            for (j = 1; j < M-1; j++) {
                int temp = 0; /* private scalar */
                
                /* Conditional data access pattern */
                if (use_cond || (i % 2 == 0)) {
                    /* Stencil operation with multiple array accesses */
                    temp += src[i-1][j] + src[i+1][j] + 
                            src[i][j-1] + src[i][j+1];
                    
                    /* Data-dependent conditional */
                    if (temp > 100) {
                        dst[i][j] = temp / 4;
                    } else {
                        dst[i][j] = temp * 2;
                    }
                    
                    /* Reduction with private variable */
                    total_sum += dst[i][j];
                }
                
                /* Worker-level computation */
                #pragma acc loop vector
                for (k = 0; k < 4; k++) {
                    tmp[i][j] += dst[i][j] * k;
                }
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning */
        #pragma acc kernels loop gang vector independent \
                private(j) reduction(+:global_sum)
        for (i = 0; i < N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                /* Access global declared data */
                global_matrix[i][j] = src[i][j] + dst[i][j];
                row_sum += global_matrix[i][j];
                
                /* Another conditional pattern */
                if (j % 3 == 0 && argc > 1) {
                    row_sum -= src[i][j];
                }
            }
            row_sums[i] = row_sum;
            global_sum += row_sum;
        }
        
        /* THIRD: Nested parallelism with explicit clauses */
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
                copy(row_sums[0:N])
        {
            #pragma acc loop gang
            for (i = 0; i < N; i += 32) {
                #pragma acc loop worker
                for (j = 0; j < 32 && (i+j) < N; j++) {
                    int idx = i + j;
                    #pragma acc loop vector
                    for (k = 0; k < 16; k++) {
                        if (k < M) {
                            /* Complex indexing */
                            dst[idx][k] += row_sums[idx] * k;
                        }
                    }
                }
            }
        }
    }
    
    /* MIXED OpenMP outside OpenACC region */
    #pragma omp parallel for private(j) reduction(+:total_sum)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Cross-verify results */
            total_sum += dst[i][j] % 256;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Checksum: total_sum = %d, global_sum = %d\n", 
           total_sum, global_sum);
}

/* Another function with different partitioning pattern */
void process_3d(int size, int argc) {
    int i, j, k;
    int cube[64][64][64];
    int slice_sums[64] = {0};
    
    /* Initialize */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            for (k = 0; k < size; k++) {
                cube[i][j][k] = (i + j + k) % 50;
            }
        }
    }
    
    /* OpenACC with collapse and multiple clauses */
    #pragma acc parallel loop collapse(3) gang worker vector \
            private(i,j,k) reduction(+:slice_sums) copy(cube)
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            for (k = 0; k < size; k++) {
                /* Conditional update based on multiple indices */
                if ((i + j + k) % 7 == (argc % 7)) {
                    cube[i][j][k] *= 2;
                } else if ((i * j * k) % 11 == 0) {
                    cube[i][j][k] /= 2;
                }
                
                /* Multiple reductions */
                slice_sums[i] += cube[i][j][k];
            }
        }
    }
    
    /* Process results with OpenMP */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum)
    for (i = 0; i < size; i++) {
        final_sum += slice_sums[i];
    }
    
    printf("3D Cube checksum: %d\n", final_sum);
}

int main(int argc, char **argv) {
    int N = 512, M = 512;
    
    /* Dynamic bounds from arguments to prevent constant folding */
    if (argc > 1) N = atoi(argv[1]) % 400 + 100;
    if (argc > 2) M = atoi(argv[2]) % 400 + 100;
    
    printf("Starting OpenACC partitioning tests (N=%d, M=%d)\n", N, M);
    
    /* Initialize global declared data */
    #pragma acc update device(global_matrix, global_sum)
    
    /* First processing with conditional execution */
    process_data(N, M, argc > 0, argc);
    
    /* Second processing with 3D data */
    process_3d(64, argc);
    
    /* Final mixed region with simple reduction */
    int final_check = 0;
    #pragma acc parallel loop reduction(+:final_check) \
            copy(final_check) gang vector
    for (int i = 0; i < 1000; i++) {
        final_check += i * (argc % 10);
    }
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
