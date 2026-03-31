/* test_oacc_partition.c - Exercise OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:N]) copy(sum)
        {
            #pragma acc loop gang(1)
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 2.0f;
            }
            sum = data[0];
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2.0f;
        }
        sum = data[0];
    }
    
    printf("Gang redundant: sum = %f, data[%d] = %f\n", sum, N-1, data[N-1]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int use_acc) {
    float data[N];
    float reduction_sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)(i % 100);
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:N]) copy(reduction_sum)
        {
            #pragma acc loop gang reduction(+:reduction_sum)
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 3.0f + (float)i;
                reduction_sum += data[i];
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 3.0f + (float)i;
            reduction_sum += data[i];
        }
    }
    
    printf("Gang partitioned: reduction_sum = %f, data[0] = %f\n", reduction_sum, data[0]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int use_acc) {
    float matrix[M][M];
    float worker_local[M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
        worker_local[i] = 0.0f;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:M][0:M]) create(worker_local[0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    worker_local[j] = matrix[i][j] * 0.5f;
                    matrix[i][j] = worker_local[j] + (float)(i + j);
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                worker_local[j] = matrix[i][j] * 0.5f;
                matrix[i][j] = worker_local[j] + (float)(i + j);
            }
        }
    }
    
    printf("Worker partitioned: matrix[%d][%d] = %f\n", M-1, M-1, matrix[M-1][M-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int use_acc) {
    float data[M][M];
    float temp[M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * j);
        }
        temp[i] = (float)i;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M]) copy(temp[0:M])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] + temp[i] - temp[j];
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] + temp[i] - temp[j];
            }
        }
    }
    
    printf("Gang+worker partitioned: data[0][0] = %f, data[%d][%d] = %f\n", 
           data[0][0], M-1, M-1, data[M-1][M-1]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int use_acc) {
    float vec1[N], vec2[N], result[N];
    
    for (int i = 0; i < N; i++) {
        vec1[i] = (float)i * 1.5f;
        vec2[i] = (float)(N - i) * 0.5f;
        result[i] = 0.0f;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(vec1[0:N], vec2[0:N], result[0:N])
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                result[i] = vec1[i] * vec2[i] + (float)(i % 10);
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            result[i] = vec1[i] * vec2[i] + (float)(i % 10);
        }
    }
    
    printf("Vector partitioned: result[0] = %f, result[%d] = %f\n", 
           result[0], N-1, result[N-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int use_acc) {
    float matrix[M][M];
    float row_sums[M];
    
    for (int i = 0; i < M; i++) {
        row_sums[i] = 0.0f;
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:M][0:M]) copy(row_sums[0:M])
        {
            #pragma acc loop gang vector
            for (int i = 0; i < M; i++) {
                float row_sum = 0.0f;
                #pragma acc loop reduction(+:row_sum)
                for (int j = 0; j < M; j++) {
                    matrix[i][j] = matrix[i][j] * 2.0f;
                    row_sum += matrix[i][j];
                }
                row_sums[i] = row_sum;
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            float row_sum = 0.0f;
            for (int j = 0; j < M; j++) {
                matrix[i][j] = matrix[i][j] * 2.0f;
                row_sum += matrix[i][j];
            }
            row_sums[i] = row_sum;
        }
    }
    
    printf("Gang+vector partitioned: row_sums[0] = %f, row_sums[%d] = %f\n", 
           row_sums[0], M-1, row_sums[M-1]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int use_acc) {
    float data[M][M];
    float col_avgs[M];
    
    for (int i = 0; i < M; i++) {
        col_avgs[i] = 0.0f;
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i + j * 2);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M]) copy(col_avgs[0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] / (float)M;
                    col_avgs[j] += data[i][j];
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] / (float)M;
                col_avgs[j] += data[i][j];
            }
        }
    }
    
    printf("Worker+vector partitioned: col_avgs[0] = %f, col_avgs[%d] = %f\n", 
           col_avgs[0], M-1, col_avgs[M-1]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int use_acc) {
    float grid[M][M][P];
    float result[M][M];
    
    /* Initialize 3D grid */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            result[i][j] = 0.0f;
            for (int k = 0; k < P; k++) {
                grid[i][j][k] = (float)(i * 100 + j * 10 + k);
            }
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(grid[0:M][0:M][0:P]) copy(result[0:M][0:M])
        {
            /* Triple nested loop with explicit partitioning */
            #pragma acc loop gang
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < M-1; j++) {
                    #pragma acc loop vector
                    for (int k = 1; k < P-1; k++) {
                        /* Stencil computation requiring all levels */
                        grid[i][j][k] = (grid[i-1][j][k] + grid[i][j-1][k] + 
                                        grid[i][j][k-1]) * 0.333f;
                        result[i][j] += grid[i][j][k];
                    }
                }
            }
        }
    } else {
        for (int i = 1; i < M-1; i++) {
            for (int j = 1; j < M-1; j++) {
                for (int k = 1; k < P-1; k++) {
                    grid[i][j][k] = (grid[i-1][j][k] + grid[i][j-1][k] + 
                                    grid[i][j][k-1]) * 0.333f;
                    result[i][j] += grid[i][j][k];
                }
            }
        }
    }
    
    printf("Fully partitioned: result[1][1] = %f, result[%d][%d] = %f\n", 
           result[1][1], M-2, M-2, result[M-2][M-2]);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(int use_acc) {
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.2f;
        c[i] = 0.0f;
    }
    
    if (use_acc) {
        #pragma acc kernels copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
            
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                c[i] = c[i] * 2.0f;
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
            c[i] = c[i] * 2.0f;
        }
    }
    
    printf("Kernels partitioning: c[0] = %f, c[%d] = %f\n", c[0], N-1, c[N-1]);
}

int main(int argc, char *argv[]) {
    int use_acc = 1;
    int test_case = 0;
    
    /* Use command line argument to control execution */
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (argc > 2) {
            use_acc = atoi(argv[2]);
        }
    }
    
    printf("Running OpenACC partitioning tests (use_acc=%d, test_case=%d)\n", 
           use_acc, test_case);
    
    /* Conditional execution to ensure all code paths are compiled */
    switch (test_case) {
        case 0:
            test_gang_redundant(use_acc);
            break;
        case 1:
            test_gang_partitioned(use_acc);
            break;
        case 2:
            test_worker_partitioned(use_acc);
            break;
        case 3:
            test_gang_worker_partitioned(use_acc);
            break;
        case 4:
            test_vector_partitioned(use_acc);
            break;
        case 5:
            test_gang_vector_partitioned(use_acc);
            break;
        case 6:
            test_worker_vector_partitioned(use_acc);
            break;
        case 7:
            test_fully_partitioned(use_acc);
            break;
        case 8:
            test_kernels_partitioning(use_acc);
            break;
        default:
            /* Run all tests */
            test_gang_redundant(use_acc);
            test_gang_partitioned(use_acc);
            test_worker_partitioned(use_acc);
            test_gang_worker_partitioned(use_acc);
            test_vector_partitioned(use_acc);
            test_gang_vector_partitioned(use_acc);
            test_worker_vector_partitioned(use_acc);
            test_fully_partitioned(use_acc);
            test_kernels_partitioning(use_acc);
            break;
    }
    
    return 0;
}
