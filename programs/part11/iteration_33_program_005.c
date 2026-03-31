/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *data, int size) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:size]) copyin(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < size; i++) {
            data[i] = i * 0.5f;
        }
        local_sum = data[0];
    }
    
    printf("Gang redundant: data[0] = %f, local_sum = %f\n", data[0], local_sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data, int size) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    printf("Gang partitioned: sum = %f, data[%d] = %f\n", sum, size-1, data[size-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *data, int size) {
    #pragma acc parallel copy(data[0:size])
    {
        #pragma acc loop gang
        for (int i = 0; i < size/32; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < size) {
                    data[idx] = (i + j) * 1.5f;
                }
            }
        }
    }
    
    printf("Worker partitioned: data[31] = %f, data[63] = %f\n", data[31], data[63]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[M][M]) {
    float tile_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:M][0:M]) reduction(+:tile_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                data[i][j] = (i * M + j) * 0.25f;
                if (i == j) {
                    tile_sum += data[i][j];
                }
            }
        }
    }
    
    printf("Gang+worker partitioned: tile_sum = %f, data[15][15] = %f\n", 
           tile_sum, data[15][15]);
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
void test_gang_vector_partitioned(float *data, int size) {
    #pragma acc parallel loop gang vector copy(data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = sinf(data[i]) * cosf(data[i]);
    }
    
    printf("Gang+vector partitioned: data[256] = %f\n", data[256]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data[M][M]) {
    #pragma acc parallel copy(data[0:M][0:M])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            #pragma acc loop worker vector
            for (int idx = 0; idx < M*M/4; idx++) {
                int i = (block * M/4) + (idx / M);
                int j = idx % M;
                if (i < M && j < M) {
                    data[i][j] = data[i][j] * 0.9f + 0.1f;
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: data[0][0] = %f\n", data[0][0]);
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float data3d[M][M][P]) {
    float global_sum = 0.0f;
    
    #pragma acc parallel copy(data3d[0:M][0:M][0:P]) reduction(+:global_sum)
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < P-1; k++) {
                    /* 3D stencil computation */
                    data3d[i][j][k] = (data3d[i-1][j][k] + 
                                      data3d[i][j-1][k] + 
                                      data3d[i][j][k-1] + 
                                      data3d[i+1][j][k] + 
                                      data3d[i][j+1][k] + 
                                      data3d[i][j][k+1]) / 6.0f;
                    global_sum += data3d[i][j][k];
                }
            }
        }
    }
    
    printf("Fully partitioned: global_sum = %f, center = %f\n", 
           global_sum, data3d[M/2][M/2][P/2]);
}

/* Helper to initialize arrays */
void init_arrays(float *a, float *b, float *c, int size,
                 float data2d[M][M], float data3d[M][M][P]) {
    for (int i = 0; i < size; i++) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
        c[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data2d[i][j] = (i * M + j) * 0.05f;
            for (int k = 0; k < P; k++) {
                data3d[i][j][k] = (i * M * P + j * P + k) * 0.01f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test data */
    float *data1d = (float*)malloc(N * sizeof(float));
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float data2d[M][M];
    float data3d[M][M][P];
    
    init_arrays(a, b, c, N, data2d, data3d);
    
    /* Use command-line argument to control which tests run */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Force compiler to analyze all OpenACC regions regardless of execution path */
    if (argc > 2 || test_case == 0) {
        test_gang_redundant(data1d, N);
    }
    
    if (argc > 2 || test_case == 1) {
        memcpy(data1d, a, N * sizeof(float));
        test_gang_partitioned(data1d, N);
    }
    
    if (argc > 2 || test_case == 2) {
        test_worker_partitioned(data1d, N);
    }
    
    if (argc > 2 || test_case == 3) {
        test_gang_worker_partitioned(data2d);
    }
    
    if (argc > 2 || test_case == 4) {
        test_vector_partitioned(a, b, c, N);
    }
    
    if (argc > 2 || test_case == 5) {
        memcpy(data1d, a, N * sizeof(float));
        test_gang_vector_partitioned(data1d, N);
    }
    
    if (argc > 2 || test_case == 6) {
        test_worker_vector_partitioned(data2d);
    }
    
    if (argc > 2 || test_case == 7) {
        test_fully_partitioned(data3d);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Final check: data1d[0] = %f, data2d[0][0] = %f, data3d[0][0][0] = %f\n",
           data1d[0], data2d[0][0], data3d[0][0][0]);
    
    free(data1d);
    free(a);
    free(b);
    free(c);
    
    return 0;
}
