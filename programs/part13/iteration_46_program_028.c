/* test-omp-oacc-neuter-broadcast.c
 * Complex OpenACC program designed to trigger execution of the partitioning
 * type string lookup function in omp-oacc-neuter-broadcast.cc (lines 335-343).
 * Combines multiple OpenACC clauses, nested loops, mixed OpenMP, and
 * conditional data access to stress partitioning analysis.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Persistent device data - may influence partitioning decisions */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum;

/* Function with complex OpenACC compute regions */
void process_data(int N, int M, int argc) {
    /* Use argc to prevent constant folding */
    volatile int dyn_size = (argc > 1) ? atoi("256") : 128;
    int actual_N = (N < dyn_size) ? N : dyn_size;
    int actual_M = (M < dyn_size) ? M : dyn_size;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int partial_sums[512] = {0};
    int total_sum = 0;
    int tmp; /* For private clause */
    
    /* Initialize arrays */
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = (i * actual_M + j) % 97;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region for host-device data management */
    #pragma acc data copy(src, dst) copyout(partial_sums) copy(total_sum)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning */
        /* gang+worker+vector with private and reduction clauses */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp) reduction(+:total_sum)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Conditional data access - influences partitioning */
                if ((i + j) % 3 == 0) {
                    /* Stencil computation with private variable */
                    tmp = src[i-1][j] + src[i+1][j] + 
                          src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                } else if ((i + j) % 3 == 1) {
                    /* Different access pattern */
                    tmp = src[i][j] * 2;
                    dst[i][j] = tmp - src[i][j];
                } else {
                    /* Yet another pattern */
                    dst[i][j] = src[i][j];
                }
                
                /* Reduction with conditional */
                if (dst[i][j] > 50) {
                    total_sum += dst[i][j];
                }
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning strategy */
        /* Only gang+vector partitioning (no worker) */
        #pragma acc kernels loop gang vector
        for (int i = 0; i < actual_N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < actual_M; j++) {
                /* Access global device data */
                row_sum += dst[i][j] + global_matrix[i % 256][j % 256];
            }
            partial_sums[i] = row_sum;
        }
        
        /* THIRD: Worker-only partitioning scenario */
        /* Using firstprivate to force partitioning analysis */
        int block_size = 16;
        #pragma acc parallel loop worker firstprivate(block_size)
        for (int i = 0; i < actual_N; i += block_size) {
            int local_sum = 0;
            int end_i = (i + block_size < actual_N) ? i + block_size : actual_N;
            #pragma acc loop vector
            for (int k = i; k < end_i; k++) {
                local_sum += partial_sums[k];
            }
            /* Atomic update to global device variable */
            #pragma acc atomic update
            global_sum += local_sum;
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    /* This stresses compiler's multi-paradigm handling */
    int verify_array[512];
    #pragma omp parallel for
    for (int i = 0; i < actual_N; i++) {
        verify_array[i] = 0;
        for (int j = 0; j < actual_M; j++) {
            verify_array[i] += dst[i][j];
        }
    }
    
    /* Final verification output */
    int final_check = 0;
    for (int i = 0; i < actual_N; i++) {
        final_check += verify_array[i];
    }
    printf("Checksum: total_sum=%d, partial=%d, global=%d, final=%d\n", 
           total_sum, partial_sums[0], global_sum, final_check);
}

/* Helper function with another OpenACC region */
void process_global_data(int size) {
    int local_temp[256];
    
    /* Initialize global matrix on device */
    #pragma acc parallel loop gang vector collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i * j) % 31;
        }
    }
    
    /* Fully partitioned scenario (gang+worker+vector) */
    #pragma acc parallel loop gang worker vector collapse(2) \
            private(local_temp) copy(global_sum)
    for (int i = 0; i < size && i < 256; i++) {
        for (int j = 0; j < size && j < 256; j++) {
            /* Complex private array usage */
            for (int k = 0; k < 16; k++) {
                local_temp[k] = global_matrix[i][j] * k;
            }
            
            int elem_sum = 0;
            for (int k = 0; k < 16; k++) {
                elem_sum += local_temp[k];
            }
            
            #pragma acc atomic update
            global_sum += elem_sum;
        }
    }
}

int main(int argc, char **argv) {
    /* Dynamic bounds based on arguments to prevent optimization */
    int N = (argc > 1) ? atoi(argv[1]) : 512;
    int M = (argc > 2) ? atoi(argv[2]) : 512;
    
    if (N <= 0) N = 256;
    if (M <= 0) M = 256;
    
    printf("Processing %dx%d matrix...\n", N, M);
    
    /* Initialize global device data */
    global_sum = 0;
    #pragma acc enter data copyin(global_matrix, global_sum)
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, argc);
    
    /* Additional processing with different dimensions */
    process_global_data(N / 2);
    
    #pragma acc exit data delete(global_matrix) copyout(global_sum)
    
    printf("Final global sum: %d\n", global_sum);
    
    return 0;
}
