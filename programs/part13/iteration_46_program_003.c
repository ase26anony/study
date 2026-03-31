/* test_openacc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare analysis */
#pragma acc declare create(global_matrix)
static int global_matrix[256][256];

/* Function with complex OpenACC compute regions */
void compute_regions(int N, int M, int argc) {
    /* Multi-dimensional arrays with dynamic sizes */
    int (*src)[M] = malloc(N * sizeof(*src));
    int (*dst)[M] = malloc(N * sizeof(*dst));
    int temp_sum = 0;
    volatile int vol_N = N; /* Prevent constant folding */
    
    /* Initialize arrays */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < vol_N; i++) {
        for (int j = 0; j < M; j++) {
            src[i][j] = i * M + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) copyout(temp_sum)
    {
        /* FIRST OPENACC REGION: Complex partitioning with all levels */
        /* This should trigger gang+worker+vector partitioning analysis */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(temp_sum) reduction(+:temp_sum) \
                firstprivate(N, M) async(1)
        for (int i = 1; i < N-1; i++) {
            for (int j = 1; j < M-1; j++) {
                int tmp; /* Private variable */
                
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with reduction */
                    tmp = src[i-1][j] + src[i+1][j] + 
                          src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                } else if ((i + j) % 3 == 1) {
                    /* Different access pattern */
                    tmp = src[i][j] * 2;
                    dst[i][j] = tmp - src[i][j];
                } else {
                    /* Yet another pattern */
                    tmp = src[i][j];
                    dst[i][j] = tmp + 1;
                }
                
                /* Reduction with conditional */
                if (dst[i][j] > 100) {
                    temp_sum += dst[i][j];
                }
            }
        }
        
        #pragma acc wait
        
        /* SECOND OPENACC REGION: Different partitioning strategy */
        /* gang+vector partitioned without worker */
        int row_sums[N];
        #pragma acc kernels loop gang vector \
                private(temp_sum) copy(row_sums[0:N])
        for (int i = 0; i < N; i++) {
            temp_sum = 0;
            #pragma acc loop vector reduction(+:temp_sum)
            for (int j = 0; j < M; j++) {
                /* Access global device data */
                temp_sum += dst[i][j] + global_matrix[i % 256][j % 256];
            }
            row_sums[i] = temp_sum;
            
            /* Nested conditional with array access */
            if (i % 2 == 0) {
                #pragma acc loop vector
                for (int j = 0; j < M; j += 2) {
                    dst[i][j] = row_sums[i] / M;
                }
            }
        }
        
        /* THIRD REGION: worker+vector partitioned scenario */
        int col_sums[M];
        #pragma acc parallel loop worker vector \
                firstprivate(N, M) copy(col_sums[0:M])
        for (int j = 0; j < M; j++) {
            int col_total = 0;
            #pragma acc loop vector reduction(+:col_total)
            for (int i = 0; i < N; i++) {
                col_total += dst[i][j];
            }
            col_sums[j] = col_total;
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    int verify_sum = 0;
    #pragma omp parallel for reduction(+:verify_sum) schedule(dynamic)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            verify_sum += dst[i][j];
        }
    }
    
    printf("Verification checksum: %d\n", verify_sum);
    printf("Temp sum from device: %d\n", temp_sum);
    
    free(src);
    free(dst);
}

/* Another function with different partitioning patterns */
void nested_partitioning(int size) {
    int matrix[size][size];
    int vector[size];
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        vector[i] = i;
        for (int j = 0; j < size; j++) {
            matrix[i][j] = i * size + j;
        }
    }
    
    /* Fully partitioned scenario (case 7) */
    #pragma acc parallel loop collapse(2) gang worker vector \
            reduction(+:result) copy(matrix, vector, result)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Complex conditional with private computation */
            int private_val = matrix[i][j] * vector[i];
            if (private_val % 7 == 0) {
                matrix[i][j] = private_val / 7;
                result += private_val;
            } else if (private_val % 5 == 0) {
                matrix[i][j] = private_val / 5;
                result += private_val * 2;
            } else {
                matrix[i][j] = private_val;
            }
        }
    }
    
    /* Gang partitioned only (case 1) */
    int partial_sums[16] = {0};
    #pragma acc parallel loop gang num_gangs(16) \
            copy(partial_sums) firstprivate(size)
    for (int g = 0; g < 16; g++) {
        int gang_sum = 0;
        int start = g * size / 16;
        int end = (g + 1) * size / 16;
        
        #pragma acc loop worker vector reduction(+:gang_sum)
        for (int i = start; i < end; i++) {
            for (int j = 0; j < size; j++) {
                gang_sum += matrix[i][j];
            }
        }
        partial_sums[g] = gang_sum;
    }
    
    printf("Nested partitioning result: %d\n", result);
}

int main(int argc, char *argv[]) {
    /* Dynamic sizes based on arguments to prevent constant folding */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    
    /* Initialize global device matrix */
    #pragma acc parallel loop collapse(2) copy(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 256;
        }
    }
    
    printf("Starting OpenACC partitioning tests...\n");
    printf("Matrix size: %d x %d\n", N, M);
    
    /* Call functions with different partitioning scenarios */
    compute_regions(N, M, argc);
    
    if (argc > 3) {
        int size = atoi(argv[3]);
        if (size > 0 && size <= 1024) {
            nested_partitioning(size);
        }
    }
    
    /* Final OpenMP region */
    int final_check = 0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < 1000; i++) {
        final_check += i * omp_get_thread_num();
    }
    
    printf("Final check: %d\n", final_check);
    printf("Tests completed.\n");
    
    return 0;
}
