/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *data, int n) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:n]) copy(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            data[i] = i * 0.5f;
        }
        local_sum = 1.0f;  /* Simple assignment in gang-redundant region */
    }
    
    printf("Gang redundant: local_sum = %f, data[0] = %f\n", local_sum, data[0]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    printf("Gang partitioned: sum = %f, data[%d] = %f\n", sum, n-1, data[n-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float data[M][M]) {
    float row_sums[M] = {0};
    
    #pragma acc parallel copy(data[0:M][0:M]) copy(row_sums[0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker reduction(+:row_sums[i])
            for (int j = 0; j < M; j++) {
                data[i][j] = (i + j) * 0.25f;
                row_sums[i] += data[i][j];
            }
        }
    }
    
    printf("Worker partitioned: row_sums[0] = %f, data[%d][%d] = %f\n", 
           row_sums[0], M-1, M-1, data[M-1][M-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[M][M]) {
    float block_sum = 0.0f;
    int block_size = M/4;
    
    #pragma acc parallel copy(data[0:M][0:M]) reduction(+:block_sum)
    {
        #pragma acc loop gang worker collapse(2)
        for (int bi = 0; bi < M; bi += block_size) {
            for (int bj = 0; bj < M; bj += block_size) {
                float local_sum = 0.0f;
                #pragma acc loop vector collapse(2)
                for (int i = bi; i < bi + block_size && i < M; i++) {
                    for (int j = bj; j < bj + block_size && j < M; j++) {
                        data[i][j] = (i * j) * 0.1f;
                        local_sum += data[i][j];
                    }
                }
                block_sum += local_sum;
            }
        }
    }
    
    printf("Gang+worker partitioned: block_sum = %f\n", block_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *a, float *b, float *c, int n) {
    #pragma acc parallel loop vector copy(a[0:n], b[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
    
    printf("Vector partitioned: c[0] = %f, c[%d] = %f\n", c[0], n-1, c[n-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *data, int n) {
    float partial_sums[16] = {0};
    
    #pragma acc parallel copy(data[0:n]) copy(partial_sums[0:16])
    {
        #pragma acc loop gang
        for (int g = 0; g < 16; g++) {
            int start = g * (n/16);
            int end = (g == 15) ? n : (g+1) * (n/16);
            float local_sum = 0.0f;
            
            #pragma acc loop vector reduction(+:local_sum)
            for (int i = start; i < end; i++) {
                data[i] = data[i] * 3.14f;
                local_sum += data[i];
            }
            partial_sums[g] = local_sum;
        }
    }
    
    float total = 0.0f;
    for (int g = 0; g < 16; g++) total += partial_sums[g];
    printf("Gang+vector partitioned: total = %f\n", total);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data[M][M]) {
    float col_sums[M] = {0};
    
    #pragma acc parallel copy(data[0:M][0:M]) copy(col_sums[0:M])
    {
        #pragma acc loop gang
        for (int iter = 0; iter < 4; iter++) {
            #pragma acc loop worker vector collapse(2)
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < M; j++) {
                    data[i][j] = (data[i][j] + i - j) * 0.5f;
                    #pragma acc atomic update
                    col_sums[j] += data[i][j];
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: col_sums[0] = %f, col_sums[%d] = %f\n",
           col_sums[0], M-1, col_sums[M-1]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float data3d[M][M][P]) {
    float total = 0.0f;
    
    #pragma acc parallel copy(data3d[0:M][0:M][0:P]) reduction(+:total)
    {
        #pragma acc loop gang collapse(2)
        for (int plane = 0; plane < 2; plane++) {
            for (int block_i = 0; block_i < M; block_i += 8) {
                #pragma acc loop worker collapse(2)
                for (int block_j = 0; block_j < M; block_j += 8) {
                    for (int k = plane * (P/2); k < (plane+1) * (P/2); k++) {
                        #pragma acc loop vector collapse(2)
                        for (int i = block_i; i < block_i + 8 && i < M; i++) {
                            for (int j = block_j; j < block_j + 8 && j < M; j++) {
                                /* Stencil-like computation with dependencies */
                                float left = (i > 0) ? data3d[i-1][j][k] : 0.0f;
                                float top = (j > 0) ? data3d[i][j-1][k] : 0.0f;
                                float back = (k > 0) ? data3d[i][j][k-1] : 0.0f;
                                
                                data3d[i][j][k] = (left + top + back) * 0.333f + 1.0f;
                                total += data3d[i][j][k];
                            }
                        }
                    }
                }
            }
        }
    }
    
    printf("Fully partitioned: total = %f, data3d[%d][%d][%d] = %f\n",
           total, M-1, M-1, P-1, data3d[M-1][M-1][P-1]);
}

/* Helper function with conditional execution */
void conditional_test(int test_id, int argc) {
    float arr1[N], arr2[N], arr3[N];
    float arr2d[M][M];
    float arr3d[M][M][P];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 0.1f;
        arr2[i] = i * 0.2f;
        arr3[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = (i + j) * 0.01f;
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = (i + j + k) * 0.001f;
            }
        }
    }
    
    /* Use argc to create conditional execution paths */
    if (argc > test_id) {
        switch(test_id) {
            case 0: test_gang_redundant(arr1, N); break;
            case 1: test_gang_partitioned(arr1, N); break;
            case 2: test_worker_partitioned(arr2d); break;
            case 3: test_gang_worker_partitioned(arr2d); break;
            case 4: test_vector_partitioned(arr1, arr2, arr3, N); break;
            case 5: test_gang_vector_partitioned(arr1, N); break;
            case 6: test_worker_vector_partitioned(arr2d); break;
            case 7: test_fully_partitioned(arr3d); break;
        }
    }
}

int main(int argc, char *argv[]) {
    printf("OpenACC Partitioning Test Program\n");
    printf("=================================\n");
    
    /* Force compilation of all test functions by calling them conditionally */
    for (int i = 0; i < 8; i++) {
        conditional_test(i, argc);
    }
    
    /* Additional test with mixed partitioning in same region */
    if (argc > 1) {
        float mixed_arr[N];
        for (int i = 0; i < N; i++) mixed_arr[i] = i * 0.05f;
        
        #pragma acc parallel copy(mixed_arr[0:N])
        {
            /* Mixed partitioning patterns */
            #pragma acc loop gang
            for (int g = 0; g < 4; g++) {
                #pragma acc loop worker
                for (int w = 0; w < 8; w++) {
                    #pragma acc loop vector
                    for (int v = 0; v < (N/32); v++) {
                        int idx = g * (N/4) + w * (N/32) + v;
                        if (idx < N) {
                            mixed_arr[idx] = mixed_arr[idx] * 2.0f + 1.0f;
                        }
                    }
                }
            }
        }
        printf("Mixed partitioning test complete: mixed_arr[0] = %f\n", mixed_arr[0]);
    }
    
    return 0;
}
