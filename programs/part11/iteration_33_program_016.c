/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */
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
        arr[i] = arr[i] * 2.0f + (float)i;
    }
    
    printf("Gang partitioned: arr[%d] = %f\n", n-1, arr[n-1]);
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
                arr[i][j] = arr[i][j] * 0.5f;
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
        arr[i] = arr[i] / 3.14159f;
    }
    
    printf("Vector partitioned: arr[%d] = %f\n", n/2, arr[n/2]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (float)(i + j) * 2.0f;
        }
    }
    
    printf("Gang+vector partitioned: arr[0][0] = %f\n", arr[0][0]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] + (float)(i * j);
            }
        }
    }
    
    printf("Worker+vector partitioned: arr[1][1] = %f\n", arr[1][1]);
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float arr3d[M][M][P]) {
    float total = 0.0f;
    
    #pragma acc parallel copy(arr3d[0:M][0:M][0:P]) reduction(+:total)
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < P-1; k++) {
                    /* Stencil computation with data dependencies */
                    arr3d[i][j][k] = (arr3d[i-1][j][k] + 
                                      arr3d[i][j-1][k] + 
                                      arr3d[i][j][k-1]) * 0.333f;
                    total += arr3d[i][j][k];
                }
            }
        }
    }
    
    printf("Fully partitioned: total = %f, center = %f\n", 
           total, arr3d[M/2][M/2][P/2]);
}

/* Helper to initialize arrays */
void init_array_1d(float *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)(i % 100);
    }
}

void init_array_2d(float arr[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (float)((i * M + j) % 100);
        }
    }
}

void init_array_3d(float arr[M][M][P]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = (float)((i * M * P + j * P + k) % 100);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Declare test arrays */
    float arr1[N];
    float arr2[M][M];
    float arr3[M][M][P];
    
    /* Initialize arrays */
    init_array_1d(arr1, N);
    init_array_2d(arr2);
    init_array_3d(arr3);
    
    /* Use argc to control execution, ensuring compiler analyzes all paths */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 8;
    }
    
    /* Conditional execution to force compiler analysis of all OpenACC regions */
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
        default:
            /* Execute all tests in sequence when no specific case requested */
            test_gang_redundant(arr1, N);
            test_gang_partitioned(arr1, N);
            test_worker_partitioned(arr2);
            test_gang_worker_partitioned(arr2);
            test_vector_partitioned(arr1, N);
            test_gang_vector_partitioned(arr2);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr3);
            break;
    }
    
    /* Print some results to prevent dead code elimination */
    printf("Final check - arr1[0]=%f, arr2[0][0]=%f\n", arr1[0], arr2[0][0]);
    
    return 0;
}
