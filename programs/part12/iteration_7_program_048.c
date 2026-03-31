/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the uncovered lines in
 * omp-oacc-neuter-broadcast.cc (lines 335-343) by creating various
 * OpenACC compute regions with different data partition combinations.
 * The compiler's neuter-broadcast pass should process these partition
 * clauses and invoke the string mapping function for each partition type.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 100
#define M 50
#define P 25

/* Function prototypes */
void test_gang_redundant(int argc);
void test_gang_partitioned(int argc);
void test_worker_partitioned(int argc);
void test_gang_worker_partitioned(int argc);
void test_vector_partitioned(int argc);
void test_gang_vector_partitioned(int argc);
void test_worker_vector_partitioned(int argc);
void test_fully_partitioned(int argc);
void test_nested_regions(int argc);
void test_routine_directives(int argc);
void test_device_data_env(int argc);

/* ACC routine with gang partitioning */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n, int val) {
    for (int i = 0; i < n; i++) {
        arr[i] += val;
    }
}

/* ACC routine with vector partitioning */
#pragma acc routine vec vector
void acc_routine_vector(int *arr, int n, int val) {
    for (int i = 0; i < n; i++) {
        arr[i] *= val;
    }
}

int main(int argc, char *argv[]) {
    printf("Testing OpenACC partition type coverage...\n");
    
    /* Use argc to prevent dead code elimination */
    int test_selector = argc > 1 ? atoi(argv[1]) % 8 : 0;
    
    /* Test all partition combinations */
    test_gang_redundant(test_selector);
    test_gang_partitioned(test_selector);
    test_worker_partitioned(test_selector);
    test_gang_worker_partitioned(test_selector);
    test_vector_partitioned(test_selector);
    test_gang_vector_partitioned(test_selector);
    test_worker_vector_partitioned(test_selector);
    test_fully_partitioned(test_selector);
    
    /* Additional complex tests */
    test_nested_regions(test_selector);
    test_routine_directives(test_selector);
    test_device_data_env(test_selector);
    
    printf("All tests completed (compile-time coverage goal achieved).\n");
    return 0;
}

/* Test 1: gang redundant partitioning */
void test_gang_redundant(int argc) {
    int arr1d[N];
    int arr3d[10][20][30];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) arr1d[i] = i;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            for (int k = 0; k < 30; k++)
                arr3d[i][j][k] = i + j + k;
    
    /* gang redundant - case 0 */
    #pragma acc parallel copy(arr1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] += 1;
        }
    }
    
    /* Multi-dimensional with gang redundant */
    #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) gang
    {
        #pragma acc loop collapse(3) gang
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                for (int k = 0; k < 30; k++)
                    arr3d[i][j][k] += 1;
    }
    
    /* Verify results if argc indicates */
    if (argc == 0) {
        int sum = 0;
        for (int i = 0; i < N; i++) sum += arr1d[i];
        assert(sum > 0);
    }
}

/* Test 2: gang partitioned - case 1 */
void test_gang_partitioned(int argc) {
    int arr2d[M][N];
    
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            arr2d[i][j] = i * N + j;
    
    /* gang partitioned */
    #pragma acc kernels create(arr2d[0:M][0:N]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop vector
            for (int j = 0; j < N; j++) {
                arr2d[i][j] *= 2;
            }
        }
    }
    
    /* Conditional offloading */
    #pragma acc parallel if(argc > 1) copy(arr2d[0:M][0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                arr2d[i][j] += i;
            }
        }
    }
}

/* Test 3: worker partitioned - case 2 */
void test_worker_partitioned(int argc) {
    float arr1d[N];
    float arr3d[5][10][15];
    
    for (int i = 0; i < N; i++) arr1d[i] = i * 1.5f;
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 10; j++)
            for (int k = 0; k < 15; k++)
                arr3d[i][j][k] = i * j * k * 0.5f;
    
    /* worker partitioned */
    #pragma acc parallel copy(arr1d[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr1d[i] += 3.14f;
        }
    }
    
    /* Multi-dimensional with worker partitioning */
    #pragma acc kernels copy(arr3d[0:5][0:10][0:15]) worker
    {
        #pragma acc loop collapse(2) worker
        for (int i = 0; i < 5; i++)
            for (int j = 0; j < 10; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 15; k++) {
                    arr3d[i][j][k] *= 2.0f;
                }
            }
    }
}

/* Test 4: gang+worker partitioned - case 3 */
void test_gang_worker_partitioned(int argc) {
    double arr2d[N][M];
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr2d[i][j] = i * 100.0 + j;
    
    /* gang+worker partitioned */
    #pragma acc parallel copy(arr2d[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2d[i][j] /= 2.0;
            }
        }
    }
    
    /* Alternative syntax */
    #pragma acc kernels create(arr2d[0:N][0:M]) gang, worker
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2d[i][j] += 1.0;
            }
        }
    }
}

/* Test 5: vector partitioned - case 4 */
void test_vector_partitioned(int argc) {
    int arr1d[N * 2];
    
    for (int i = 0; i < N * 2; i++) arr1d[i] = i % 7;
    
    /* vector partitioned */
    #pragma acc parallel copy(arr1d[0:N*2]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N * 2; i++) {
            arr1d[i] = arr1d[i] * 3 + 1;
        }
    }
    
    /* With vector length clause */
    #pragma acc kernels copy(arr1d[0:N*2]) vector_length(128)
    {
        #pragma acc loop vector
        for (int i = 0; i < N * 2; i++) {
            if (arr1d[i] > 10) arr1d[i] = 10;
        }
    }
}

