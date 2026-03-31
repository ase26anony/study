/* test_oacc_partition.c - OpenACC partitioning test for coverage */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr) {
    float local_val = 0.0f;
    
    #pragma acc parallel copy(arr[0:N]) copyin(local_val)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < N; i++) {
            arr[i] = local_val + i * 0.1f;
        }
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i] = i * 2.0f;
            sum += arr[i];
        }
    }
    
    printf("Gang partitioned sum: %f\n", sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (i + j) * 1.5f;
            }
        }
    }
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 2.0f;
            }
        }
    }
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr) {
    #pragma acc parallel copy(arr[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = arr[i] * 3.0f + 1.0f;
        }
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = (float)(i * j) / 100.0f;
            }
        }
    }
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] + (float)(i - j);
            }
        }
    }
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float arr3d[P][M][M]) {
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M])
    {
        #pragma acc loop gang
        for (int k = 1; k < P-1; k++) {
            #pragma acc loop worker
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    /* Stencil computation with data dependencies */
                    arr3d[k][i][j] = (arr3d[k-1][i][j] + 
                                     arr3d[k][i-1][j] + 
                                     arr3d[k][i][j-1]) * 0.333f;
                }
            }
        }
    }
}

/* Mixed partitioning with kernels directive */
void test_kernels_mixed(float *arr1, float arr2[M][M]) {
    #pragma acc kernels copy(arr1[0:N], arr2[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1[i] = i * 0.5f;
        }
        
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr1[i % N] + j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    float arr1[N];
    float arr2[M][M];
    float arr3d[P][M][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i + j);
        }
    }
    
    for (int k = 0; k < P; k++) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr3d[k][i][j] = (float)(k + i + j);
            }
        }
    }
    
    /* Use argc to control which tests run, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1);
            break;
        case 1:
            test_gang_partitioned(arr1);
            break;
        case 2:
            test_worker_partitioned(arr2);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            break;
        case 4:
            test_vector_partitioned(arr1);
            break;
        case 5:
            test_gang_vector_partitioned(arr2);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            break;
        case 7:
            test_fully_partitioned(arr3d);
            break;
        case 8:
            test_kernels_mixed(arr1, arr2);
            break;
        default:
            /* Run all tests in sequence to ensure all are compiled */
            if (argc > 1) {
                test_gang_redundant(arr1);
                test_gang_partitioned(arr1);
                test_worker_partitioned(arr2);
                test_gang_worker_partitioned(arr2);
                test_vector_partitioned(arr1);
                test_gang_vector_partitioned(arr2);
                test_worker_vector_partitioned(arr2);
                test_fully_partitioned(arr3d);
                test_kernels_mixed(arr1, arr2);
            }
            break;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: arr1[0]=%f, arr1[%d]=%f\n", arr1[0], N-1, arr1[N-1]);
    printf("arr2[0][0]=%f, arr2[%d][%d]=%f\n", arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
    printf("arr3d[0][0][0]=%f\n", arr3d[0][0][0]);
    
    return 0;
}
