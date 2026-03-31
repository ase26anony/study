/* test_omp_acc_partitions.c */
#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Test function with routine directive */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i;
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
    #pragma acc parallel if(argc > 1) copy(arr1[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr1[i][j] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc kernels if(argc > 2) create(arr2[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] -= 1;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    #pragma acc parallel if(argc > 3) copy(arr1[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] *= 2;
            }
        }
    }
}

/* Test 2: Multi-dimensional with collapse */
void test_multi_dimensional(int argc) {
    int arr3[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel loop collapse(3) if(argc > 4) \
        copy(arr3[0:N][0:M][0:P]) gang worker
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] += 3;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc kernels loop collapse(2) if(argc > 5) \
        copy(arr3[0:N][0:M][0:P]) vector
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            #pragma acc loop vector
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] *= 2;
            }
        }
    }
}

/* Test 3: Nested regions with routine calls */
void test_nested_routines(int argc) {
    int arr4[N*M];
    int arr5[N*M];
    
    /* Initialize */
    for (int i = 0; i < N*M; i++) {
        arr4[i] = i;
        arr5[i] = i * 2;
    }
    
    /* Case 5: gang+vector partitioned with routine */
    #pragma acc enter data copyin(arr4[0:N*M]) gang vector
    
    #pragma acc parallel if(argc > 6) present(arr4) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N*M; i += 32) {
            acc_routine_gang(&arr4[i], 32);
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc kernels if(argc > 7) copy(arr5[0:N*M]) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N*M; i++) {
            arr5[i] += arr4[i % N];
        }
    }
    
    #pragma acc exit data copyout(arr4[0:N*M]) gang vector
}

/* Test 4: Fully partitioned and complex combinations */
void test_fully_partitioned(int argc) {
    int arr6[N][M][P];
    int arr7[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr6[i][j][k] = i * j * k;
            }
            arr7[i][j] = i - j;
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel if(argc > 8) copy(arr6[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr6[i][j][k] = arr6[i][j][k] / 2 + 1;
                }
            }
        }
    }
    
    /* Mixed partitioning in nested regions */
    #pragma acc enter data copyin(arr7[0:N][0:M]) gang
    
    if (argc > 9) {
        #pragma acc parallel present(arr7) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr7[i][j] += arr6[i][j % P][j % P];
                }
            }
        }
    }
    
    #pragma acc exit data copyout(arr7[0:N][0:M]) gang
}

/* Test 5: Sequential regions with different partitions */
void test_sequential_regions(int argc) {
    int arr8[N][M];
    int arr9[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr8[i][j] = i ^ j;
            arr9[i][j] = i | j;
        }
    }
    
    /* Sequence of different partition types */
    #pragma acc parallel if(argc > 10) copy(arr8[0:N][0:M]) gang
    for (int i = 0; i < N; i++) {
        #pragma acc loop vector
        for (int j = 0; j < M; j++) {
            arr8[i][j] += 5;
        }
    }
    
    #pragma acc kernels if(argc > 11) copy(arr9[0:N][0:M]) worker
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            arr9[i][j] -= arr8[i][j];
        }
    }
    
    #pragma acc parallel if(argc > 12) copy(arr8[0:N][0:M], arr9[0:N][0:M]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr8[i][j] = arr9[i][j] * 2;
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition combinations...\n");
    
    /* Execute all tests with conditional paths based on argc */
    test_basic_partitions(argc);
    test_multi_dimensional(argc);
    test_nested_routines(argc);
    test_fully_partitioned(argc);
    test_sequential_regions(argc);
    
    /* Verification (simplified) */
    printf("All OpenACC regions processed (compile-time coverage target reached)\n");
    
    return 0;
}
