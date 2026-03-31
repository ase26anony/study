/* test_oacc_partition.c - Exercise OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyin(n) reduction(+:sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5f;
            sum += arr[i];
        }
    }
    
    printf("Gang redundant: sum = %f, arr[0] = %f\n", sum, arr[0]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i;
    }
    
    printf("Gang partitioned: arr[%d] = %f\n", n-1, arr[n-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            #pragma acc loop worker
            for (int k = 0; k < 4; k++) {
                arr[i][j] += (i + j + k) * 0.1f;
            }
        }
    }
    
    printf("Worker partitioned: arr[%d][%d] = %f\n", M-1, M-1, arr[M-1][M-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M]) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:M][0:M]) reduction(+:local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = (i * M + j) * 0.5f;
                local_sum += arr[i][j];
            }
        }
    }
    
    printf("Gang+worker partitioned: sum = %f\n", local_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * arr[i] - 1.0f;
    }
    
    printf("Vector partitioned: arr[%d] = %f\n", n/2, arr[n/2]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (arr[i][j] > 0) ? arr[i][j] : -arr[i][j];
        }
    }
    
    printf("Gang+vector partitioned: arr[0][0] = %f\n", arr[0][0]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            #pragma acc loop worker vector
            for (int k = 0; k < 8; k++) {
                arr[i][j] += k * 0.01f;
            }
        }
    }
    
    printf("Worker+vector partitioned: arr[%d][%d] = %f\n", 0, 0, arr[0][0]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr3d[P][M][M]) {
    float total = 0.0f;
    
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M]) reduction(+:total)
    {
        #pragma acc loop gang
        for (int p = 1; p < P-1; p++) {
            #pragma acc loop worker
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    /* Stencil computation requiring all levels */
                    arr3d[p][i][j] = (arr3d[p-1][i][j] + 
                                     arr3d[p][i-1][j] + 
                                     arr3d[p][i][j-1]) * 0.333f;
                    total += arr3d[p][i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned: total = %f, arr3d[%d][%d][%d] = %f\n", 
           total, P/2, M/2, M/2, arr3d[P/2][M/2][M/2]);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *arr1, float *arr2, int n) {
    #pragma acc kernels copy(arr1[0:n], arr2[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr1[i] = i * 0.1f;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            arr2[i] = arr1[i] * 2.0f;
        }
        
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            arr2[i] += 1.0f;
        }
    }
    
    printf("Kernels partitioning: arr2[%d] = %f\n", n-1, arr2[n-1]);
}

int main(int argc, char *argv[]) {
    /* Initialize arrays */
    float arr1[N];
    float arr2[M][M];
    float arr3[P][M][M];
    float arr4[N], arr5[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i;
        arr4[i] = (float)(i % 10);
        arr5[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i + j);
        }
    }
    
    for (int p = 0; p < P; p++) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr3[p][i][j] = (float)(p + i + j);
            }
        }
    }
    
    /* Use argc to control execution, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1, N);
            break;
        case 1:
            test_gang_partitioned(arr1, N);
            break;
        case 2:
            test_worker_partitioned(arr2);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            break;
        case 4:
            test_vector_partitioned(arr1, N);
            break;
        case 5:
            test_gang_vector_partitioned(arr2);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            break;
        case 7:
            test_fully_partitioned(arr3);
            break;
        case 8:
            test_kernels_partitioning(arr4, arr5, N);
            break;
        default:
            /* Execute all tests in sequence when no specific case */
            test_gang_redundant(arr1, N);
            test_gang_partitioned(arr1, N);
            test_worker_partitioned(arr2);
            test_gang_worker_partitioned(arr2);
            test_vector_partitioned(arr1, N);
            test_gang_vector_partitioned(arr2);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr3);
            test_kernels_partitioning(arr4, arr5, N);
            break;
    }
    
    /* Prevent dead code elimination */
    printf("Final check: arr1[0] = %f, arr2[0][0] = %f\n", arr1[0], arr2[0][0]);
    
    return 0;
}
