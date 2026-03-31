/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition string mapping logic
 * in GCC's OpenACC neuter-broadcast pass (lines 335-343 of omp-oacc-neuter-broadcast.cc).
 * It uses various OpenACC compute constructs with explicit data partitioning
 * across gang, worker, and vector dimensions.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Test function 1: Basic partition combinations on 3D array */
void test_basic_partitions(void) {
    int arr1[N][M][P];
    int i, j, k;
    
    /* Initialize array */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < P; k++)
                arr1[i][j][k] = i + j + k;
    
    /* Case 0: gang redundant */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 1;
    }
    
    /* Case 1: gang partitioned */
    #pragma acc kernels create(arr1[0:N][0:M][0:P]) gang(static:2)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++)
            #pragma acc loop worker
            for (j = 0; j < M; j++)
                #pragma acc loop vector
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 2;
    }
    
    /* Case 2: worker partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 3;
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc kernels copy(arr1[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 4;
    }
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 5;
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 6;
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc kernels copy(arr1[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 7;
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                for (k = 0; k < P; k++)
                    arr1[i][j][k] += 8;
    }
}

/* Test function 2: Multi-dimensional array with collapse clause */
void test_collapse_partitions(void) {
    double arr2[N][M][P];
    int i, j, k;
    
    /* Initialize */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < P; k++)
                arr2[i][j][k] = (double)(i * j * k);
    
    /* Collapse with gang partitioning */
    #pragma acc parallel loop collapse(3) gang copy(arr2[0:N][0:M][0:P])
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < P; k++)
                arr2[i][j][k] *= 1.1;
    
    /* Collapse with vector partitioning */
    #pragma acc kernels loop collapse(2) vector copy(arr2[0:N][0:M][0:P])
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < P; k++)
                arr2[i][j][k] *= 1.2;
}

/* Test function 3: Nested and sequential compute regions */
void test_nested_regions(int condition) {
    int arr3[N][M];
    int i, j;
    
    /* Initialize */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            arr3[i][j] = i * M + j;
    
    /* Conditional offloading with gang partition */
    #pragma acc parallel if(condition) gang copy(arr3[0:N][0:M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            /* Nested worker partition region */
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                arr3[i][j] += i + j;
            }
        }
    }
    
    /* Sequential region with different partition */
    #pragma acc kernels if(!condition) worker copy(arr3[0:N][0:M])
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                arr3[i][j] *= 2;
    }
}

/* Routine directive with partition specification */
#pragma acc routine vec gang
void acc_routine_func(float *arr, int n) {
    int i;
    #pragma acc loop gang vector
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + 1.0f;
    }
}

/* Test function 4: Routine directive with partition propagation */
void test_routine_partitions(void) {
    float arr4[N*M];
    int i;
    
    /* Initialize */
    for (i = 0; i < N*M; i++)
        arr4[i] = (float)i;
    
    /* Compute region calling partitioned routine */
    #pragma acc parallel copy(arr4[0:N*M]) gang vector
    {
        #pragma acc loop gang vector
        for (i = 0; i < N*M; i += 64) {
            int chunk = (i + 64 < N*M) ? 64 : N*M - i;
            acc_routine_func(&arr4[i], chunk);
        }
    }
}

/* Test function 5: Device data environment with partitions */
void test_device_data_partitions(void) {
    int arr5[N][M];
    int i, j;
    
    /* Initialize */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            arr5[i][j] = 0;
    
    /* Enter data with gang partition */
    #pragma acc enter data copyin(arr5[0:N][0:M]) gang
    
    /* Compute region 1: worker partition with present data */
    #pragma acc parallel present(arr5) worker
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                arr5[i][j] += i * j;
    }
    
    /* Compute region 2: vector partition with present data */
    #pragma acc kernels present(arr5) vector
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                arr5[i][j] *= 2;
    }
    
    /* Exit data */
    #pragma acc exit data copyout(arr5[0:N][0:M])
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    int test_condition = (argc > 1) ? atoi(argv[1]) : 1;
    
    printf("Testing OpenACC partition combinations...\n");
    
    /* Execute different test paths based on condition */
    if (test_condition & 1) {
        test_basic_partitions();
        printf("Basic partitions test completed.\n");
    }
    
    if (test_condition & 2) {
        test_collapse_partitions();
        printf("Collapse partitions test completed.\n");
    }
    
    if (test_condition & 4) {
        test_nested_regions(test_condition);
        printf("Nested regions test completed.\n");
    }
    
    if (test_condition & 8) {
        test_routine_partitions();
        printf("Routine partitions test completed.\n");
    }
    
    if (test_condition & 16) {
        test_device_data_partitions();
        printf("Device data partitions test completed.\n");
    }
    
    printf("All tests completed successfully.\n");
    return 0;
}
