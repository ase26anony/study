/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

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
    
    printf("Gang redundant: sum = %f\n", sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float reduction_sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:reduction_sum) copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i;
        reduction_sum += arr[i];
    }
    
    printf("Gang partitioned: reduction = %f\n", reduction_sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (i + j) * 0.5f;
            }
        }
    }
    
    printf("Worker partitioned: arr[0][0] = %f, arr[M-1][M-1] = %f\n", 
           arr[0][0], arr[M-1][M-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M]) {
    float tile_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:M][0:M]) reduction(+:tile_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + i - j) * 1.1f;
                if (i % 4 == 0 && j % 4 == 0) {
                    tile_sum += arr[i][j];
                }
            }
        }
    }
    
    printf("Gang+worker partitioned: tile_sum = %f\n", tile_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * arr[i] - arr[i] / 2.0f;
    }
    
    printf("Vector partitioned: arr[0] = %f, arr[n-1] = %f\n", arr[0], arr[n-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (i * M + j) * 0.25f;
        }
    }
    
    printf("Gang+vector partitioned: arr[1][1] = %f\n", arr[1][1]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M]) {
    float row_sums[M] = {0};
    
    #pragma acc parallel copy(arr[0:M][0:M]) copyout(row_sums[0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            float private_sum = 0.0f;
            #pragma acc loop worker vector reduction(+:private_sum)
            for (int j = 0; j < M; j++) {
                arr[i][j] = (i * 0.3f) + (j * 0.7f);
                private_sum += arr[i][j];
            }
            row_sums[i] = private_sum;
        }
    }
    
    printf("Worker+vector partitioned: row_sums[0] = %f\n", row_sums[0]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr3d[P][M][M]) {
    float global_sum = 0.0f;
    
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M]) reduction(+:global_sum)
    {
        #pragma acc loop gang
        for (int k = 0; k < P; k++) {
            #pragma acc loop worker
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    /* Stencil computation requiring all levels */
                    arr3d[k][i][j] = (arr3d[k][i-1][j] + arr3d[k][i][j-1] + 
                                     arr3d[k][i+1][j] + arr3d[k][i][j+1]) * 0.25f;
                    global_sum += arr3d[k][i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned: global_sum = %f\n", global_sum);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *arr1, float arr2[M][M]) {
    /* Use argc to ensure both paths are analyzed */
    int use_kernels = 1;
    
    if (use_kernels) {
        #pragma acc kernels copy(arr1[0:N]) copy(arr2[0:M][0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                arr1[i] = i * 0.01f;
            }
            
            #pragma acc loop gang worker
            for (int i = 0; i < M; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr2[i][j] = arr1[i*M+j % N] * 2.0f;
                }
            }
        }
    }
    
    printf("Kernels partitioning: arr1[10] = %f\n", arr1[10]);
}

int main(int argc, char *argv[]) {
    /* Initialize data arrays */
    float arr1[N];
    float arr2[M][M];
    float arr3d[P][M][M];
    
    for (int i = 0; i < N; i++) arr1[i] = i * 0.5f;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++)
            arr2[i][j] = (i + j) * 1.0f;
    for (int k = 0; k < P; k++)
        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++)
                arr3d[k][i][j] = (k + i + j) * 0.1f;
    
    /* Use command-line argument to control execution path */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9; /* 0-8 for different cases */
    }
    
    /* Force compiler to analyze all OpenACC constructs by having them
       in conditional blocks based on runtime value */
    if (test_case == 0 || argc > 2) {
        test_gang_redundant(arr1, N);
    }
    if (test_case == 1 || argc > 2) {
        test_gang_partitioned(arr1, N);
    }
    if (test_case == 2 || argc > 2) {
        test_worker_partitioned(arr2);
    }
    if (test_case == 3 || argc > 2) {
        test_gang_worker_partitioned(arr2);
    }
    if (test_case == 4 || argc > 2) {
        test_vector_partitioned(arr1, N);
    }
    if (test_case == 5 || argc > 2) {
        test_gang_vector_partitioned(arr2);
    }
    if (test_case == 6 || argc > 2) {
        test_worker_vector_partitioned(arr2);
    }
    if (test_case == 7 || argc > 2) {
        test_fully_partitioned(arr3d);
    }
    if (test_case == 8 || argc > 2) {
        test_kernels_partitioning(arr1, arr2);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Final check - arr1[0]=%f, arr2[0][0]=%f, arr3d[0][0][0]=%f\n",
           arr1[0], arr2[0][0], arr3d[0][0][0]);
    
    return 0;
}
