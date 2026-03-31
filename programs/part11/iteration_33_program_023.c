/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int size) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:size]) copy(sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < size; i++) {
            arr[i] = i * 1.5f;
        }
        sum = arr[0] + arr[size-1];
    }
    
    if (sum > 1000) printf("Gang redundant sum: %f\n", sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int size) {
    float reduction_sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:size]) reduction(+:reduction_sum)
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 2.0f + i;
        reduction_sum += arr[i];
    }
    
    if (reduction_sum > 0) printf("Gang partitioned reduction: %f\n", reduction_sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M]) {
    float temp[M];
    
    #pragma acc parallel copy(arr[0:M][0:M]) create(temp[0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                temp[j] = arr[i][j] * 0.5f;
            }
            
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = temp[j] + i + j;
            }
        }
    }
    
    printf("Worker partitioned first element: %f\n", arr[0][0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M]) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:M][0:M]) reduction(+:local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + i - j) * 1.1f;
                local_sum += arr[i][j];
            }
        }
    }
    
    if (local_sum > 0) printf("Gang+worker partitioned sum: %f\n", local_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int size) {
    #pragma acc parallel loop vector copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        // Element-wise operations suitable for vectorization
        arr[i] = arr[i] * arr[i] - arr[i] / 3.14f + i * 0.01f;
    }
    
    printf("Vector partitioned first/last: %f, %f\n", arr[0], arr[size-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (i * M + j) * 0.25f;
        }
    }
    
    printf("Gang+vector partitioned middle: %f\n", arr[M/2][M/2]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M]) {
    float row_sums[M];
    
    #pragma acc parallel copy(arr[0:M][0:M]) create(row_sums[0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            row_sums[i] = 0.0f;
            
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 0.9f + (i + j) * 0.1f;
                row_sums[i] += arr[i][j];
            }
        }
    }
    
    printf("Worker+vector partitioned row0 sum: %f\n", row_sums[0]);
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float arr3d[P][M][M]) {
    float temp[P][M][M];
    float global_sum = 0.0f;
    
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M]) create(temp[0:P][0:M][0:M]) \
                         reduction(+:global_sum)
    {
        // Initialize temp array
        #pragma acc loop gang
        for (int k = 0; k < P; k++) {
            #pragma acc loop worker
            for (int i = 0; i < M; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    temp[k][i][j] = (k * M * M + i * M + j) * 0.01f;
                }
            }
        }
        
        // Stencil-like computation with data dependencies
        #pragma acc loop gang
        for (int k = 1; k < P-1; k++) {
            #pragma acc loop worker
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    // 3D stencil computation
                    arr3d[k][i][j] = (temp[k-1][i][j] + temp[k][i-1][j] + 
                                     temp[k][i][j-1] + temp[k][i+1][j] + 
                                     temp[k][i][j+1] + temp[k+1][i][j]) / 6.0f;
                    global_sum += arr3d[k][i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned global sum: %f\n", global_sum);
}

/* Mixed partitioning with runtime condition */
void test_mixed_partitioning(float *arr1, float arr2[M][M], int use_gpu) {
    if (use_gpu) {
        // Force compiler to analyze both paths
        #pragma acc parallel loop gang copy(arr1[0:N])
        for (int i = 0; i < N; i++) {
            arr1[i] = arr1[i] * 3.14f;
        }
        
        #pragma acc parallel loop gang worker copy(arr2[0:M][0:M])
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr2[i][j] + i - j;
            }
        }
    } else {
        // Host fallback path
        for (int i = 0; i < N; i++) {
            arr1[i] = arr1[i] * 2.0f;
        }
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr2[i][j] * 2.0f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Allocate and initialize test arrays
    float array1[N];
    float array2[M][M];
    float array3d[P][M][M];
    
    // Initialize with simple values
    for (int i = 0; i < N; i++) {
        array1[i] = (float)i;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            array2[i][j] = (float)(i * M + j);
        }
    }
    
    for (int k = 0; k < P; k++) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                array3d[k][i][j] = (float)(k * M * M + i * M + j) * 0.1f;
            }
        }
    }
    
    // Use command-line argument to control which tests run
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9;  // 0-8 for different cases
    }
    
    // Execute different partitioning patterns based on input
    switch (test_case) {
        case 0:
            test_gang_redundant(array1, N);
            break;
        case 1:
            test_gang_partitioned(array1, N);
            break;
        case 2:
            test_worker_partitioned(array2);
            break;
        case 3:
            test_gang_worker_partitioned(array2);
            break;
        case 4:
            test_vector_partitioned(array1, N);
            break;
        case 5:
            test_gang_vector_partitioned(array2);
            break;
        case 6:
            test_worker_vector_partitioned(array2);
            break;
        case 7:
            test_fully_partitioned(array3d);
            break;
        default:
            // Test mixed partitioning with runtime condition
            test_mixed_partitioning(array1, array2, argc > 2);
            break;
    }
    
    // Also call some functions unconditionally to ensure compilation
    // of all OpenACC constructs regardless of runtime path
    if (argc > 3) {
        test_gang_redundant(array1, N);
        test_vector_partitioned(array1, N);
    }
    
    // Print results to prevent dead code elimination
    printf("Final array1[0]: %f, array1[%d]: %f\n", 
           array1[0], N-1, array1[N-1]);
    printf("Final array2[0][0]: %f, array2[%d][%d]: %f\n",
           array2[0][0], M-1, M-1, array2[M-1][M-1]);
    printf("Final array3d[0][0][0]: %f\n", array3d[0][0][0]);
    
    return 0;
}
