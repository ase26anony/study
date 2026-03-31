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
void compute_kernel(int N, int M, int use_condition, int argc) {
    int i, j, k;
    volatile int dynamic_bound = argc; /* Prevent constant folding */
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp[512][512];
    int total_sum = 0;
    int row_sums[512] = {0};
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2) /* Mix OpenMP with OpenACC */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = i * M + j;
            dst[i][j] = 0;
            tmp[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src, dst, total_sum) copyin(tmp) create(row_sums)
    {
        /* FIRST COMPUTE REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:total_sum) \
                firstprivate(use_condition) /* Multiple data clauses */
        for (i = 1; i < N-1; i++) {
            for (j = 1; j < M-1; j++) {
                int temp = 0; /* private scalar */
                
                /* Conditional data access to force broadcast analysis */
                if (use_condition && (i % 2 == 0)) {
                    /* Stencil operation with neighbor access */
                    for (k = -1; k <= 1; k++) {
                        temp += src[i+k][j-1] + src[i+k][j] + src[i+k][j+1];
                    }
                    dst[i][j] = temp / 9;
                } else {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                    dst[i][j] = temp;
                }
                
                /* Reduction with conditional */
                if (dst[i][j] > 100) {
                    total_sum += dst[i][j];
                }
            }
        }
        
        /* SECOND COMPUTE REGION: Different partitioning (gang+vector only) */
        /* Should trigger different partitioning type analysis */
        #pragma acc kernels loop gang vector independent \
                private(j) reduction(+:global_sum)
        for (i = 0; i < N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                /* Access global device data */
                row_sum += dst[i][j];
                if (j % 3 == 0) {
                    global_matrix[i][j] = dst[i][j]; /* Use declared data */
                }
            }
            row_sums[i] = row_sum;
            global_sum += row_sum; /* Reduction on global variable */
        }
        
        /* THIRD REGION: Worker-level partitioning */
        /* Nested loops with worker parallelism */
        #pragma acc parallel loop gang worker num_gangs(4) num_workers(2) \
                firstprivate(dynamic_bound)
        for (i = 0; i < N; i += 2) {
            #pragma acc loop worker vector
            for (j = 0; j < M; j++) {
                /* Complex conditional with multiple branches */
                if (dynamic_bound % 2) {
                    tmp[i][j] = dst[i][j] * 3;
                    if (j > M/2) {
                        tmp[i+1][j] = dst[i][j] / 2;
                    }
                } else {
                    tmp[i][j] = dst[i][j] + row_sums[i % 512];
                }
            }
        }
    }
    
    /* Additional OpenMP region outside ACC data region */
    #pragma omp parallel for reduction(+:total_sum)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            total_sum += tmp[i][j] % 256;
        }
    }
    
    printf("Checksum: total_sum = %d, global_sum = %d\n", total_sum, global_sum);
}

/* Second function with different partitioning pattern */
void compute_fully_partitioned(int size, int argc) {
    int i, j, k;
    int matrix[128][128];
    int result[128][128];
    int sum = 0;
    
    /* Initialize */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i][j] = (i * j) % 97;
        }
    }
    
    /* This should potentially trigger "fully partitioned" analysis */
    #pragma acc data copy(matrix, result, sum)
    {
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:sum) \
                firstprivate(size, argc)
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                int val = matrix[i][j];
                
                /* Nested conditional with multiple private computations */
                for (k = 0; k < 3; k++) {
                    if ((i + j + k) % 4 == 0) {
                        val += k * argc; /* Use argc to prevent optimization */
                    }
                }
                
                /* Multiple assignment patterns */
                if (i > j) {
                    result[i][j] = val * 2;
                } else if (i < j) {
                    result[i][j] = val / 2;
                } else {
                    result[i][j] = val;
                }
                
                sum += result[i][j];
            }
        }
        
        /* Additional region with vector-only partitioning */
        #pragma acc kernels loop vector independent
        for (i = 0; i < size; i++) {
            int row_val = 0;
            #pragma acc loop vector reduction(+:row_val)
            for (j = 0; j < size; j++) {
                row_val += result[i][j];
            }
            matrix[i][0] = row_val;
        }
    }
    
    printf("Fully partitioned checksum: %d\n", sum);
}

int main(int argc, char **argv) {
    int N = 512;
    int M = 512;
    
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
    
    printf("Running OpenACC partitioning tests with N=%d, M=%d\n", N, M);
    
    /* Initialize global matrix */
    #pragma acc parallel loop collapse(2)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = i + j;
        }
    }
    
    /* Call compute kernels with different parameters */
    compute_kernel(N, M, argc % 2, argc);
    compute_fully_partitioned(128, argc);
    
    /* Final OpenMP region */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp critical
        printf("OpenMP thread %d completed\n", tid);
    }
    
    return 0;
}
