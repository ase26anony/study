/* test_oacc_partition.c - Exercise OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copy(sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5f;
        }
        sum = arr[0] + arr[n-1];
    }
    
    if (sum > 1000) printf("Gang redundant sum: %f\n", sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float reduction_sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:reduction_sum) copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i;
        reduction_sum += arr[i];
    }
    
    if (reduction_sum > 0) printf("Gang partitioned reduction: %f\n", reduction_sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = (i + j) * 0.5f;
            }
        }
    }
    
    if (arr[0][0] >= 0) printf("Worker partitioned first: %f\n", arr[0][0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M], int m) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:m][0:m]) copy(local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 1.1f;
                if (i == j) local_sum += arr[i][j];
            }
        }
    }
    
    if (local_sum > 0) printf("Gang+worker diagonal sum: %f\n", local_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        // Element-wise operation suitable for vectorization
        arr[i] = arr[i] * arr[i] - arr[i] / 3.0f + 1.0f;
    }
    
    if (arr[0] > 0) printf("Vector partitioned first: %f\n", arr[0]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M], int m) {
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:m][0:m])
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = (i * m + j) * 0.25f;
        }
    }
    
    if (arr[m-1][m-1] > 0) printf("Gang+vector last: %f\n", arr[m-1][m-1]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][P], int m, int p) {
    #pragma acc parallel copy(arr[0:m][0:p])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < p; j++) {
                arr[i][j] = (i * 0.3f + j * 0.7f) * 2.0f;
            }
        }
    }
    
    if (arr[0][0] >= 0) printf("Worker+vector first: %f\n", arr[0][0]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr[M][M][P], int m, int p) {
    float total = 0.0f;
    
    #pragma acc parallel copy(arr[0:m][0:m][0:p]) copy(total)
    {
        #pragma acc loop gang
        for (int i = 1; i < m-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < m-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < p-1; k++) {
                    // Stencil computation with data dependencies
                    arr[i][j][k] = (arr[i-1][j][k] + arr[i][j-1][k] + 
                                   arr[i][j][k-1]) * 0.333f;
                    total += arr[i][j][k];
                }
            }
        }
    }
    
    if (total > 0) printf("Fully partitioned total: %f\n", total);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *arr1, float *arr2, int n) {
    #pragma acc kernels copy(arr1[0:n], arr2[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr1[i] = i * 2.0f;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            arr2[i] = arr1[i] * 0.5f;
        }
    }
    
    if (arr2[0] >= 0) printf("Kernels test first: %f\n", arr2[0]);
}

int main(int argc, char *argv[]) {
    float arr1[N];
    float arr2[M][M];
    float arr3[M][P];
    float arr4[M][M][P];
    float arr5[N], arr6[N];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i;
        arr5[i] = (float)i * 0.5f;
        arr6[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i * M + j);
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr3[i][j] = (float)(i * P + j) * 0.1f;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr4[i][j][k] = (float)(i * M * P + j * P + k) * 0.01f;
            }
        }
    }
    
    // Use argc to conditionally execute different tests
    // This ensures all OpenACC constructs are processed by the compiler
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1, N);
            break;
        case 1:
            test_gang_partitioned(arr1, N);
            break;
        case 2:
            test_worker_partitioned(arr2, M);
            break;
        case 3:
            test_gang_worker_partitioned(arr2, M);
            break;
        case 4:
            test_vector_partitioned(arr1, N);
            break;
        case 5:
            test_gang_vector_partitioned(arr2, M);
            break;
        case 6:
            test_worker_vector_partitioned(arr3, M, P);
            break;
        case 7:
            test_fully_partitioned(arr4, M, P);
            break;
        case 8:
            test_kernels_partitioning(arr5, arr6, N);
            break;
        default:
            // Execute all tests to ensure all code paths are compiled
            if (test_case > 100) {
                test_gang_redundant(arr1, N);
                test_gang_partitioned(arr1, N);
                test_worker_partitioned(arr2, M);
                test_gang_worker_partitioned(arr2, M);
                test_vector_partitioned(arr1, N);
                test_gang_vector_partitioned(arr2, M);
                test_worker_vector_partitioned(arr3, M, P);
                test_fully_partitioned(arr4, M, P);
                test_kernels_partitioning(arr5, arr6, N);
            }
            break;
    }
    
    // Print results to prevent dead code elimination
    printf("Results: %f, %f, %f\n", 
           arr1[0], arr1[N-1], 
           (M > 0 && P > 0) ? arr3[M-1][P-1] : 0.0f);
    
    return 0;
}
