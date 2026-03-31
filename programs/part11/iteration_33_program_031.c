/* test_oacc_partition.c - Test OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *data, int size) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:size]) copyin(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < size; i++) {
            data[i] = i * 1.5f;
        }
        local_sum = 42.0f;  /* Simple assignment in gang-redundant region */
    }
    
    /* Use conditional to ensure compiler analyzes both paths */
    if (size > 0) {
        #pragma acc parallel present(data[0:size])
        {
            /* No loop - just gang redundant computation */
            data[0] += local_sum;
        }
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data, int size) {
    float sum = 0.0f;
    
    #pragma acc data copy(data[0:size]) create(sum)
    {
        #pragma acc parallel loop gang reduction(+:sum)
        for (int i = 0; i < size; i++) {
            data[i] = (float)i * 2.0f;
            sum += data[i];
        }
        
        /* Nested gang-partitioned region with data dependency */
        #pragma acc parallel loop gang
        for (int i = 1; i < size - 1; i++) {
            data[i] = (data[i-1] + data[i+1]) * 0.5f;
        }
    }
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float data[M][M]) {
    #pragma acc data copy(data[0:M][0:M])
    {
        /* Outer loop gang, inner loop worker partitioned */
        #pragma acc parallel loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                data[i][j] = (float)(i * M + j);
            }
        }
        
        /* Pure worker partitioned computation */
        float temp[M];
        #pragma acc parallel loop worker copy(temp[0:M])
        for (int k = 0; k < M; k++) {
            temp[k] = 0.0f;
            for (int j = 0; j < M; j++) {
                temp[k] += data[k][j];
            }
        }
    }
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[M][M]) {
    #pragma acc data copy(data[0:M][0:M])
    {
        /* Explicit gang+worker partitioning */
        #pragma acc parallel loop gang worker
        for (int i = 0; i < M * M; i++) {
            int row = i / M;
            int col = i % M;
            data[row][col] = (float)(row * col);
        }
        
        /* Nested gang+worker with reduction */
        float row_sums[M];
        #pragma acc parallel loop gang worker copy(row_sums[0:M])
        for (int i = 0; i < M; i++) {
            row_sums[i] = 0.0f;
            #pragma acc loop worker reduction(+:row_sums[i])
            for (int j = 0; j < M; j++) {
                row_sums[i] += data[i][j];
            }
        }
    }
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *data, int size) {
    #pragma acc data copy(data[0:size])
    {
        /* Vector partitioned loop with element-wise operations */
        #pragma acc parallel loop vector
        for (int i = 0; i < size; i++) {
            data[i] = data[i] * 3.14159f + 2.71828f;
        }
        
        /* Vector partitioned with stride */
        #pragma acc parallel loop vector
        for (int i = 0; i < size; i += 2) {
            data[i] = data[i] * 0.5f;
        }
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float data[M][M]) {
    #pragma acc data copy(data[0:M][0:M])
    {
        /* Gang+vector partitioning on 2D array */
        #pragma acc parallel loop gang vector
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = (float)(i + j) * 0.1f;
            }
        }
        
        /* Mixed partitioning with gang outer, vector inner */
        #pragma acc parallel loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] * data[i][j];
            }
        }
    }
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data[M][M]) {
    #pragma acc data copy(data[0:M][0:M])
    {
        /* Worker+vector explicit partitioning */
        #pragma acc parallel loop worker vector
        for (int i = 0; i < M * M; i++) {
            int idx = i;
            int row = idx / M;
            int col = idx % M;
            data[row][col] = (float)(row - col);
        }
        
        /* Nested worker+vector with private variables */
        #pragma acc parallel loop worker
        for (int i = 0; i < M; i++) {
            float private_sum = 0.0f;
            #pragma acc loop vector reduction(+:private_sum)
            for (int j = 0; j < M; j++) {
                private_sum += data[i][j];
            }
            data[i][0] = private_sum;
        }
    }
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float data[M][M][P]) {
    #pragma acc data copy(data[0:M][0:M][0:P])
    {
        /* Triple nested loop with explicit partitioning at all levels */
        #pragma acc parallel loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    data[i][j][k] = (float)(i * M * P + j * P + k);
                }
            }
        }
        
        /* Stencil computation requiring careful partitioning */
        #pragma acc parallel loop gang
        for (int i = 1; i < M - 1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M - 1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < P - 1; k++) {
                    /* 3D stencil - average of 6 neighbors */
                    data[i][j][k] = (data[i-1][j][k] + data[i+1][j][k] +
                                     data[i][j-1][k] + data[i][j+1][k] +
                                     data[i][j][k-1] + data[i][j][k+1]) / 6.0f;
                }
            }
        }
        
        /* Reduction across all dimensions with full partitioning */
        float global_sum = 0.0f;
        #pragma acc parallel loop gang reduction(+:global_sum)
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker reduction(+:global_sum)
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector reduction(+:global_sum)
                for (int k = 0; k < P; k++) {
                    global_sum += data[i][j][k];
                }
            }
        }
        data[0][0][0] = global_sum;  /* Store result */
    }
}

