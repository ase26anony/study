/* test_oacc_partition.c - Test OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(int argc) {
    float data[N];
    float sum = 0.0f;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = i * 0.1f;
    }
    
    /* Use argc to ensure both paths are considered */
    if (argc > 1) {
        /* Gang redundant - no loop, just parallel region */
        #pragma acc parallel copy(data[0:N]) copy(sum)
        {
            sum = data[0] + data[N-1];
        }
    } else {
        /* Alternative with gang(1) */
        #pragma acc parallel loop gang(1) copy(data[0:N]) reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("Gang redundant test: sum = %f\n", sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float local_sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:n]) reduction(+:local_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i * 0.01f;
        local_sum += arr[i];
    }
    
    printf("Gang partitioned: local_sum = %f, arr[0] = %f\n", local_sum, arr[0]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *arr, int n) {
    /* Nested loops with worker partitioning on inner loop */
    #pragma acc parallel copy(arr[0:n])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            int start = block * (n / 4);
            int end = start + (n / 4);
            
            #pragma acc loop worker
            for (int i = start; i < end; i++) {
                arr[i] = arr[i] * 3.0f - i * 0.02f;
            }
        }
    }
    
    printf("Worker partitioned: arr[%d] = %f\n", n/2, arr[n/2]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr2d[M][M]) {
    float total = 0.0f;
    
    #pragma acc parallel loop gang worker copy(arr2d[0:M][0:M]) reduction(+:total)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = (float)(i * M + j) * 0.5f;
            total += arr2d[i][j];
        }
    }
    
    printf("Gang+worker partitioned: total = %f, arr2d[0][0] = %f\n", total, arr2d[0][0]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *vec, int n) {
    #pragma acc parallel loop vector copy(vec[0:n])
    for (int i = 0; i < n; i++) {
        /* Element-wise operation suitable for vectorization */
        vec[i] = vec[i] * vec[i] + sinf((float)i * 0.01f);
    }
    
    printf("Vector partitioned: vec[10] = %f, vec[100] = %f\n", vec[10], vec[100]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *data, int n) {
    float max_val = -1e9f;
    
    #pragma acc parallel loop gang vector copy(data[0:n]) reduction(max:max_val)
    for (int i = 0; i < n; i++) {
        data[i] = cosf((float)i * 0.03f) * 100.0f;
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    
    printf("Gang+vector partitioned: max_val = %f\n", max_val);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr2d[M][M]) {
    #pragma acc parallel copy(arr2d[0:M][0:M])
    {
        #pragma acc loop gang
        for (int block_row = 0; block_row < 4; block_row++) {
            int row_start = block_row * (M / 4);
            int row_end = row_start + (M / 4);
            
            #pragma acc loop worker vector
            for (int i = row_start; i < row_end; i++) {
                for (int j = 0; j < M; j++) {
                    /* Stencil-like computation */
                    float left = (j > 0) ? arr2d[i][j-1] : 0.0f;
                    float up = (i > 0) ? arr2d[i-1][j] : 0.0f;
                    arr2d[i][j] = (left + up) * 0.5f + (float)(i + j) * 0.01f;
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: arr2d[%d][%d] = %f\n", 
           M/2, M/2, arr2d[M/2][M/2]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr3d[P][M][M]) {
    float grand_total = 0.0f;
    
    /* Complex nested computation with explicit partitioning at all levels */
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M]) reduction(+:grand_total)
    {
        #pragma acc loop gang
        for (int k = 0; k < P; k++) {
            #pragma acc loop worker
            for (int i = 0; i < M; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    /* 3D stencil computation */
                    float val = 0.0f;
                    if (k > 0) val += arr3d[k-1][i][j] * 0.2f;
                    if (i > 0) val += arr3d[k][i-1][j] * 0.3f;
                    if (j > 0) val += arr3d[k][i][j-1] * 0.3f;
                    if (k < P-1) val += arr3d[k+1][i][j] * 0.2f;
                    
                    arr3d[k][i][j] = val + (float)(k * M * M + i * M + j) * 0.001f;
                    grand_total += arr3d[k][i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned: grand_total = %f, arr3d[0][0][0] = %f\n", 
           grand_total, arr3d[0][0][0]);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *a, float *b, int n) {
    #pragma acc kernels copy(a[0:n], b[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 0.25f;
        }
        
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2.0f;
        }
        
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            b[i] += sinf(a[i]);
        }
    }
    
    printf("Kernels test: b[0] = %f, b[n-1] = %f\n", b[0], b[n-1]);
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test arrays */
    float array1[N];
    float array2[M][M];
    float array3[P][M][M];
    float array4[N];
    float array5[N];
    
    /* Initialize with simple patterns */
    for (int i = 0; i < N; i++) {
        array1[i] = (float)i;
        array4[i] = (float)(i % 100);
        array5[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            array2[i][j] = (float)(i + j);
        }
    }
    
    for (int k = 0; k < P; k++) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                array3[k][i][j] = (float)(k + i + j) * 0.1f;
            }
        }
    }
    
    /* Use argc to control which tests run, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(argc);
            break;
        case 1:
            test_gang_partitioned(array1, N);
            break;
        case 2:
            test_worker_partitioned(array1, N);
            break;
        case 3:
            test_gang_worker_partitioned(array2);
            break;
        case 4:
            test_vector_partitioned(array4, N);
            break;
        case 5:
            test_gang_vector_partitioned(array1, N);
            break;
        case 6:
            test_worker_vector_partitioned(array2);
            break;
        case 7:
            test_fully_partitioned(array3);
            break;
        case 8:
            test_kernels_partitioning(array4, array5, N);
            break;
        default:
            /* Run all tests sequentially */
            test_gang_redundant(argc);
            test_gang_partitioned(array1, N);
            test_worker_partitioned(array1, N);
            test_gang_worker_partitioned(array2);
            test_vector_partitioned(array4, N);
            test_gang_vector_partitioned(array1, N);
            test_worker_vector_partitioned(array2);
            test_fully_partitioned(array3);
            test_kernels_partitioning(array4, array5, N);
            break;
    }
    
    /* Print some results to prevent dead code elimination */
    printf("Final check - array1[0] = %f, array2[0][0] = %f\n", array1[0], array2[0][0]);
    
    return 0;
}
