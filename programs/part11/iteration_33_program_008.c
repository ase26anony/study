/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *data, int size) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:size]) copy(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < size; i++) {
            data[i] = i * 1.5f;
        }
        local_sum = 42.0f;  /* Simple assignment in gang-redundant region */
    }
    
    printf("Gang redundant: first element = %f, local_sum = %f\n", data[0], local_sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data, int size) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    printf("Gang partitioned: sum = %f, last element = %f\n", sum, data[size-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float data[M][M]) {
    #pragma acc parallel copy(data[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                data[i][j] = (i + j) * 0.5f;
            }
        }
    }
    
    printf("Worker partitioned: data[%d][%d] = %f\n", M/2, M/2, data[M/2][M/2]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[M][M]) {
    float row_sums[M] = {0};
    
    #pragma acc parallel copy(data[0:M][0:M]) copy(row_sums[0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            float row_sum = 0.0f;
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                data[i][j] = (i * M + j) * 0.25f;
                row_sum += data[i][j];
            }
            row_sums[i] = row_sum;
        }
    }
    
    printf("Gang+worker partitioned: row_sums[0] = %f, row_sums[%d] = %f\n", 
           row_sums[0], M-1, row_sums[M-1]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *a, float *b, float *c, int size) {
    #pragma acc parallel loop vector copy(a[0:size], b[0:size], c[0:size])
    for (int i = 0; i < size; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
    
    printf("Vector partitioned: c[0] = %f, c[%d] = %f\n", c[0], size-1, c[size-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float data[M][M]) {
    #pragma acc parallel loop gang vector collapse(2) copy(data[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (i * i + j * j) * 0.1f;
        }
    }
    
    printf("Gang+vector partitioned: data[0][0] = %f, data[%d][%d] = %f\n", 
           data[0][0], M-1, M-1, data[M-1][M-1]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data[M][P]) {
    #pragma acc parallel copy(data[0:M][0:P])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            #pragma acc loop worker vector collapse(2)
            for (int i = block * (M/4); i < (block+1) * (M/4); i++) {
                for (int j = 0; j < P; j++) {
                    data[i][j] = (i - j) * (i + j) * 0.01f;
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: data[%d][%d] = %f\n", 
           M/2, P/2, data[M/2][P/2]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float data[M][M]) {
    /* Initialize with some values */
    #pragma acc parallel loop gang collapse(2) copy(data[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i * 10.0f + j;
        }
    }
    
    /* Complex stencil computation with full partitioning */
    float temp[M][M];
    
    #pragma acc parallel copy(data[0:M][0:M]) create(temp[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                float sum = 0.0f;
                #pragma acc loop vector reduction(+:sum)
                for (int k = -1; k <= 1; k++) {
                    for (int l = -1; l <= 1; l++) {
                        sum += data[i+k][j+l];
                    }
                }
                temp[i][j] = sum / 9.0f;
            }
        }
        
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 1; i < M-1; i++) {
            for (int j = 1; j < M-1; j++) {
                data[i][j] = temp[i][j];
            }
        }
    }
    
    printf("Fully partitioned: data[%d][%d] = %f\n", 
           M/2, M/2, data[M/2][M/2]);
}

/* Helper function with conditional execution to ensure compiler analysis */
void conditional_test(float *arr1, float arr2[M][M], float arr3[M][P], 
                      float *a, float *b, float *c, int test_id) {
    if (test_id == 0 || test_id == 1) {
        test_gang_redundant(arr1, N);
    }
    if (test_id == 0 || test_id == 2) {
        test_gang_partitioned(arr1, N);
    }
    if (test_id == 0 || test_id == 3) {
        test_worker_partitioned(arr2);
    }
    if (test_id == 0 || test_id == 4) {
        test_gang_worker_partitioned(arr2);
    }
    if (test_id == 0 || test_id == 5) {
        test_vector_partitioned(a, b, c, N);
    }
    if (test_id == 0 || test_id == 6) {
        test_gang_vector_partitioned(arr2);
    }
    if (test_id == 0 || test_id == 7) {
        test_worker_vector_partitioned(arr3);
    }
    if (test_id == 0 || test_id == 8) {
        test_fully_partitioned(arr2);
    }
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test arrays */
    float arr1[N];
    float arr2[M][M];
    float arr3[M][P];
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 0.5f;
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
        c[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = 0.0f;
        }
        for (int j = 0; j < P; j++) {
            arr3[i][j] = 0.0f;
        }
    }
    
    /* Determine which tests to run based on command line argument */
    int test_id = 0;  /* 0 = run all tests */
    if (argc > 1) {
        test_id = atoi(argv[1]);
        if (test_id < 0 || test_id > 8) test_id = 0;
    }
    
    /* Use conditional execution to ensure compiler analyzes all paths */
    if (argc > 2) {
        /* Alternative path with different partitioning */
        #pragma acc parallel loop gang copy(arr1[0:N])
        for (int i = 0; i < N; i++) {
            arr1[i] += 100.0f;
        }
    }
    
    /* Run the selected tests */
    conditional_test(arr1, arr2, arr3, a, b, c, test_id);
    
    /* Print some results to prevent dead code elimination */
    printf("Final check - arr1[0] = %f, arr1[%d] = %f\n", arr1[0], N-1, arr1[N-1]);
    printf("Final check - arr2[0][0] = %f, arr2[%d][%d] = %f\n", 
           arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
    printf("Final check - c[100] = %f\n", c[100]);
    
    return 0;
}
