/* test_omp_acc_partitions.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_omp_acc_partitions.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 10
#define M 20
#define P 30

/* Routine with gang partitioning */
#pragma acc routine vec gang
void gang_routine(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] += i;
    }
}

/* Routine with worker partitioning */
#pragma acc routine seq worker
void worker_routine(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] *= 2;
    }
}

/* Routine with vector partitioning */
#pragma acc routine vector
void vector_routine(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] -= 1;
    }
}

/* Test 1: Basic partition combinations */
void test_basic_partitions(int argc) {
    int arr1[N][M][P];
    int arr2[N][M];
    int arr3[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr1[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Case 0: gang redundant */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr1[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc kernels create(arr2[0:N][0:M]) gang(static:1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] = i * j;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 1) {
        #pragma acc parallel copy(arr3[0:N]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                arr3[i] = i * 2;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr1[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 2: Vector partition combinations */
void test_vector_partitions(int argc) {
    int arr4[N][M][P];
    int arr5[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr4[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(arr4[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr4[i][j][k] += k;
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc kernels copy(arr5[0:N][0:M]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr5[i][j] = i + j;
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (argc > 2) {
        #pragma acc parallel copy(arr4[0:N][0:M][0:P]) worker vector
        {
            #pragma acc loop worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr4[i][j][k] -= 1;
                    }
                }
            }
        }
    }
}

/* Test 3: Fully partitioned and complex cases */
void test_fully_partitioned(int argc) {
    int arr6[N][M][P];
    int arr7[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr6[i][j][k] = 1;
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr6[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr6[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
    
    /* Nested regions with different partitions */
    if (argc > 3) {
        #pragma acc parallel if(argc > 4) gang copy(arr7[0:N][0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Nested worker region */
                #pragma acc parallel worker present(arr7)
                {
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        arr7[i][j] = i * j;
                    }
                }
            }
        }
    }
}

/* Test 4: Device data environment with partitions */
void test_device_data_partitions() {
    int arr8[N][M][P];
    int arr9[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr9[i] = i;
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr8[i][j][k] = 0;
            }
        }
    }
    
    /* Enter data with gang partition */
    #pragma acc enter data copyin(arr8[0:N][0:M][0:P]) gang
    
    /* Compute with worker partition on already allocated data */
    #pragma acc parallel present(arr8) worker
    {
        #pragma acc loop worker collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr8[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    /* Enter data with vector partition */
    #pragma acc enter data copyin(arr9[0:N]) vector
    
    /* Call partitioned routines */
    #pragma acc parallel present(arr9) gang
    {
        gang_routine(arr9, N);
    }
    
    #pragma acc parallel present(arr9) worker
    {
        worker_routine(arr9, N);
    }
    
    #pragma acc parallel present(arr9) vector
    {
        vector_routine(arr9, N);
    }
    
    /* Exit data */
    #pragma acc exit data copyout(arr8[0:N][0:M][0:P])
    #pragma acc exit data copyout(arr9[0:N])
}

/* Test 5: Mixed partition types in sequential regions */
void test_mixed_sequential_partitions(int argc) {
    int arr10[N][M];
    int arr11[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr11[i] = 0;
        for (int j = 0; j < M; j++) {
            arr10[i][j] = i * 10 + j;
        }
    }
    
    /* Sequential regions with different partitions */
    #pragma acc parallel copy(arr10[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr10[i][j] += 5;
            }
        }
    }
    
    if (argc > 5) {
        #pragma acc kernels copy(arr11[0:N]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                arr11[i] = arr10[i][0];
            }
        }
    }
    
    #pragma acc parallel copy(arr10[0:N][0:M]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr10[i][j] *= 2;
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all tests with argc for conditional paths */
    test_basic_partitions(argc);
    test_vector_partitions(argc);
    test_fully_partitioned(argc);
    test_device_data_partitions();
    test_mixed_sequential_partitions(argc);
    
    printf("Tests completed (compile-time coverage target achieved).\n");
    
    /* Simple runtime validation */
    int validation_arr[5] = {0};
    #pragma acc parallel copy(validation_arr[0:5]) gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 5; i++) {
            validation_arr[i] = i + 1;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        if (validation_arr[i] != i + 1) {
            printf("Validation error at index %d\n", i);
        }
    }
    
    return 0;
}
