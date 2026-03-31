/* test_oacc_partition.c - OpenACC partitioning test for coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(int use_acc) {
    float scalar = 1.0f;
    float arr[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    if (use_acc) {
        #pragma acc parallel copy(scalar, arr[0:N])
        {
            scalar = 3.14159f;
            #pragma acc loop gang(1)
            for (int i = 0; i < N; i++) {
                arr[i] = arr[i] * scalar;
            }
        }
    }
    
    printf("Gang redundant: scalar = %f, arr[0] = %f\n", scalar, arr[0]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    for (int i = 0; i < N; i++) data[i] = (float)(i + 1);
    
    if (use_acc) {
        #pragma acc parallel loop gang reduction(+:sum) copy(data[0:N])
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2.0f;
            sum += data[i];
        }
    }
    
    printf("Gang partitioned: sum = %f, data[N-1] = %f\n", sum, data[N-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int use_acc) {
    float matrix[M][M];
    float row_sums[M] = {0};
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:M][0:M], row_sums[0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                float row_sum = 0.0f;
                #pragma acc loop worker reduction(+:row_sum)
                for (int j = 0; j < M; j++) {
                    matrix[i][j] = matrix[i][j] * 0.5f;
                    row_sum += matrix[i][j];
                }
                row_sums[i] = row_sum;
            }
        }
    }
    
    printf("Worker partitioned: row_sums[0] = %f\n", row_sums[0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int use_acc) {
    float data[M][M];
    float block_sums[4][4] = {0};
    int block_size = M / 4;
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * M + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M], block_sums[0:4][0:4])
        {
            #pragma acc loop gang worker collapse(2)
            for (int bi = 0; bi < 4; bi++) {
                for (int bj = 0; bj < 4; bj++) {
                    float block_sum = 0.0f;
                    for (int i = bi * block_size; i < (bi + 1) * block_size; i++) {
                        for (int j = bj * block_size; j < (bj + 1) * block_size; j++) {
                            data[i][j] = data[i][j] * 1.5f;
                            block_sum += data[i][j];
                        }
                    }
                    block_sums[bi][bj] = block_sum;
                }
            }
        }
    }
    
    printf("Gang+worker partitioned: block_sums[0][0] = %f\n", block_sums[0][0]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int use_acc) {
    float vec1[N], vec2[N], result[N];
    
    for (int i = 0; i < N; i++) {
        vec1[i] = (float)i;
        vec2[i] = (float)(N - i);
    }
    
    if (use_acc) {
        #pragma acc parallel loop vector copyin(vec1[0:N], vec2[0:N]) copyout(result[0:N])
        for (int i = 0; i < N; i++) {
            result[i] = vec1[i] * vec2[i] + vec1[i] / (vec2[i] + 1.0f);
        }
    }
    
    printf("Vector partitioned: result[0] = %f, result[N-1] = %f\n", result[0], result[N-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int use_acc) {
    float matrix[M][M];
    float col_max[M];
    
    for (int i = 0; i < M; i++) {
        col_max[i] = -1.0e10f;
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)((i + 1) * (j + 1));
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:M][0:M], col_max[0:M])
        {
            #pragma acc loop gang
            for (int j = 0; j < M; j++) {
                float max_val = -1.0e10f;
                #pragma acc loop vector reduction(max:max_val)
                for (int i = 0; i < M; i++) {
                    matrix[i][j] = matrix[i][j] * 0.8f;
                    if (matrix[i][j] > max_val) max_val = matrix[i][j];
                }
                col_max[j] = max_val;
            }
        }
    }
    
    printf("Gang+vector partitioned: col_max[0] = %f\n", col_max[0]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int use_acc) {
    float data[M][M];
    float row_products[M];
    
    for (int i = 0; i < M; i++) {
        row_products[i] = 1.0f;
        for (int j = 0; j < M; j++) {
            data[i][j] = 1.0f + (float)(i * M + j) * 0.01f;
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M], row_products[0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                float product = 1.0f;
                #pragma acc loop worker vector reduction(*:product)
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] * 1.1f;
                    product *= data[i][j];
                }
                row_products[i] = product;
            }
        }
    }
    
    printf("Worker+vector partitioned: row_products[0] = %e\n", row_products[0]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int use_acc) {
    float volume[M][M][P];
    float slice_means[M];
    
    /* Initialize 3D volume */
    for (int i = 0; i < M; i++) {
        slice_means[i] = 0.0f;
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                volume[i][j][k] = (float)(i * M * P + j * P + k);
            }
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(volume[0:M][0:M][0:P], slice_means[0:M])
        {
            /* Triple nested loop with explicit partitioning */
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                float slice_sum = 0.0f;
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    float row_sum = 0.0f;
                    #pragma acc loop vector reduction(+:row_sum)
                    for (int k = 0; k < P; k++) {
                        /* Stencil-like computation to create dependencies */
                        float old_val = volume[i][j][k];
                        float neighbor_sum = 0.0f;
                        int count = 0;
                        
                        if (i > 0) { neighbor_sum += volume[i-1][j][k]; count++; }
                        if (j > 0) { neighbor_sum += volume[i][j-1][k]; count++; }
                        if (k > 0) { neighbor_sum += volume[i][j][k-1]; count++; }
                        if (i < M-1) { neighbor_sum += volume[i+1][j][k]; count++; }
                        if (j < M-1) { neighbor_sum += volume[i][j+1][k]; count++; }
                        if (k < P-1) { neighbor_sum += volume[i][j][k+1]; count++; }
                        
                        if (count > 0) {
                            volume[i][j][k] = (old_val + neighbor_sum) / (count + 1.0f);
                        }
                        row_sum += volume[i][j][k];
                    }
                    slice_sum += row_sum;
                }
                slice_means[i] = slice_sum / (M * P);
            }
        }
    }
    
    printf("Fully partitioned: slice_means[0] = %f, slice_means[M-1] = %f\n", 
           slice_means[0], slice_means[M-1]);
}