/* Helper to initialize arrays */
void init_array(float *arr, int size, float value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value;
    }
}

int main(int argc, char *argv[]) {
    /* Allocate test arrays */
    float arr1[N];
    float arr2[M][M];
    float arr3[M][M][P];
    
    /* Initialize arrays */
    init_array(arr1, N, 1.0f);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i + j);
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = (float)(i * j * k);
            }
        }
    }
    
    /* Use command-line argument to control which tests run */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Conditional execution to ensure all code paths are compiled */
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1, N);
            printf("Test 0 (gang redundant): arr1[0]=%f, arr1[%d]=%f\n", 
                   arr1[0], N-1, arr1[N-1]);
            break;
        case 1:
            test_gang_partitioned(arr1, N);
            printf("Test 1 (gang partitioned): arr1[0]=%f, arr1[%d]=%f\n",
                   arr1[0], N-1, arr1[N-1]);
            break;
        case 2:
            test_worker_partitioned(arr2);
            printf("Test 2 (worker partitioned): arr2[0][0]=%f, arr2[%d][%d]=%f\n",
                   arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            printf("Test 3 (gang+worker partitioned): arr2[0][0]=%f\n", arr2[0][0]);
            break;
        case 4:
            test_vector_partitioned(arr1, N);
            printf("Test 4 (vector partitioned): arr1[0]=%f, arr1[1]=%f\n",
                   arr1[0], arr1[1]);
            break;
        case 5:
            test_gang_vector_partitioned(arr2);
            printf("Test 5 (gang+vector partitioned): arr2[0][0]=%f\n", arr2[0][0]);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            printf("Test 6 (worker+vector partitioned): arr2[0][0]=%f\n", arr2[0][0]);
            break;
        case 7:
            test_fully_partitioned(arr3);
            printf("Test 7 (fully partitioned): arr3[0][0][0]=%f\n", arr3[0][0][0]);
            break;
        default:
            /* Run all tests to ensure all OpenACC constructs are processed */
            test_gang_redundant(arr1, N);
            test_gang_partitioned(arr1, N);
            test_worker_partitioned(arr2);
            test_gang_worker_partitioned(arr2);
            test_vector_partitioned(arr1, N);
            test_gang_vector_partitioned(arr2);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr3);
            
            /* Print results from all tests to prevent dead code elimination */
            printf("All tests executed:\n");
            printf("  arr1[0]=%f, arr1[%d]=%f\n", arr1[0], N-1, arr1[N-1]);
            printf("  arr2[0][0]=%f, arr2[%d][%d]=%f\n", 
                   arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
            printf("  arr3[0][0][0]=%f\n", arr3[0][0][0]);
            break;
    }
    
    return 0;
}
