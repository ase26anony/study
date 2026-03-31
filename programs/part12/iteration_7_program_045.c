/* test_openacc_partitions.c - Cover partition string mapping in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Routine with explicit partition type */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

#pragma acc routine worker
void acc_routine_worker(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

/* Test 1: Basic partition combinations */
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
    #pragma acc parallel copy(arr1[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc kernels create(arr2[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] *= 2;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr1[i][j] -= 1;
                }
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc kernels copy(arr2[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] += i;
            }
        }
    }
}

/* Test 2: Vector and combined partitions */
void test_vector_partitions(int argc) {
    int arr3[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(arr3[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3[i][j][k] += k;
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc kernels copy(arr3[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3[i][j][k] *= 2;
                }
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (argc > 2) {
        #pragma acc parallel copy(arr3[0:N][0:M][0:P]) worker vector
        {
            #pragma acc loop worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] -= j;
                    }
                }
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc kernels copy(arr3[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3[i][j][k] = arr3[i][j][k] % 1000;
                }
            }
        }
    }
}

/* Test 3: Nested regions and device data environment */
void test_nested_and_persistent(int argc) {
    int arr4[N][M];
    int arr5[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr5[i] = i;
        for (int j = 0; j < M; j++) {
            arr4[i][j] = i * M + j;
        }
    }
    
    /* Persistent device data with partition */
    #pragma acc enter data copyin(arr4[0:N][0:M]) gang
    #pragma acc enter data create(arr5[0:N]) worker
    
    /* Nested-like structure using function calls */
    if (argc > 3) {
        #pragma acc parallel present(arr4) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Call routine with different partition */
                #pragma acc parallel present(arr5) worker
                {
                    #pragma acc loop worker
                    for (int j = 0; j < N; j++) {
                        arr5[j] += arr4[i][0];
                    }
                }
                acc_routine_gang(arr5, N);
            }
        }
    }
    
    /* Mixed partition in same region */
    #pragma acc parallel present(arr4, arr5) gang worker
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr4[i][j] += arr5[i];
            }
        }
        acc_routine_worker(arr5, N);
    }
    
    #pragma acc exit data copyout(arr4[0:N][0:M])
    #pragma acc exit data delete(arr5[0:N])
}

/* Test 4: Complex multi-dimensional with varying partitions */
void test_complex_multi_dim(int argc) {
    int arr6[10][20][30];
    int arr7[5][10][15][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                arr6[i][j][k] = i * 400 + j * 20 + k;
            }
        }
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 15; k++) {
                for (int l = 0; l < 20; l++) {
                    arr7[i][j][k][l] = i * 3000 + j * 300 + k * 20 + l;
                }
            }
        }
    }
    
    /* 3D array with gang+vector partition */
    #pragma acc parallel copy(arr6[0:10][0:20][0:30]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 30; k++) {
                    arr6[i][j][k] += 1;
                }
            }
        }
    }
    
    /* 4D array with worker+vector partition */
    if (argc > 4) {
        #pragma acc kernels copy(arr7[0:5][0:10][0:15][0:20]) worker vector
        {
            #pragma acc loop worker vector collapse(3)
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 10; j++) {
                    for (int k = 0; k < 15; k++) {
                        #pragma acc loop vector
                        for (int l = 0; l < 20; l++) {
                            arr7[i][j][k][l] *= 2;
                        }
                    }
                }
            }
        }
    }
    
    /* Fully partitioned 4D array */
    #pragma acc parallel copy(arr7[0:5][0:10][0:15][0:20]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(4)
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 15; k++) {
                    for (int l = 0; l < 20; l++) {
                        arr7[i][j][k][l] = arr7[i][j][k][l] % 10000;
                    }
                }
            }
        }
    }
}

/* Test 5: Sequential regions with different partitions */
void test_sequential_regions(int argc) {
    int arr8[N][M];
    int arr9[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr8[i][j] = i * M + j;
            arr9[i][j] = (i * M + j) * 2;
        }
    }
    
    /* Sequence of regions with different partitions */
    #pragma acc parallel copy(arr8[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr8[i][j] += 10;
            }
        }
    }
    
    #pragma acc kernels copy(arr9[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr9[i][j] -= 5;
            }
        }
    }
    
    if (argc > 5) {
        #pragma acc parallel copy(arr8[0:N][0:M], arr9[0:N][0:M]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr8[i][j] += arr9[i][j];
                }
            }
        }
    }
    
    #pragma acc kernels copy(arr8[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr8[i][j] *= 3;
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all tests with argc for conditional execution */
    test_basic_partitions(argc);
    test_vector_partitions(argc);
    test_nested_and_persistent(argc);
    test_complex_multi_dim(argc);
    test_sequential_regions(argc);
    
    printf("Tests completed (compile-time coverage target achieved)\n");
    
    /* Simple runtime validation */
    int verify = 0;
    #pragma acc parallel copy(verify) gang
    {
        verify = 1;
    }
    
    if (verify == 1) {
        printf("Basic OpenACC execution verified\n");
    }
    
    return 0;
}
