/* test_omp_acc_partitions.c */
#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Routine with explicit partition specification */
#pragma acc routine vec
void increment_element(int *arr, int idx, int value) {
    arr[idx] += value;
}

/* Gang-partitioned routine */
#pragma acc routine gang
void gang_routine(int *arr, int start, int end) {
    #pragma acc loop gang
    for (int i = start; i < end; i++) {
        arr[i] *= 2;
    }
}

/* Test function 1: Basic partition combinations */
void test_basic_partitions(int argc) {
    int arr1[N][M];
    int arr2[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = i * j;
        }
    }
    
    /* Case 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels copy(arr2[0:N][0:M]) gang(static:2)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    arr2[i][j] *= 2;
                }
            }
        }
    }
}

/* Test function 2: Worker and vector partitions */
void test_worker_vector_partitions(int argc) {
    int arr3[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 3) {
        #pragma acc parallel create(arr3[0:N][0:M][0:P]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] += k;
                    }
                }
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc kernels copy(arr3[0:N][0:M][0:P]) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] *= 2;
                    }
                }
            }
        }
    }
}

/* Test function 3: Vector and combined partitions */
void test_vector_combined_partitions(int argc) {
    int arr4[N][M];
    int arr5[N][M][P];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr4[i][j] = i - j;
            for (int k = 0; k < P; k++) {
                arr5[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 5) {
        #pragma acc parallel copy(arr4[0:N][0:M]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                #pragma acc loop seq
                for (int j = 0; j < M; j++) {
                    increment_element(&arr4[i][j], 0, 10);
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (argc > 6) {
        #pragma acc kernels create(arr5[0:N][0:M][0:P]) gang vector
        {
            #pragma acc loop gang vector collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr5[i][j][k] += 5;
                    }
                }
            }
        }
    }
}

/* Test function 4: Worker+vector and fully partitioned */
void test_worker_vector_full_partitions(int argc) {
    int arr6[N][M][P];
    int arr7[N][M][P];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr6[i][j][k] = i + j * k;
                arr7[i][j][k] = i * j + k;
            }
        }
    }
    
    /* Persistent device data with partitioning */
    #pragma acc enter data copyin(arr6[0:N][0:M][0:P]) gang
    #pragma acc enter data copyin(arr7[0:N][0:M][0:P]) worker
    
    /* Case 6: worker+vector partitioned */
    if (argc > 7) {
        #pragma acc parallel present(arr6) worker vector
        {
            #pragma acc loop worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr6[i][j][k] -= 3;
                    }
                }
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (argc > 8) {
        #pragma acc kernels present(arr7) gang worker vector
        {
            #pragma acc loop gang worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr7[i][j][k] /= 2;
                    }
                }
            }
        }
    }
    
    /* Nested parallel region */
    if (argc > 9) {
        #pragma acc parallel present(arr6) gang
        {
            gang_routine(&arr6[0][0][0], 0, N*M*P);
            
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc kernels present(arr6) worker
                {
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        #pragma acc loop vector
                        for (int k = 0; k < P; k++) {
                            arr6[i][j][k] += i + j + k;
                        }
                    }
                }
            }
        }
    }
    
    #pragma acc exit data copyout(arr6[0:N][0:M][0:P])
    #pragma acc exit data copyout(arr7[0:N][0:M][0:P])
}

/* Test function 5: Mixed partition types in sequential regions */
void test_mixed_sequential_partitions(int argc) {
    int arr8[N][M];
    int arr9[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr8[i][j] = i * 2 + j;
            arr9[i][j] = i + j * 3;
        }
    }
    
    /* Sequential regions with different partitions */
    #pragma acc parallel copy(arr8[0:N][0:M]) if(argc > 10) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr8[i][j] += 100;
            }
        }
    }
    
    #pragma acc kernels copy(arr9[0:N][0:M]) if(argc > 11) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr9[i][j] -= 50;
            }
        }
    }
    
    /* Combined partition with collapse */
    #pragma acc parallel copy(arr8[0:N][0:M], arr9[0:N][0:M]) \
        if(argc > 12) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr8[i][j] = arr8[i][j] + arr9[i][j];
                arr9[i][j] = arr8[i][j] - arr9[i][j];
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test functions with argc-based conditions
       to prevent dead code elimination */
    test_basic_partitions(argc);
    test_worker_vector_partitions(argc);
    test_vector_combined_partitions(argc);
    test_worker_vector_full_partitions(argc);
    test_mixed_sequential_partitions(argc);
    
    /* Final validation on host */
    int validation_arr[10][10];
    #pragma acc parallel copy(validation_arr[0:10][0:10]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                validation_arr[i][j] = i * 10 + j;
            }
        }
    }
    
    /* Verify results */
    int valid = 1;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (validation_arr[i][j] != i * 10 + j) {
                valid = 0;
                break;
            }
        }
        if (!valid) break;
    }
    
    if (valid) {
        printf("All OpenACC partition tests completed (compile-time coverage achieved).\n");
    } else {
        printf("Validation error detected.\n");
    }
    
    return 0;
}
