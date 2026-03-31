/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(float *data) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:N]) copyin(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < N; i++) {
            data[i] = i * 0.5f;
        }
        local_sum = 1.0f;  // Simple assignment in gang-redundant region
    }
    
    printf("Gang redundant test completed. data[0]=%.2f, data[%d]=%.2f\n", 
           data[0], N-1, data[N-1]);
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(float *data) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    printf("Gang partitioned test completed. sum=%.2f\n", sum);
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(float *data) {
    #pragma acc parallel copy(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < N) {
                    data[idx] = (i + j) * 1.5f;
                }
            }
        }
    }
    
    printf("Worker partitioned test completed. data[%d]=%.2f\n", 
           M*M-1, data[M*M-1]);
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(float *data) {
    #pragma acc parallel copy(data[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 3.0f - i * 0.1f;
        }
    }
    
    printf("Gang+worker partitioned test completed. data[0]=%.2f\n", data[0]);
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(float *data) {
    #pragma acc parallel loop vector copy(data[0:N])
    for (int i = 0; i < N; i++) {
        // Element-wise operations suitable for vectorization
        data[i] = data[i] * data[i] + 1.0f / (data[i] + 1.0f);
    }
    
    printf("Vector partitioned test completed. data[%d]=%.2f\n", N/2, data[N/2]);
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(float arr2d[M][M]) {
    #pragma acc parallel copy(arr2d[0:M][0:M])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = (i * j) / (float)M;
            }
        }
    }
    
    printf("Gang+vector partitioned test completed. arr2d[%d][%d]=%.2f\n", 
           M-1, M-1, arr2d[M-1][M-1]);
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(float arr2d[M][M]) {
    #pragma acc parallel copy(arr2d[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = arr2d[i][j] * 2.0f + (i - j) * 0.5f;
            }
        }
    }
    
    printf("Worker+vector partitioned test completed. arr2d[0][0]=%.2f\n", 
           arr2d[0][0]);
}

/* Test 8: Fully partitioned (case 7) - Complex nested computation */
void test_fully_partitioned(float arr3d[P][M][M]) {
    // Initialize 3D array
    #pragma acc parallel loop gang collapse(2) copy(arr3d[0:P][0:M][0:M])
    for (int k = 0; k < P; k++) {
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr3d[k][i][j] = (float)(k * M * M + i * M + j);
            }
        }
    }
    
    // Stencil-like computation with explicit partitioning
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M])
    {
        #pragma acc loop gang
        for (int k = 1; k < P-1; k++) {
            #pragma acc loop worker
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    // 3D stencil computation
                    arr3d[k][i][j] = (arr3d[k-1][i][j] + 
                                     arr3d[k][i-1][j] + 
                                     arr3d[k][i][j-1]) * 0.333f;
                }
            }
        }
    }
    
    printf("Fully partitioned test completed. arr3d[%d][%d][%d]=%.2f\n", 
           P/2, M/2, M/2, arr3d[P/2][M/2][M/2]);
}

/* Test 9: Mixed partitioning with runtime condition */
void test_mixed_partitioning(float *data, int use_gang) {
    if (use_gang) {
        #pragma acc parallel loop gang copy(data[0:N])
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 4.0f - 2.0f;
        }
    } else {
        #pragma acc parallel loop worker copy(data[0:N])
        for (int i = 0; i < N; i++) {
            data[i] = data[i] / 2.0f + 1.0f;
        }
    }
    
    printf("Mixed partitioning test completed. data[100]=%.2f\n", data[100]);
}

int main(int argc, char *argv[]) {
    // Allocate and initialize test arrays
    float *data1 = (float *)malloc(N * sizeof(float));
    float arr2d[M][M];
    float arr3d[P][M][M];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        data1[i] = (float)i / N;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = (float)(i + j);
        }
    }
    
    // Use command-line argument to control which tests run
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    // Execute tests based on input or all if no specific test
    switch (test_case) {
        case 0:
            test_gang_redundant(data1);
            break;
        case 1:
            test_gang_partitioned(data1);
            break;
        case 2:
            test_worker_partitioned(data1);
            break;
        case 3:
            test_gang_worker_partitioned(data1);
            break;
        case 4:
            test_vector_partitioned(data1);
            break;
        case 5:
            test_gang_vector_partitioned(arr2d);
            break;
        case 6:
            test_worker_vector_partitioned(arr2d);
            break;
        case 7:
            test_fully_partitioned(arr3d);
            break;
        case 8:
            test_mixed_partitioning(data1, argc > 2);
            break;
        default:
            // Run all tests to ensure all code paths are compiled
            test_gang_redundant(data1);
            test_gang_partitioned(data1);
            test_worker_partitioned(data1);
            test_gang_worker_partitioned(data1);
            test_vector_partitioned(data1);
            test_gang_vector_partitioned(arr2d);
            test_worker_vector_partitioned(arr2d);
            test_fully_partitioned(arr3d);
            test_mixed_partitioning(data1, 1);
            printf("All partitioning tests completed.\n");
            break;
    }
    
    // Print some results to prevent dead code elimination
    printf("Final check - data1[0]=%.4f, data1[%d]=%.4f\n", 
           data1[0], N-1, data1[N-1]);
    
    free(data1);
    return 0;
}
