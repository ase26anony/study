/* test_oacc_partition.c - OpenACC partitioning coverage test */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyout(sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5f;
        }
        sum = arr[0] + arr[n-1];
    }
    
    if (sum < 0) printf("unreachable\n"); /* prevent dead code elimination */
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float reduction_sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:n]) reduction(+:reduction_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i;
        reduction_sum += arr[i];
    }
    
    if (reduction_sum < 0) printf("unreachable\n");
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (float)(i * M + j);
            }
        }
    }
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M]) {
    float tile_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:M][0:M]) reduction(+:tile_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 0.5f;
                tile_sum += arr[i][j];
            }
        }
    }
    
    if (tile_sum < 0) printf("unreachable\n");
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        /* Element-wise operation suitable for vectorization */
        arr[i] = arr[i] * arr[i] - 2.0f * arr[i] + 1.0f;
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *arr, int n) {
    float partial_sum = 0.0f;
    
    #pragma acc parallel loop gang vector copy(arr[0:n]) reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = (arr[i] > 0) ? arr[i] : -arr[i];
        partial_sum += arr[i];
    }
    
    if (partial_sum < 0) printf("unreachable\n");
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                /* Stencil-like computation with data dependencies */
                float left = (j > 0) ? arr[i][j-1] : 0.0f;
                float up = (i > 0) ? arr[i-1][j] : 0.0f;
                arr[i][j] = (left + up) * 0.5f + 1.0f;
            }
        }
    }
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr[M][M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < M-1; k++) {
                    /* 3D stencil computation requiring all levels */
                    arr[i][j][k] = (arr[i-1][j][k] + arr[i+1][j][k] +
                                   arr[i][j-1][k] + arr[i][j+1][k] +
                                   arr[i][j][k-1] + arr[i][j][k+1]) / 6.0f;
                }
            }
        }
    }
}

/* Mixed partitioning with runtime condition */
void test_mixed_partitioning(float *arr1, float arr2[M][M], int n, int mode) {
    if (mode > 0) {
        /* This path forces analysis of gang partitioning */
        #pragma acc parallel loop gang copy(arr1[0:n])
        for (int i = 0; i < n; i++) {
            arr1[i] = arr1[i] * 3.14f;
        }
    } else {
        /* This path forces analysis of vector partitioning */
        #pragma acc parallel loop vector copy(arr2[0:M][0:M])
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr2[i][j] / 2.0f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test arrays */
    float *arr1d = (float*)malloc(N * sizeof(float));
    float arr2d[M][M];
    float arr3d[M][M][M];
    
    for (int i = 0; i < N; i++) {
        arr1d[i] = (float)i / N;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = (float)(i + j);
            for (int k = 0; k < M; k++) {
                arr3d[i][j][k] = (float)(i + j + k);
            }
        }
    }
    
    /* Use command-line argument to control which tests run */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1d, N);
            break;
        case 1:
            test_gang_partitioned(arr1d, N);
            break;
        case 2:
            test_worker_partitioned(arr2d);
            break;
        case 3:
            test_gang_worker_partitioned(arr2d);
            break;
        case 4:
            test_vector_partitioned(arr1d, N);
            break;
        case 5:
            test_gang_vector_partitioned(arr1d, N);
            break;
        case 6:
            test_worker_vector_partitioned(arr2d);
            break;
        case 7:
            test_fully_partitioned(arr3d);
            break;
        default:
            /* Run all tests to ensure all OpenACC constructs are compiled */
            test_gang_redundant(arr1d, N);
            test_gang_partitioned(arr1d, N);
            test_worker_partitioned(arr2d);
            test_gang_worker_partitioned(arr2d);
            test_vector_partitioned(arr1d, N);
            test_gang_vector_partitioned(arr1d, N);
            test_worker_vector_partitioned(arr2d);
            test_fully_partitioned(arr3d);
            test_mixed_partitioning(arr1d, arr2d, N, argc);
            break;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: arr1d[0]=%.2f, arr1d[%d]=%.2f\n", 
           arr1d[0], N-1, arr1d[N-1]);
    printf("arr2d[0][0]=%.2f, arr2d[%d][%d]=%.2f\n",
           arr2d[0][0], M-1, M-1, arr2d[M-1][M-1]);
    printf("arr3d[0][0][0]=%.2f\n", arr3d[0][0][0]);
    
    free(arr1d);
    return 0;
}