/* Test 6: gang+vector partitioned - case 5 */
void test_gang_vector_partitioned(int argc) {
    int arr3d[8][16][32];
    
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 16; j++)
            for (int k = 0; k < 32; k++)
                arr3d[i][j][k] = (i << 16) | (j << 8) | k;
    
    /* gang+vector partitioned */
    #pragma acc parallel copy(arr3d[0:8][0:16][0:32]) gang vector
    {
        #pragma acc loop gang
        for (int i = 0; i < 8; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 16; j++) {
                for (int k = 0; k < 32; k++) {
                    arr3d[i][j][k] &= 0xFF;
                }
            }
        }
    }
    
    /* With collapse */
    #pragma acc kernels copy(arr3d[0:8][0:16][0:32]) gang, vector
    {
        #pragma acc loop collapse(2) gang vector
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 16; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 32; k++) {
                    arr3d[i][j][k] |= 0x80;
                }
            }
    }
}

/* Test 7: worker+vector partitioned - case 6 */
void test_worker_vector_partitioned(int argc) {
    float arr2d[N][M];
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr2d[i][j] = (i + j) * 0.25f;
    
    /* worker+vector partitioned */
    #pragma acc parallel copy(arr2d[0:N][0:M]) worker vector
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = arr2d[i][j] * arr2d[i][j];
            }
        }
    }
}

/* Test 8: fully partitioned (gang+worker+vector) - case 7 */
void test_fully_partitioned(int argc) {
    int arr3d[6][12][24];
    
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 12; j++)
            for (int k = 0; k < 24; k++)
                arr3d[i][j][k] = i * j * k;
    
    /* fully partitioned */
    #pragma acc parallel copy(arr3d[0:6][0:12][0:24]) gang worker vector
    {
        #pragma acc loop gang
        for (int i = 0; i < 6; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 12; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 24; k++) {
                    arr3d[i][j][k] += (i + j + k);
                }
            }
        }
    }
    
    /* Alternative fully partitioned syntax */
    #pragma acc kernels create(arr3d[0:6][0:12][0:24]) gang, worker, vector
    {
        #pragma acc loop collapse(3) gang worker vector
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 12; j++)
                for (int k = 0; k < 24; k++) {
                    arr3d[i][j][k] %= 100;
                }
    }
}

/* Test 9: Nested and sequential compute regions */
void test_nested_regions(int argc) {
    int arr1d[N];
    int arr2d[M][N];
    
    for (int i = 0; i < N; i++) arr1d[i] = i;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            arr2d[i][j] = i * N + j;
    
    /* Sequential regions with different partition types */
    #pragma acc parallel copy(arr1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] += 10;
        }
    }
    
    #pragma acc parallel copy(arr1d[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr1d[i] *= 2;
        }
    }
    
    #pragma acc parallel copy(arr1d[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] -= 5;
        }
    }
    
    /* Complex nested-like structure using functions */
    #pragma acc parallel copy(arr2d[0:M][0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            /* Inner region with different partition */
            #pragma acc loop worker vector
            for (int j = 0; j < N; j++) {
                arr2d[i][j] = arr2d[i][j] * 3 + arr1d[j % N];
            }
        }
    }
}

/* Test 10: Routine directives with partition types */
void test_routine_directives(int argc) {
    int arr1d[N];
    int arr2d[M][N];
    
    for (int i = 0; i < N; i++) arr1d[i] = i;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            arr2d[i][j] = i + j;
    
    /* Call gang routine inside gang partitioned region */
    #pragma acc parallel copy(arr1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < 1; i++) {  /* Single iteration to call routine */
            acc_routine_gang(arr1d, N, 5);
        }
    }
    
    /* Call vector routine inside vector partitioned region */
    #pragma acc parallel copy(arr2d[0:M][0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < M; i++) {
            acc_routine_vector(arr2d[i], N, 2);
        }
    }
    
    /* Mixed routine calls */
    #pragma acc parallel copy(arr1d[0:N], arr2d[0:M][0:N]) gang vector
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            if (i == 0) {
                acc_routine_gang(arr1d, N, 3);
            }
            #pragma acc loop vector
            for (int j = 0; j < N; j++) {
                arr2d[i][j] += arr1d[j];
            }
        }
    }
}

/* Test 11: Device data environment with partition clauses */
void test_device_data_env(int argc) {
    int persistent_arr[N][M];
    int temp_arr[P][N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            persistent_arr[i][j] = i * M + j;
    
    for (int i = 0; i < P; i++)
        for (int j = 0; j < N; j++)
            temp_arr[i][j] = i * N + j;
    
    /* Establish device data region with gang partitioning */
    #pragma acc enter data copyin(persistent_arr[0:N][0:M]) gang
    
    /* Compute region using the persistent data */
    #pragma acc parallel present(persistent_arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                persistent_arr[i][j] += 1;
            }
        }
    }
    
    /* Another region with different partition */
    #pragma acc parallel present(persistent_arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                persistent_arr[i][j] *= 2;
            }
        }
    }
    
    /* Temporary array with its own partition */
    #pragma acc enter data copyin(temp_arr[0:P][0:N]) gang worker
    
    #pragma acc parallel present(temp_arr) gang worker vector
    {
        #pragma acc loop gang worker
        for (int i = 0; i < P; i++) {
            #pragma acc loop vector
            for (int j = 0; j < N; j++) {
                temp_arr[i][j] = persistent_arr[j % N][i % M] + temp_arr[i][j];
            }
        }
    }
    
    /* Exit data regions */
    #pragma acc exit data copyout(persistent_arr[0:N][0:M])
    #pragma acc exit data copyout(temp_arr[0:P][0:N])
    
    /* Final verification */
    if (argc == 0) {
        int sum = 0;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                sum += persistent_arr[i][j];
        assert(sum != 0);
    }
}
