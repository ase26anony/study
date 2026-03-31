/* test-openacc-partitioning.c
 * 
 * Complex OpenACC program designed to trigger execution of the partitioning
 * type string lookup function in omp-oacc-neuter-broadcast.cc (lines 335-343).
 * Uses multiple OpenACC constructs with explicit data clauses and partitioning
 * to force compiler analysis of data distribution strategies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data - may influence partitioning decisions */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data access patterns */
void compute_kernels(int N, int M, int use_condition, int argc) {
    int i, j, k;
    int local_sum = 0;
    
    /* 2D arrays for stencil operations */
    int (*src)[M] = malloc(N * sizeof(*src));
    int (*dst)[M] = malloc(N * sizeof(*dst));
    
    if (!src || !dst) {
        fprintf(stderr, "Allocation failed\n");
        exit(1);
    }
    
    /* Initialize arrays with volatile-like pattern to prevent optimization */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = (i + j) % 100;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) copyin(N, M, use_condition) copy(local_sum)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario */
        /* Uses gang+worker+vector with private, firstprivate, and reduction */
        printf("Starting first parallel region with gang+worker+vector partitioning\n");
        
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) firstprivate(use_condition) reduction(+:local_sum) \
                present(src, dst, N, M)
        for (i = 1; i < N-1; i++) {
            for (j = 1; j < M-1; j++) {
                int temp = 0;  /* private variable */
                
                /* Conditional data access pattern - may affect broadcasting */
                if (use_condition && (i % 2 == 0)) {
                    /* Stencil computation with data-dependent access */
                    for (k = -1; k <= 1; k++) {
                        temp += src[i+k][j] + src[i][j+k];
                    }
                    dst[i][j] = temp / 5;
                } else {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                    dst[i][j] = temp;
                }
                
                /* Reduction with conditional */
                if (dst[i][j] > 50) {
                    local_sum += dst[i][j];
                }
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach */
        /* Uses only gang+vector partitioning (no worker) */
        printf("Starting kernels region with gang+vector partitioning\n");
        
        #pragma acc kernels loop gang vector \
                private(j) reduction(+:local_sum) \
                present(src, dst, N, M)
        for (i = 0; i < N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                /* Another conditional pattern */
                if (argc > 2 || (i * j) % 7 == 0) {
                    row_sum += src[i][j];
                }
            }
            dst[i][0] = row_sum;  /* Store row sum in first column */
            local_sum += row_sum;
        }
        
        /* THIRD: Nested loops with separate directives */
        /* Outer loop gang partitioned, inner loop vector partitioned */
        printf("Starting nested directive region\n");
        
        #pragma acc parallel loop gang present(src, dst, N, M)
        for (i = 0; i < N; i += 2) {
            #pragma acc loop worker vector private(j)
            for (j = 0; j < M; j++) {
                /* Access global device data */
                dst[i][j] += global_matrix[i % 256][j % 256];
            }
        }
    }
    
    /* PURE OPENMP REGION: Mix pragma types to stress compiler */
    printf("Starting OpenMP region (mixing pragmas)\n");
    
    #pragma omp parallel for private(i, j) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Post-processing on CPU */
            dst[i][j] = dst[i][j] % 1000;
            local_sum += dst[i][j];
        }
    }
    
    printf("Local sum: %d\n", local_sum);
    global_sum += local_sum;
    
    free(src);
    free(dst);
}

/* Another function with different partitioning pattern */
void compute_reductions(int size, int argc) {
    int i, j;
    int matrix[100][100];
    int total = 0;
    
    /* Initialize */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix[i][j] = (i * j + argc) % 100;
        }
    }
    
    /* OpenACC with gang partitioned reduction */
    #pragma acc data copy(matrix, total)
    {
        /* Fully partitioned scenario (gang+worker+vector) */
        #pragma acc parallel loop gang worker vector collapse(2) \
                reduction(+:total) copyin(size)
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                /* Complex conditional reduction */
                if ((i + j) % size == 0) {
                    total += matrix[i][j];
                } else if ((i * j) % 7 == 0) {
                    total -= matrix[i][j] / 2;
                }
            }
        }
        
        /* Worker+vector partitioned section */
        #pragma acc parallel loop worker vector \
                reduction(max:total) present(matrix)
        for (i = 0; i < 100; i++) {
            int row_max = 0;
            #pragma acc loop vector reduction(max:row_max)
            for (j = 0; j < 100; j++) {
                if (matrix[i][j] > row_max) {
                    row_max = matrix[i][j];
                }
            }
            if (row_max > total) {
                total = row_max;
            }
        }
    }
    
    printf("Reduction total: %d\n", total);
}

int main(int argc, char **argv) {
    /* Dynamic bounds to prevent constant folding */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    int use_condition = argc > 3 ? atoi(argv[3]) : 1;
    
    printf("Starting OpenACC partitioning test with N=%d, M=%d\n", N, M);
    
    /* Initialize global device data */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j * 3) % 100;
        }
    }
    
    /* Call functions with different partitioning scenarios */
    compute_kernels(N > 1000 ? 1000 : N, M > 1000 ? 1000 : M, use_condition, argc);
    compute_reductions(N % 20 + 1, argc);
    
    /* Final verification output */
    printf("Global checksum: %d\n", global_sum);
    printf("Test completed successfully\n");
    
    return 0;
}
