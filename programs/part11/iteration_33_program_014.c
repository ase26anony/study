/* test_oacc_partition.c - Test OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyin(n) reduction(+:local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5f;
            local_sum += arr[i];
        }
    }
    
    if (local_sum > 0) {
        printf("Gang redundant: first element = %f\n", arr[0]);
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:n]) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i;
        sum += arr[i];
    }
    
    if (sum > 0) {
        printf("Gang partitioned: sum = %f\n", sum);
    }
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M], int m) {
    float temp[M][M];
    
    #pragma acc parallel copy(arr[0:m][0:m]) create(temp[0:m][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                temp[i][j] = arr[i][j] * 3.0f;
            }
        }
        
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = temp[i][j] + i + j;
            }
        }
    }
    
    printf("Worker partitioned: arr[0][0] = %f\n", arr[0][0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M], int m) {
    #pragma acc parallel loop gang worker copy(arr[0:m][0:m])
    for (int i = 0; i < m * m; i++) {
        int row = i / m;
        int col = i % m;
        arr[row][col] = (arr[row][col] + row * col) * 0.5f;
    }
    
    printf("Gang+worker partitioned: arr[%d][%d] = %f\n", 
           m-1, m-1, arr[m-1][m-1]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        // Element-wise operations suitable for vectorization
        arr[i] = arr[i] * arr[i] - 2.0f * arr[i] + 1.0f;
    }
    
    printf("Vector partitioned: arr[%d] = %f\n", n-1, arr[n-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][P], int m, int p) {
    float local_max = 0.0f;
    
    #pragma acc parallel loop gang vector copy(arr[0:m][0:p]) reduction(max:local_max)
    for (int i = 0; i < m * p; i++) {
        int row = i / p;
        int col = i % p;
        arr[row][col] = (row * 1.0f) / (col + 1.0f);
        if (arr[row][col] > local_max) {
            local_max = arr[row][col];
        }
    }
    
    printf("Gang+vector partitioned: max = %f\n", local_max);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            #pragma acc loop worker vector
            for (int idx = 0; idx < (m * m) / 4; idx++) {
                int i = (block * (m/2) + idx / m) % m;
                int j = idx % m;
                arr[i][j] = arr[i][j] + (i * j * 0.01f);
            }
        }
    }
    
    printf("Worker+vector partitioned: arr[%d][%d] = %f\n", 
           m/2, m/2, arr[m/2][m/2]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr[M][M][P], int m, int p) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:m][0:m][0:p]) reduction(+:sum)
    {
        // Triple nested loop with explicit partitioning
        #pragma acc loop gang
        for (int i = 1; i < m - 1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < m - 1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < p - 1; k++) {
                    // Stencil computation with data dependencies
                    arr[i][j][k] = (arr[i-1][j][k] + arr[i][j-1][k] + 
                                   arr[i][j][k-1]) * 0.333f;
                    sum += arr[i][j][k];
                }
            }
        }
    }
    
    printf("Fully partitioned: sum = %f\n", sum);
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
        
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            arr1[i] = arr1[i] + arr2[i];
        }
    }
    
    printf("Kernels partitioning: arr1[0] = %f, arr2[%d] = %f\n", 
           arr1[0], n-1, arr2[n-1]);
}

int main(int argc, char *argv[]) {
    // Initialize arrays
    float arr1[N];
    float arr2[M][M];
    float arr3[M][P];
    float arr4[M][M][P];
    float arr5[N];
    float arr6[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 1.0f;
        arr5[i] = i * 0.5f;
        arr6[i] = i * 0.25f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = i * M + j;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr3[i][j] = i * P + j;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr4[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    // Use argc to control which tests run, ensuring all code paths are compiled
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9;  // 0-8 for different cases
    }
    
    // All tests are compiled, but execution depends on runtime condition
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
            test_gang_vector_partitioned(arr3, M, P);
            break;
        case 6:
            test_worker_vector_partitioned(arr2, M);
            break;
        case 7:
            test_fully_partitioned(arr4, M, P);
            break;
        case 8:
            test_kernels_partitioning(arr5, arr6, N);
            break;
        default:
            // Run all tests if no specific case
            test_gang_redundant(arr1, N);
            test_gang_partitioned(arr1, N);
            test_worker_partitioned(arr2, M);
            test_gang_worker_partitioned(arr2, M);
            test_vector_partitioned(arr1, N);
            test_gang_vector_partitioned(arr3, M, P);
            test_worker_vector_partitioned(arr2, M);
            test_fully_partitioned(arr4, M, P);
            test_kernels_partitioning(arr5, arr6, N);
            break;
    }
    
    // Print some results to prevent dead code elimination
    printf("Final check - arr1[0] = %f, arr2[0][0] = %f\n", arr1[0], arr2[0][0]);
    
    return 0;
}
