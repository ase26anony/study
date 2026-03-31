/* test_openacc_partitions.c
 * Designed to trigger partition string mapping in GCC's omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_openacc_partitions.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 20

/* Routine with explicit partition specification */
#pragma acc routine vec
void increment_element(int *arr, int idx) {
    arr[idx] += 1;
}

/* Gang-partitioned routine */
#pragma acc routine gang
void gang_increment(int *arr, int start, int end) {
    #pragma acc loop gang
    for (int i = start; i < end; i++) {
        arr[i] += 2;
    }
}

/* Test 1: Basic partition combinations */
void test_basic_partitions(int argc) {
    int arr1[N][M];
    int arr2[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i * M + j;
            arr2[i][j] = i * M + j;
        }
    }
    
    /* Case 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels copy(arr2[0:N][0:M]) gang(static:1)
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

/* Test 2: Worker and vector partitions */
void test_worker_vector_partitions(int argc) {
    int arr3[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i * M * P + j * P + k;
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
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] += 3;
                    }
                }
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc kernels copy(arr3[0:N][0:M][0:P]) gang, worker
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] -= 1;
                    }
                }
            }
        }
    }
}

/* Test 3: Vector and combined partitions */
void test_vector_combined_partitions(int argc) {
    int arr4[N][M];
    int arr5[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr4[i][j] = i * M + j;
            arr5[i][j] = i * M + j;
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 5) {
        #pragma acc parallel copy(arr4[0:N][0:M]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    increment_element(&arr4[i][j], 0);
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (argc > 6) {
        #pragma acc kernels copy(arr5[0:N][0:M]) gang, vector
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr5[i][j] += 5;
                }
            }
        }
    }
}

/* Test 4: Worker+vector and fully partitioned */
void test_worker_vector_fully_partitions(int argc) {
    int arr6[N][M][P];
    int arr7[N][M][P];
    
    /* Initialize 3D arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr6[i][j][k] = i * M * P + j * P + k;
                arr7[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (argc > 7) {
        #pragma acc parallel copy(arr6[0:N][0:M][0:P]) worker, vector
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr6[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (argc > 8) {
        #pragma acc kernels copy(arr7[0:N][0:M][0:P]) gang, worker, vector
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr7[i][j][k] += 7;
                    }
                }
            }
        }
    }
}

/* Test 5: Nested regions and device data environment */
void test_nested_and_persistent(int argc) {
    int arr8[N][M];
    int arr9[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr8[i][j] = i * M + j;
            arr9[i][j] = i * M + j;
        }
    }
    
    /* Persistent device data with gang partition */
    #pragma acc enter data copyin(arr8[0:N][0:M]) gang
    
    /* Nested conditional regions */
    if (argc > 9) {
        #pragma acc parallel present(arr8) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Nested worker region */
                #pragma acc kernels worker if(argc > 10)
                {
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        arr8[i][j] += 10;
                    }
                }
            }
        }
    }
    
    /* Collapsed loop with mixed partitions */
    if (argc > 11) {
        #pragma acc parallel loop collapse(2) gang, vector copy(arr9[0:N][0:M])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr9[i][j] += i + j;
            }
        }
    }
    
    /* Exit data */
    #pragma acc exit data copyout(arr8[0:N][0:M])
}

/* Test 6: Routine calls with different partition contexts */
void test_routine_partitions(int argc) {
    int arr10[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr10[i] = i;
    }
    
    /* Call gang-partitioned routine from vector region */
    if (argc > 12) {
        #pragma acc parallel copy(arr10[0:N]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                /* This should trigger partition reconciliation */
                gang_increment(arr10, 0, N);
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test functions with conditional arguments */
    test_basic_partitions(argc);
    test_worker_vector_partitions(argc);
    test_vector_combined_partitions(argc);
    test_worker_vector_fully_partitions(argc);
    test_nested_and_persistent(argc);
    test_routine_partitions(argc);
    
    printf("Test completed.\n");
    
    /* Simple validation */
    int validation_arr[10] = {0};
    #pragma acc parallel copy(validation_arr[0:10]) gang, worker, vector
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            validation_arr[i] = i * 2;
        }
    }
    
    for (int i = 0; i < 10; i++) {
        if (validation_arr[i] != i * 2) {
            printf("Validation error at index %d\n", i);
        }
    }
    
    return 0;
}
