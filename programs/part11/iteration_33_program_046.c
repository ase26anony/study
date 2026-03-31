/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float sum = 0.0f;
    
    /* Simple parallel region without loop - should be gang redundant */
    #pragma acc parallel copy(arr[0:n]) copyin(n) reduction(+:sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5f;
            sum += arr[i];
        }
    }
    
    printf("Gang redundant: sum = %f, arr[0] = %f, arr[%d] = %f\n", 
           sum, arr[0], n-1, arr[n-1]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float total = 0.0f;
    
    /* Explicit gang partitioning on outer loop */
    #pragma acc parallel copy(arr[0:n]) copyin(n) reduction(+:total)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            float local_sum = 0.0f;
            #pragma acc loop worker reduction(+:local_sum)
            for (int j = 0; j < 8; j++) {
                local_sum += (i + j) * 0.1f;
            }
            arr[i] = local_sum;
            total += local_sum;
        }
    }
    
    printf("Gang partitioned: total = %f\n", total);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *arr, int n) {
    /* Worker-only partitioning */
    #pragma acc parallel copy(arr[0:n]) copyin(n)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            float temp = 0.0f;
            #pragma acc loop vector reduction(+:temp)
            for (int j = 0; j < 16; j++) {
                temp += (i * j) * 0.01f;
            }
            arr[i] = temp;
        }
    }
    
    printf("Worker partitioned: arr[10] = %f, arr[100] = %f\n", 
           arr[10], arr[100]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M]) {
    float max_val = 0.0f;
    
    #pragma acc parallel copy(arr[0:M][0:M]) reduction(max:max_val)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = (i * M + j) * 0.5f;
                if (arr[i][j] > max_val) max_val = arr[i][j];
            }
        }
    }
    
    printf("Gang+worker partitioned: max = %f\n", max_val);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    /* Vector-only partitioning for element-wise operations */
    #pragma acc parallel copy(arr[0:n]) copyin(n)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 2.0f + 1.0f;
        }
    }
    
    printf("Vector partitioned: arr[0] = %f, arr[%d] = %f\n", 
           arr[0], n-1, arr[n-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:M][0:M]) reduction(+:sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + i - j) * 0.3f;
                sum += arr[i][j];
            }
        }
    }
    
    printf("Gang+vector partitioned: sum = %f\n", sum);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *arr, int n) {
    #pragma acc parallel copy(arr[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            /* Complex enough to require both worker and vector partitioning */
            float x = arr[i];
            for (int k = 0; k < 4; k++) {
                x = x * 0.9f + 0.1f;
            }
            arr[i] = x;
        }
    }
    
    printf("Worker+vector partitioned: arr[50] = %f\n", arr[50]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr3d[P][M][M]) {
    float global_max = -1e9f;
    
    /* Triple-nested loop with explicit partitioning at all levels */
    #pragma acc parallel copy(arr3d[0:P][0:M][0:M]) reduction(max:global_max)
    {
        #pragma acc loop gang
        for (int k = 0; k < P; k++) {
            #pragma acc loop worker
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    /* Stencil computation requiring careful partitioning */
                    arr3d[k][i][j] = (arr3d[k][i-1][j] + 
                                     arr3d[k][i][j-1] + 
                                     arr3d[k][i][j]) * 0.333f;
                    if (arr3d[k][i][j] > global_max) {
                        global_max = arr3d[k][i][j];
                    }
                }
            }
        }
    }
    
    printf("Fully partitioned: global_max = %f\n", global_max);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *a, float *b, int n) {
    #pragma acc kernels copy(a[0:n], b[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2.0f;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + 1.0f;
        }
        
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            b[i] = b[i] * 0.5f;
        }
    }
    
    printf("Kernels partitioning: b[0] = %f, b[%d] = %f\n", 
           b[0], n-1, b[n-1]);
}

int main(int argc, char *argv[]) {
    /* Initialize test data */
    float arr1[N];
    float arr2[M][M];
    float arr3d[P][M][M];
    float arr_a[N], arr_b[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 0.1f;
        arr_a[i] = i * 0.2f;
        arr_b[i] = i * 0.3f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (i + j) * 0.5f;
            for (int k = 0; k < P; k++) {
                arr3d[k][i][j] = (i + j + k) * 0.1f;
            }
        }
    }
    
    /* Use argc to control which tests run, ensuring all code is compiled */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9;  /* 0-8 for different tests */
    }
    
    /* Force compilation of all OpenACC regions regardless of execution path */
    if (argc > 2) {
        /* Test all partitioning types */
        test_gang_redundant(arr1, N);
        test_gang_partitioned(arr1, N);
        test_worker_partitioned(arr1, N);
        test_gang_worker_partitioned(arr2);
        test_vector_partitioned(arr1, N);
        test_gang_vector_partitioned(arr2);
        test_worker_vector_partitioned(arr1, N);
        test_fully_partitioned(arr3d);
        test_kernels_partitioning(arr_a, arr_b, N);
    } else {
        /* Execute based on test_case to ensure runtime variability */
        switch (test_case) {
            case 0:
                test_gang_redundant(arr1, N);
                break;
            case 1:
                test_gang_partitioned(arr1, N);
                break;
            case 2:
                test_worker_partitioned(arr1, N);
                break;
            case 3:
                test_gang_worker_partitioned(arr2);
                break;
            case 4:
                test_vector_partitioned(arr1, N);
                break;
            case 5:
                test_gang_vector_partitioned(arr2);
                break;
            case 6:
                test_worker_vector_partitioned(arr1, N);
                break;
            case 7:
                test_fully_partitioned(arr3d);
                break;
            case 8:
                test_kernels_partitioning(arr_a, arr_b, N);
                break;
            default:
                /* Fallback: run a subset */
                test_gang_redundant(arr1, N);
                test_fully_partitioned(arr3d);
                break;
        }
    }
    
    /* Print some results to prevent dead code elimination */
    printf("Final check: arr1[0] = %f, arr2[0][0] = %f, arr3d[0][0][0] = %f\n",
           arr1[0], arr2[0][0], arr3d[0][0][0]);
    
    return 0;
}
