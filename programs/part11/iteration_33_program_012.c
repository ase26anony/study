/* test_oacc_partition.c - Test OpenACC partitioning for coverage */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *data, int size) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:size]) copyout(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < 1; i++) {
            local_sum = 42.0f;
        }
        
        /* Simple assignment without loop partitioning */
        data[0] = local_sum;
    }
    
    if (local_sum != 42.0f) {
        printf("Gang redundant test failed\n");
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data, int size) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(data[0:size]) reduction(+:sum)
    for (int i = 0; i < size; i++) {
        data[i] = i * 1.5f;
        sum += data[i];
    }
    
    printf("Gang partitioned sum: %f\n", sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *data, int size) {
    #pragma acc parallel copy(data[0:size])
    {
        #pragma acc loop gang
        for (int i = 0; i < size/M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < size) {
                    data[idx] = (i + j) * 0.5f;
                }
            }
        }
    }
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[M][M]) {
    #pragma acc parallel copy(data[0:M][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = (i * M + j) * 0.25f;
            }
        }
    }
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *data, int size) {
    #pragma acc parallel loop vector copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2.0f + 1.0f;
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float data[M][M]) {
    float factor = 2.5f;
    
    #pragma acc parallel loop gang vector copy(data[0:M][0:M]) private(factor)
    for (int i = 0; i < M; i++) {
        factor = 1.0f + i * 0.1f;
        for (int j = 0; j < M; j++) {
            data[i][j] = (i + j) * factor;
        }
    }
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data[M][M]) {
    #pragma acc parallel copy(data[0:M][0:M])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            #pragma acc loop worker vector
            for (int i = block * (M/4); i < (block + 1) * (M/4); i++) {
                for (int j = 0; j < M; j++) {
                    data[i][j] = (i * j) / (float)M;
                }
            }
        }
    }
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float data[M][M]) {
    float temp[M][M];
    
    #pragma acc data create(temp[0:M][0:M]) copy(data[0:M][0:M])
    {
        #pragma acc parallel
        {
            /* Initialize temp array */
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 1; k++) {  /* Vector loop for SIMD */
                        temp[i][j] = i * M + j;
                    }
                }
            }
            
            /* Stencil computation with full partitioning */
            #pragma acc loop gang
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < M-1; j++) {
                    #pragma acc loop vector
                    for (int iter = 0; iter < 4; iter++) {
                        data[i][j] = (temp[i-1][j] + temp[i][j-1] + 
                                     temp[i+1][j] + temp[i][j+1]) * 0.25f;
                    }
                }
            }
        }
    }
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *data, int size) {
    #pragma acc kernels copy(data[0:size])
    {
        #pragma acc loop gang
        for (int i = 0; i < size/P; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < P; j++) {
                int idx = i * P + j;
                if (idx < size) {
                    data[idx] = data[idx] * 3.14f;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    float arr1[N];
    float arr2[M][M];
    float arr3[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 0.1f;
        arr3[i] = i * 0.2f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = 0.0f;
        }
    }
    
    /* Use argc to control which tests run, ensuring all code is compiled */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9;  /* 0-8 for different cases */
    }
    
    /* Force compilation of all paths with conditional execution */
    if (test_case == 0 || argc == 1) {
        test_gang_redundant(arr1, N);
    }
    
    if (test_case == 1 || argc == 1) {
        test_gang_partitioned(arr1, N);
    }
    
    if (test_case == 2 || argc == 1) {
        test_worker_partitioned(arr3, N);
    }
    
    if (test_case == 3 || argc == 1) {
        test_gang_worker_partitioned(arr2);
    }
    
    if (test_case == 4 || argc == 1) {
        test_vector_partitioned(arr1, N);
    }
    
    if (test_case == 5 || argc == 1) {
        test_gang_vector_partitioned(arr2);
    }
    
    if (test_case == 6 || argc == 1) {
        test_worker_vector_partitioned(arr2);
    }
    
    if (test_case == 7 || argc == 1) {
        test_fully_partitioned(arr2);
    }
    
    if (test_case == 8 || argc == 1) {
        test_kernels_partitioning(arr3, N);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: arr1[0]=%f, arr1[%d]=%f\n", arr1[0], N-1, arr1[N-1]);
    printf("arr2[0][0]=%f, arr2[%d][%d]=%f\n", arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
    printf("arr3[100]=%f\n", arr3[100]);
    
    return 0;
}
