/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare directive partitioning analysis */
#pragma acc declare create(global_matrix, global_scalar)
static int global_matrix[256][256];
static int global_scalar = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void process_data(int N, int M, int argc) {
    /* Multi-dimensional arrays with dynamic sizes to prevent constant folding */
    int (*src)[M] = malloc(N * sizeof(*src));
    int (*dst)[M] = malloc(N * sizeof(*dst));
    int tmp_buffer[N][M];
    
    /* Volatile to prevent optimization */
    volatile int seed = argc;
    int total_sum = 0;
    int row_sums[N];
    
    if (!src || !dst) {
        fprintf(stderr, "Allocation failed\n");
        free(src); free(dst);
        return;
    }
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            src[i][j] = (i * M + j) % 100;
            dst[i][j] = 0;
            tmp_buffer[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) \
                     copyin(tmp_buffer[0:N][0:M]) \
                     copy(total_sum, row_sums[0:N])
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario
           Mix of private, firstprivate, reduction clauses with gang/worker/vector */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_buffer) firstprivate(seed) reduction(+:total_sum)
        for (int i = 1; i < N-1; i++) {
            for (int j = 1; j < M-1; j++) {
                /* Conditional data access pattern */
                int tmp;
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with data-dependent access */
                    tmp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                } else if ((i * j) % 5 == 0) {
                    tmp = src[i][j] * 2;
                } else {
                    tmp = src[i][j] / 2;
                }
                
                /* Use private array element */
                tmp_buffer[i][j] = tmp;
                
                /* Conditional store with reduction */
                if (tmp > 50) {
                    dst[i][j] = tmp;
                    total_sum += tmp;
                }
                
                /* Access global declared data */
                global_matrix[i % 256][j % 256] += tmp % 10;
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach
           Only gang+vector partitioning, no worker level */
        #pragma acc kernels loop gang vector independent
        for (int i = 0; i < N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < M; j++) {
                /* Different access pattern */
                row_sum += dst[i][j];
                if (j % 2 == 0) {
                    dst[i][j] += src[i][j];
                }
            }
            row_sums[i] = row_sum;
            global_scalar += row_sum % 100;
        }
        
        /* THIRD REGION: Worker-level partitioning emphasized */
        #pragma acc parallel loop gang worker num_gangs(8) num_workers(4) \
                private(tmp_buffer) reduction(+:total_sum)
        for (int i = 0; i < N; i += 2) {
            int local_sum = 0;
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                /* Vector partitioned computation */
                int val = dst[i][j] - src[i][j];
                tmp_buffer[i][j] = val;
                local_sum += val;
                
                /* Conditional with complex expression */
                if (val > 0 && (i * j) % 7 == seed % 7) {
                    dst[i][j] = val * 3;
                }
            }
            total_sum += local_sum;
        }
    }
    
    /* OpenMP region outside OpenACC data region */
    int verify_sum = 0;
    #pragma omp parallel for reduction(+:verify_sum) schedule(dynamic)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            verify_sum += dst[i][j];
        }
    }
    
    printf("Total sum from reductions: %d\n", total_sum);
    printf("Verification sum: %d\n", verify_sum);
    printf("Global scalar: %d\n", global_scalar);
    
    /* Cleanup */
    free(src); free(dst);
}

/* Another function with different partitioning pattern */
void nested_partitioning(int size) {
    int matrix[size][size];
    int vector[size];
    
    /* Initialize */
    #pragma acc parallel loop gang
    for (int i = 0; i < size; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < size; j++) {
            matrix[i][j] = i * size + j;
        }
        vector[i] = 0;
    }
    
    /* Fully partitioned scenario (gang+worker+vector) */
    #pragma acc parallel loop gang worker vector collapse(2) \
            copy(matrix[0:size][0:size]) copyout(vector[0:size])
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Complex conditional with multiple branches */
            if (matrix[i][j] % 2 == 0) {
                matrix[i][j] *= 2;
            } else if (matrix[i][j] % 3 == 0) {
                matrix[i][j] /= 3;
            } else {
                matrix[i][j] += 1;
            }
            
            /* Reduction-like operation */
            #pragma acc atomic
            vector[i] += matrix[i][j] % 100;
        }
    }
}

int main(int argc, char **argv) {
    /* Dynamic bounds to prevent constant folding */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    if (N <= 0) N = 512;
    if (M <= 0) M = 512;
    
    printf("Running with N=%d, M=%d\n", N, M);
    
    /* Initialize global declared data */
    #pragma acc parallel loop collapse(2)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 50;
        }
    }
    
    /* Process with complex partitioning scenarios */
    process_data(N, M, argc);
    
    /* Additional partitioning pattern */
    if (argc > 3) {
        nested_partitioning(N % 256);
    }
    
    /* Final OpenMP region */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp critical
        printf("Thread %d completed\n", tid);
    }
    
    return 0;
}