/* Main driver with conditional execution */
int main(int argc, char *argv[]) {
    int use_acc = 1;  /* Default: use OpenACC */
    int test_case = -1; /* Default: run all tests */
    
    /* Parse command line arguments */
    if (argc > 1) {
        use_acc = atoi(argv[1]);
    }
    if (argc > 2) {
        test_case = atoi(argv[2]);
    }
    
    printf("Running OpenACC partitioning tests (use_acc=%d, test_case=%d)\n", 
           use_acc, test_case);
    
    /* Conditional test execution based on command line */
    if (test_case == -1 || test_case == 0) test_gang_redundant(use_acc);
    if (test_case == -1 || test_case == 1) test_gang_partitioned(use_acc);
    if (test_case == -1 || test_case == 2) test_worker_partitioned(use_acc);
    if (test_case == -1 || test_case == 3) test_gang_worker_partitioned(use_acc);
    if (test_case == -1 || test_case == 4) test_vector_partitioned(use_acc);
    if (test_case == -1 || test_case == 5) test_gang_vector_partitioned(use_acc);
    if (test_case == -1 || test_case == 6) test_worker_vector_partitioned(use_acc);
    if (test_case == -1 || test_case == 7) test_fully_partitioned(use_acc);
    
    /* Force all functions to be compiled by referencing them */
    if (argc > 3) {
        /* This ensures all functions are considered by the compiler */
        void (*funcs[])(int) = {
            test_gang_redundant,
            test_gang_partitioned,
            test_worker_partitioned,
            test_gang_worker_partitioned,
            test_vector_partitioned,
            test_gang_vector_partitioned,
            test_worker_vector_partitioned,
            test_fully_partitioned
        };
        printf("Total test functions: %lu\n", sizeof(funcs)/sizeof(funcs[0]));
    }
    
    return 0;
}
