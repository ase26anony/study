/* test_oacc_partition.c - Exercise OpenACC partitioning classification */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyin(n) reduction(+:sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 0.5f;
            sum += arr[i];
        }
    }
    
    if (sum < 0) printf("Unlikely"); /* Prevent dead code elimination */
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + (float)i;
    }
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *arr, int n) {
    #pragma acc parallel loop worker copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        /* Complex enough for worker partitioning */
        float temp = arr[i];
        for (int j = 0; j < 4; j++) {
            temp = temp * 0.9f + (float)j;
        }
        arr[i] = temp;
    }
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m]) copyin(m)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = (float)(i * m + j) * 0.25f;
            }
        }
    }
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * arr[i] - arr[i] / 2.0f;
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang vector copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = (arr[i] > 0.0f) ? arr[i] : -arr[i];
    }
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m]) copyin(m)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + (float)(i + j);
            }
        }
    }
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m]) copyin(m)
    {
        /* Triple nested with explicit clauses */
        #pragma acc loop gang
        for (int i = 1; i < m-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < m-1; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    /* Stencil-like computation with data dependencies */
                    float neighbor_sum = 0.0f;
                    for (int di = -1; di <= 1; di++) {
                        for (int dj = -1; dj <= 1; dj++) {
                            if (di == 0 && dj == 0) continue;
                            neighbor_sum += arr[i+di][j+dj];
                        }
                    }
                    arr[i][j] = neighbor_sum * 0.125f + (float)k * 0.1f;
                }
            }
        }
    }
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *arr1, float *arr2, int n) {
    #pragma acc kernels copy(arr1[0:n], arr2[0:n]) copyin(n)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr1[i] = (float)i * 0.5f;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            arr2[i] = arr1[i] * 2.0f;
        }
        
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            arr2[i] = arr2[i] + 1.0f;
        }
    }
}

int main(int argc, char *argv[]) {
    float array1[N];
    float array2[N];
    float matrix1[M][M];
    float matrix2[M][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = (float)(i % 100) * 0.01f;
        array2[i] = (float)(i % 50) * 0.02f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix1[i][j] = (float)(i * M + j) * 0.01f;
            matrix2[i][j] = (float)(i + j) * 0.02f;
        }
    }
    
    /* Use argc to ensure all code paths are analyzed */
    int test_case = (argc > 1) ? atoi(argv[1]) % 9 : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(array1, N);
            break;
        case 1:
            test_gang_partitioned(array1, N);
            break;
        case 2:
            test_worker_partitioned(array1, N);
            break;
        case 3:
            test_gang_worker_partitioned(matrix1, M);
            break;
        case 4:
            test_vector_partitioned(array1, N);
            break;
        case 5:
            test_gang_vector_partitioned(array1, N);
            break;
        case 6:
            test_worker_vector_partitioned(matrix1, M);
            break;
        case 7:
            test_fully_partitioned(matrix2, M);
            break;
        case 8:
            test_kernels_partitioning(array1, array2, N);
            break;
        default:
            /* Execute all tests in sequence when no specific case */
            test_gang_redundant(array1, N);
            test_gang_partitioned(array2, N);
            test_worker_partitioned(array1, N);
            test_gang_worker_partitioned(matrix1, M);
            test_vector_partitioned(array2, N);
            test_gang_vector_partitioned(array1, N);
            test_worker_vector_partitioned(matrix2, M);
            test_fully_partitioned(matrix1, M);
            test_kernels_partitioning(array1, array2, N);
            break;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %.2f %.2f %.2f %.2f\n", 
           array1[0], array1[N-1], 
           matrix1[0][0], matrix2[M-1][M-1]);
    
    return 0;
}
