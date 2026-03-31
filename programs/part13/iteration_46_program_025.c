#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare directive analysis */
#pragma acc declare create(global_matrix)
static int global_matrix[256][256];

/* Function with complex OpenACC compute regions */
void process_data(int N, int M, int argc) {
    int src[512][512];
    int dst[512][512];
    int tmp_matrix[256][256];
    int total_sum = 0;
    int partial_sums[16] = {0};
    volatile int dynamic_bound = (argc > 1) ? atoi("64") : 128; /* Prevent constant folding */
    
    /* Initialize source arrays with pattern */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            src[i][j] = i * M + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with complex partitioning scenarios */
    #pragma acc data copy(src, dst) copyin(dynamic_bound) copyout(total_sum)
    {
        /* FIRST PARALLEL REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_matrix) reduction(+:total_sum) \
                firstprivate(dynamic_bound) async(1)
        for (int i = 1; i < N-1; i++) {
            for (int j = 1; j < M-1; j++) {
                /* Local temporary with conditional access pattern */
                int temp = 0;
                
                /* Stencil operation with conditional */
                if ((i + j) % 3 == 0) {
                    temp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                } else if ((i + j) % 3 == 1) {
                    temp = src[i][j] * 2;
                } else {
                    temp = src[i][j] / 2;
                }
                
                /* Conditional store with reduction */
                if (temp > dynamic_bound) {
                    dst[i][j] = temp;
                    total_sum += temp;
                } else {
                    dst[i][j] = src[i][j];
                    total_sum += src[i][j];
                }
                
                /* Access global declared data */
                global_matrix[i % 256][j % 256] += (i * j) % 7;
            }
        }
        
        #pragma acc wait
        
        /* SECOND PARALLEL REGION: Different partitioning - gang+vector only */
        /* Should trigger different partitioning type analysis */
        int row_sums[512] = {0};
        #pragma acc parallel loop gang vector \
                private(partial_sums) copy(row_sums)
        for (int i = 0; i < N; i++) {
            int local_sum = 0;
            #pragma acc loop vector reduction(+:local_sum)
            for (int j = 0; j < M; j++) {
                /* Conditional reduction with array access */
                if (dst[i][j] % 2 == 0) {
                    local_sum += dst[i][j];
                } else {
                    local_sum -= dst[i][j] / 2;
                }
            }
            row_sums[i] = local_sum;
            
            /* Nested conditional with worker-level parallelism hint */
            if (i % 4 == 0) {
                #pragma acc atomic update
                partial_sums[i % 16] += local_sum;
            }
        }
        
        /* THIRD REGION: Kernels directive with automatic partitioning */
        /* Forces compiler to determine optimal partitioning */
        int final_sum = 0;
        #pragma acc kernels loop reduction(+:final_sum) \
                copyin(row_sums) copy(final_sum)
        for (int i = 0; i < N; i += 2) {
            int pair_sum = 0;
            for (int j = 0; j < M; j += 2) {
                /* 2x2 block reduction with conditional */
                if (i + 1 < N && j + 1 < M) {
                    pair_sum += dst[i][j] + dst[i+1][j] + 
                               dst[i][j+1] + dst[i+1][j+1];
                }
            }
            final_sum += pair_sum * row_sums[i % 512];
        }
        
        total_sum += final_sum;
    }
    
    /* Mixed OpenMP outside OpenACC region */
    int host_array[1000];
    #pragma omp parallel for private(tmp_matrix)
    for (int i = 0; i < 1000; i++) {
        host_array[i] = i * total_sum % 100;
    }
    
    printf("Total checksum: %d\n", total_sum);
}

/* Another function with different access patterns */
void nested_partitioning(int size, int iter) {
    int A[128][128], B[128][128], C[128][128];
    
    /* Initialize */
    #pragma acc parallel loop collapse(2) copy(A, B)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i][j] = (i + j) % 10;
            B[i][j] = (i * j) % 10;
        }
    }
    
    /* Matrix multiply with explicit partitioning clauses */
    #pragma acc parallel loop gang collapse(1) \
            copyin(A, B) copyout(C)
    for (int i = 0; i < size; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < size; j++) {
            int sum = 0;
            #pragma acc loop seq
            for (int k = 0; k < size; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
            
            /* Access global data with conditional */
            if ((i + j + iter) % 5 == 0) {
                #pragma acc atomic update
                global_matrix[i % 256][j % 256] += sum % 100;
            }
        }
    }
    
    /* Reduction with worker partitioning */
    int matrix_sum = 0;
    #pragma acc parallel loop gang worker reduction(+:matrix_sum) \
            copyin(C) copy(matrix_sum)
    for (int i = 0; i < size; i++) {
        int row_sum = 0;
        #pragma acc loop vector reduction(+:row_sum)
        for (int j = 0; j < size; j++) {
            row_sum += C[i][j];
        }
        matrix_sum += row_sum;
    }
    
    printf("Matrix sum after iteration %d: %d\n", iter, matrix_sum);
}

int main(int argc, char **argv) {
    /* Dynamic bounds to prevent compile-time optimization */
    int N = (argc > 1) ? atoi(argv[1]) : 512;
    int M = (argc > 2) ? atoi(argv[2]) : 512;
    
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    
    printf("Processing with N=%d, M=%d\n", N, M);
    
    /* Initialize global matrix */
    #pragma acc parallel loop collapse(2) copyout(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = 0;
        }
    }
    
    /* Process with complex partitioning */
    process_data(N, M, argc);
    
    /* Additional calls with different parameters */
    for (int iter = 0; iter < 3; iter++) {
        nested_partitioning(64 + iter * 16, iter);
    }
    
    /* Final verification with mixed pragmas */
    int final_check = 0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            final_check += global_matrix[i][j];
        }
    }
    
    printf("Final global matrix checksum: %d\n", final_check);
    
    return 0;
}
