/* test_oacc_partition.c - Test OpenACC partitioning for coverage */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:N]) copyin(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < N; i++) {
            arr[i] = i * 0.5f;
        }
        local_sum = arr[0];  /* Simple assignment in gang-redundant region */
    }
    
    printf("Gang redundant: arr[0]=%f, arr[%d]=%f\n", arr[0], N-1, arr[N-1]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:N]) reduction(+:sum)
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * 2.0f + i;
        sum += arr[i];
    }
    
    printf("Gang partitioned: sum=%f\n", sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (i + j) * 1.5f;
            }
        }
    }
    
    printf("Worker partitioned: arr[0][0]=%f, arr[%d][%d]=%f\n", 
           arr[0][0], M-1, M-1, arr[M-1][M-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M]) {
    float tile_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:M][0:M]) reduction(+:tile_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 0.7f;
                if (i % 2 == 0 && j % 2 == 0) {
                    tile_sum += arr[i][j];
                }
            }
        }
    }
    
    printf("Gang+worker partitioned: tile_sum=%f\n", tile_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr) {
    #pragma acc parallel loop vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        /* Element-wise operations suitable for vectorization */
        arr[i] = arr[i] * arr[i] - 2.0f * arr[i] + 1.0f;
    }
    
    printf("Vector partitioned: arr[10]=%f, arr[100]=%f\n", arr[10], arr[100]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (float)(i * j) / (float)M;
        }
    }
    
    printf("Gang+vector partitioned: arr[5][5]=%f\n", arr[5][5]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] > 0.5f) ? 1.0f : 0.0f;
            }
        }
    }
    
    printf("Worker+vector partitioned: threshold check complete\n");
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr3d[M][M][P]) {
    float global_sum = 0.0f;
    
    #pragma acc parallel copy(arr3d[0:M][0:M][0:P]) reduction(+:global_sum)
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < P-1; k++) {
                    /* Stencil computation with data dependencies */
                    arr3d[i][j][k] = (arr3d[i-1][j][k] + 
                                      arr3d[i][j-1][k] + 
                                      arr3d[i][j][k-1]) * 0.333f;
                    global_sum += arr3d[i][j][k];
                }
            }
        }
    }
    
    printf("Fully partitioned: global_sum=%f\n", global_sum);
}

/* Mixed partitioning with runtime condition */
void test_mixed_partitioning(float *arr1, float arr2[M][M], int use_gpu) {
    if (use_gpu) {
        /* Force compiler to analyze both OpenACC and host paths */
        #pragma acc parallel loop gang copy(arr1[0:N])
        for (int i = 0; i < N; i++) {
            arr1[i] = arr1[i] * 3.14f;
        }
        
        #pragma acc parallel loop worker copy(arr2[0:M][0:M])
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr1[i % N] + arr2[i][j];
            }
        }
    } else {
        /* Host fallback path */
        for (int i = 0; i < N; i++) {
            arr1[i] = arr1[i] * 3.14f;
        }
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr1[i % N] + arr2[i][j];
            }
        }
    }
    
    printf("Mixed partitioning: arr1[50]=%f, arr2[10][10]=%f\n", 
           arr1[50], arr2[10][10]);
}

int main(int argc, char *argv[]) {
    /* Initialize test data */
    float *arr1 = (float*)malloc(N * sizeof(float));
    float arr2[M][M];
    float arr3d[M][M][P];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i / N;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i + j) / (M * 2);
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = (float)(i + j + k) / (M + M + P);
            }
        }
    }
    
    /* Select test based on command line argument */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1);
            break;
        case 1:
            test_gang_partitioned(arr1);
            break;
        case 2:
            test_worker_partitioned(arr2);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            break;
        case 4:
            test_vector_partitioned(arr1);
            break;
        case 5:
            test_gang_vector_partitioned(arr2);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            break;
        case 7:
            test_fully_partitioned(arr3d);
            break;
        case 8:
            test_mixed_partitioning(arr1, arr2, (argc > 2) ? atoi(argv[2]) : 1);
            break;
        default:
            /* Run all tests to ensure all code paths are compiled */
            test_gang_redundant(arr1);
            test_gang_partitioned(arr1);
            test_worker_partitioned(arr2);
            test_gang_worker_partitioned(arr2);
            test_vector_partitioned(arr1);
            test_gang_vector_partitioned(arr2);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr3d);
            test_mixed_partitioning(arr1, arr2, 1);
            printf("All partitioning tests completed\n");
            break;
    }
    
    /* Prevent dead code elimination */
    float final_check = arr1[0] + arr2[0][0] + arr3d[0][0][0];
    printf("Final check value: %f\n", final_check);
    
    free(arr1);
    return 0;
}
