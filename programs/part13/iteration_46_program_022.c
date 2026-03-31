/* test-openacc-partitioning.c
 * 
 * Complex OpenACC program designed to trigger execution of the partitioning
 * type string lookup function in omp-oacc-neuter-broadcast.cc (lines 335-343).
 * Uses multiple OpenACC constructs with explicit data clauses, nested loops,
 * conditional access patterns, and mixed OpenMP pragmas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Persistent device data - may influence partitioning decisions */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with complex OpenACC parallel region */
void compute_stencil(int N, int M, int src[N][M], int dst[N][M], int *total_sum) {
    int i, j;
    
    /* Complex parallel region with explicit partitioning clauses */
    #pragma acc parallel loop gang worker vector collapse(2) \
        private(i, j) firstprivate(N, M) reduction(+:*total_sum) \
        copyin(src[0:N][0:M]) copyout(dst[0:N][0:M])
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            int temp;
            
            /* Conditional data access pattern */
            if ((i + j) % 3 == 0) {
                /* Stencil computation with data-dependent access */
                temp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                dst[i][j] = temp / 4;
            } else if ((i * j) % 5 == 0) {
                /* Different access pattern */
                temp = src[i][j] * 2 - src[i-1][j-1];
                dst[i][j] = temp;
            } else {
                /* Simple copy */
                dst[i][j] = src[i][j];
            }
            
            /* Reduction with conditional */
            if (dst[i][j] > 100) {
                *total_sum += dst[i][j];
            }
        }
    }
}

/* Second function with different partitioning approach */
void compute_row_sums(int N, int M, int matrix[N][M], int row_sums[N]) {
    int i, j;
    
    /* Kernels region with gang+vector partitioning (no worker) */
    #pragma acc kernels loop gang vector \
        private(j) reduction(+:row_sums[0:N]) \
        copy(matrix[0:N][0:M])
    for (i = 0; i < N; i++) {
        int row_sum = 0;
        #pragma acc loop vector
        for (j = 0; j < M; j++) {
            /* Access global device data */
            row_sum += matrix[i][j] + global_matrix[i % 256][j % 256];
        }
        row_sums[i] = row_sum;
    }
}

/* Function using OpenMP (mixed pragmas) */
void host_initialization(int N, int *host_array) {
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        host_array[i] = i * 2;
    }
}

int main(int argc, char *argv[]) {
    /* Dynamic sizing to prevent constant folding */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N < 10) N = 512;
    if (M < 10) M = 512;
    
    /* Allocate 2D arrays */
    int (*src)[M] = malloc(N * sizeof(*src));
    int (*dst)[M] = malloc(N * sizeof(*dst));
    int *row_sums = malloc(N * sizeof(int));
    int *host_array = malloc(N * sizeof(int));
    
    if (!src || !dst || !row_sums || !host_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            src[i][j] = (i + j) % 100;
            dst[i][j] = 0;
        }
        row_sums[i] = 0;
    }
    
    /* Initialize global matrix on device */
    #pragma acc parallel loop collapse(2) copyout(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i * j) % 50;
        }
    }
    
    int total_sum = 0;
    
    /* OpenACC data region with complex data management */
    #pragma acc data copyin(src[0:N][0:M]) copyout(dst[0:N][0:M], row_sums[0:N]) \
        copy(total_sum) create(host_array[0:N])
    {
        /* First compute: stencil with gang+worker+vector partitioning */
        compute_stencil(N, M, src, dst, &total_sum);
        
        /* Second compute: row sums with gang+vector partitioning */
        compute_row_sums(N, M, dst, row_sums);
        
        /* Mixed OpenMP/OpenACC: host computation inside data region */
        #pragma acc host_data use_device(host_array)
        {
            /* This host call with device data may trigger special handling */
            host_initialization(N, host_array);
        }
        
        /* Additional parallel region with worker+vector partitioning */
        int block_sum = 0;
        #pragma acc parallel loop worker vector reduction(+:block_sum) \
            copyin(dst[0:N][0:M])
        for (int i = 0; i < N; i += 16) {
            for (int j = 0; j < M; j += 16) {
                int block_avg = 0;
                int count = 0;
                
                /* Nested loops inside parallel region */
                for (int ii = i; ii < i + 16 && ii < N; ii++) {
                    for (int jj = j; jj < j + 16 && jj < M; jj++) {
                        block_avg += dst[ii][jj];
                        count++;
                    }
                }
                
                if (count > 0) {
                    block_sum += block_avg / count;
                }
            }
        }
        
        total_sum += block_sum;
        
        /* Fully partitioned example: each element processed independently */
        int max_val = 0;
        #pragma acc parallel loop gang worker vector reduction(max:max_val) \
            copyin(dst[0:N][0:M])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (dst[i][j] > max_val) {
                    max_val = dst[i][j];
                }
            }
        }
    }
    
    /* Pure OpenMP region outside OpenACC data region */
    host_initialization(N, host_array);
    
    /* Verification and output */
    int final_check = 0;
    for (int i = 0; i < N; i++) {
        final_check += row_sums[i] % 1000;
    }
    
    printf("Total sum: %d\n", total_sum);
    printf("Row sums checksum: %d\n", final_check);
    printf("Max value in dst: %d\n", dst[N/2][M/2]);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(row_sums);
    free(host_array);
    
    return 0;
}
