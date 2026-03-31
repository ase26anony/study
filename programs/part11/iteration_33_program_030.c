/* test_oacc_partition.c - OpenACC Partitioning Test for GCC Coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int size) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:size]) copy(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < 1; i++) {
            local_sum = 42.0f;
        }
        arr[0] = local_sum;
    }
    
    if (size > 0) {
        arr[size-1] = local_sum;
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int size) {
    float reduction_sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:reduction_sum) copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = (float)i * 1.5f;
        reduction_sum += arr[i];
    }
    
    arr[0] = reduction_sum / size;
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (float)(i * M + j) * 0.1f;
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
                arr[i][j] = (float)(i + j) * 2.0f;
                if (i == j) {
                    tile_sum += arr[i][j];
                }
            }
        }
    }
    
    arr[0][0] = tile_sum;
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int size) {
    #pragma acc parallel loop vector copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 2.0f + 1.0f;
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (float)(i * j) / (float)M;
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
                arr[i][j] = (float)(i * i + j * j) * 0.01f;
            }
        }
    }
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float arr3d[P][M][M]) {
    float global_max = 0.0f;
    
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M]) reduction(max:global_max)
    {
        #pragma acc loop gang
        for (int k = 0; k < P; k++) {
            #pragma acc loop worker
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    /* Stencil computation requiring all partitioning levels */
                    float val = (arr3d[k][i-1][j] + arr3d[k][i][j-1] + 
                                arr3d[k][i+1][j] + arr3d[k][i][j+1]) * 0.25f;
                    arr3d[k][i][j] = val;
                    
                    if (val > global_max) {
                        global_max = val;
                    }
                }
            }
        }
    }
    
    arr3d[0][0][0] = global_max;
}

/* Helper function with conditional execution */
void conditional_test(int test_id, float *arr1, float arr2[M][M], float arr3d[P][M][M]) {
    if (test_id % 2 == 0) {
        /* Force compiler to analyze both paths */
        test_gang_partitioned(arr1, N);
        test_worker_partitioned(arr2);
    } else {
        test_vector_partitioned(arr1, N);
        test_gang_worker_partitioned(arr2);
    }
}

int main(int argc, char *argv[]) {
    float array1[N];
    float array2[M][M];
    float array3d[P][M][M];
    
    /* Initialize arrays */
    memset(array1, 0, sizeof(array1));
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            array2[i][j] = (float)(i + j);
        }
    }
    
    for (int k = 0; k < P; k++) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                array3d[k][i][j] = (float)(k * M * M + i * M + j) * 0.1f;
            }
        }
    }
    
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 8;
    }
    
    /* Execute different partitioning patterns based on input */
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
    }
    
    /* Conditional execution to ensure all code paths are analyzed */
    if (argc > 2) {
        conditional_test(test_case, array1, array2, array3d);
    }
    
    /* Additional combined test to hit multiple cases in one compilation */
    if (argc > 3) {
        test_gang_partitioned(array1, N);
        test_worker_vector_partitioned(array2);
        test_fully_partitioned(array3d);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: array1[0]=%.2f, array1[%d]=%.2f\n", 
           array1[0], N-1, array1[N-1]);
    printf("array2[0][0]=%.2f, array2[%d][%d]=%.2f\n",
           array2[0][0], M-1, M-1, array2[M-1][M-1]);
    printf("array3d[0][0][0]=%.2f\n", array3d[0][0][0]);
    
    return 0;
}
