/* test_openacc_partitions.c - Cover partition string mapping in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Routine with explicit partition */
#pragma acc routine seq
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
}

/* Gang-partitioned routine */
#pragma acc routine gang
void gang_operation(int *arr, int size, int factor) {
    #pragma acc loop gang
    for (int i = 0; i < size; i++) {
        arr[i] *= factor;
    }
}

/* Vector-partitioned routine */
#pragma acc routine vector
void vector_operation(int *arr, int size, int offset) {
    #pragma acc loop vector
    for (int i = 0; i < size; i++) {
        arr[i] += offset;
    }
}

/* Test 1: Basic partition combinations */
void test_basic_partitions(int argc) {
    int arr1[N][M];
    int arr2[N][M];
    
    /* Initialize arrays */
    #pragma acc parallel loop gang, worker collapse(2) copy(arr1[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i * M + j;
        }
    }
    
    /* Gang redundant (case 0) */
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
    
    /* Gang partitioned (case 1) */
    #pragma acc kernels create(arr2[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr1[i][j] * 2;
            }
        }
    }
    
    /* Worker partitioned (case 2) */
    #pragma acc parallel loop worker copy(arr1[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] -= 5;
        }
    }
}

/* Test 2: Multi-dimensional with complex partitions */
void test_multi_dimensional(int argc) {
    int arr3d[P][M][N];
    
    /* Fully partitioned (case 7) */
    #pragma acc enter data copyin(arr3d[0:P][0:M][0:N]) gang, worker, vector
    
    /* Gang+worker partitioned (case 3) */
    #pragma acc parallel present(arr3d) gang, worker
    {
        #pragma acc loop gang
        for (int i = 0; i < P; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < N; k++) {
                    arr3d[i][j][k] = i * M * N + j * N + k;
                }
            }
        }
    }
    
    /* Vector partitioned (case 4) */
    if (argc > 2) {
        #pragma acc parallel loop vector collapse(3) copy(arr3d[0:P][0:M][0:N])
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < N; k++) {
                    arr3d[i][j][k] *= 3;
                }
            }
        }
    }
    
    /* Gang+vector partitioned (case 5) */
    #pragma acc kernels copy(arr3d[0:P][0:M][0:N]) gang, vector
    {
        #pragma acc loop gang
        for (int i = 0; i < P; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < N; k++) {
                    arr3d[i][j][k] += 100;
                }
            }
        }
    }
    
    #pragma acc exit data copyout(arr3d[0:P][0:M][0:N])
}

/* Test 3: Nested regions and routine calls */
void test_nested_regions(int argc) {
    int arr4[N][M][P];
    int temp[N];
    
    /* Initialize with device data region */
    #pragma acc enter data create(arr4[0:N][0:M][0:P], temp[0:N]) gang
    
    /* Worker+vector partitioned (case 6) */
    #pragma acc parallel present(arr4, temp) worker, vector
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            temp[i] = 0;
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr4[i][j][k] = i + j + k;
                    temp[i] += arr4[i][j][k];
                }
            }
        }
    }
    
    /* Nested parallel region with conditional */
    if (argc > 3) {
        #pragma acc parallel present(arr4) if(argc > 10) gang
        {
            /* Inner region with different partition */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc parallel loop vector present(arr4)
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr4[i][j][k] /= 2;
                    }
                }
            }
        }
    }
    
    /* Call partitioned routines */
    #pragma acc parallel present(temp) gang
    {
        gang_operation(temp, N, 2);
    }
    
    #pragma acc parallel loop present(temp) vector
    for (int i = 0; i < N; i++) {
        vector_operation(&temp[i], 1, i);
    }
    
    #pragma acc exit data copyout(arr4[0:N][0:M][0:P], temp[0:N])
}

/* Test 4: Sequential regions with mixed partitions */
void test_mixed_partitions() {
    int arr5[N][M];
    int arr6[M][P];
    
    /* Mixed partition types in sequence */
    #pragma acc parallel loop gang, worker copy(arr5[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr5[i][j] = 1;
        }
    }
    
    #pragma acc kernels loop worker, vector copy(arr6[0:M][0:P])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr6[i][j] = arr5[i % N][j % M];
        }
    }
    
    /* Combined gang+worker+vector (fully partitioned) */
    #pragma acc parallel copy(arr5[0:N][0:M], arr6[0:M][0:P]) gang, worker, vector
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 1; k++) {  /* Dummy vector loop */
                    arr5[i][j] += arr6[j % M][i % P];
                }
            }
        }
    }
}

/* Test 5: Complex data clauses with partitions */
void test_complex_data_clauses(int argc) {
    int *dyn_arr = (int*)malloc(N * M * sizeof(int));
    int static_arr[N][M];
    
    if (!dyn_arr) return;
    
    /* Present clause with partition */
    #pragma acc enter data copyin(dyn_arr[0:N*M]) gang
    #pragma acc enter data create(static_arr[0:N][0:M]) worker
    
    /* Multiple data clauses with different partitions */
    #pragma acc parallel present(dyn_arr[0:N*M], static_arr[0:N][0:M]) \
                copyout(static_arr[0:N][0:M]) gang, vector
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                dyn_arr[i * M + j] = i * j;
                static_arr[i][j] = dyn_arr[i * M + j] % 100;
            }
        }
    }
    
    if (argc > 4) {
        /* Conditional update with different partition */
        #pragma acc update self(static_arr[0:N][0:M]) if(argc > 5) gang
    }
    
    #pragma acc exit data delete(dyn_arr[0:N*M], static_arr[0:N][0:M])
    free(dyn_arr);
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all tests with argc-based conditions to prevent dead code elimination */
    test_basic_partitions(argc);
    test_multi_dimensional(argc);
    test_nested_regions(argc);
    test_mixed_partitions();
    test_complex_data_clauses(argc);
    
    /* Final validation region */
    int final_check = 0;
    int check_arr[10] = {0};
    
    #pragma acc parallel loop reduction(+:final_check) gang, worker, vector copy(check_arr[0:10])
    for (int i = 0; i < 10; i++) {
        check_arr[i] = i * 2;
        final_check += check_arr[i];
    }
    
    printf("Final check value: %d\n", final_check);
    printf("Expected: %d\n", 90);  /* Sum of 0*2 + 1*2 + ... + 9*2 */
    
    return (final_check == 90) ? 0 : 1;
}
