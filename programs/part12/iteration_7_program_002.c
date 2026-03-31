/* Test program to cover OpenACC partition string mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 16
#define P 8

/* Routine with explicit partition specification */
#pragma acc routine vec gang
void increment_element(float *arr, int i, int j, int k, float value) {
    arr[i * M * P + j * P + k] += value;
}

/* Function with nested compute regions */
void test_nested_partitions(int argc) {
    float arr1[N][M][P];
    float arr2[N][M][P];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr1[i][j][k] = i * 100.0f + j * 10.0f + k;
                arr2[i][j][k] = 0.0f;
            }
        }
    }
    
    /* Test 1: gang redundant partitioning */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr1[i][j][k] += 1.0f;
                    }
                }
            }
        }
    }
    
    /* Test 2: gang partitioned */
    #pragma acc enter data copyin(arr2[0:N][0:M][0:P]) gang(static:2)
    
    #pragma acc parallel present(arr2) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr2[i][j][k] = arr1[i][j][k] * 2.0f;
                }
            }
        }
    }
    
    /* Test 3: worker partitioned */
    float arr3[N][M];
    #pragma acc enter data create(arr3[0:N][0:M]) worker
    
    #pragma acc kernels present(arr3) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr3[i][j] = (float)(i + j);
            }
        }
    }
    
    /* Test 4: gang+worker partitioned */
    float arr4[N][M][P];
    #pragma acc enter data create(arr4[0:N][0:M][0:P]) gang, worker
    
    #pragma acc parallel present(arr4) gang, worker
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr4[i][j][k] = arr1[i][j][k] + arr2[i][j][k];
                }
            }
        }
    }
    
    #pragma acc exit data delete(arr2, arr3, arr4)
}

/* Function with vector partitioning variations */
void test_vector_partitions(int argc) {
    double matrix[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (double)(i * M + j);
        }
    }
    
    /* Test 5: vector partitioned */
    #pragma acc parallel copy(matrix[0:N][0:M]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                matrix[i][j] *= 1.5;
            }
        }
    }
    
    /* Test 6: gang+vector partitioned */
    if (argc > 2) {
        #pragma acc kernels copy(matrix[0:N][0:M]) gang, vector
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    matrix[i][j] += (double)(i + j);
                }
            }
        }
    }
    
    /* Test 7: worker+vector partitioned */
    #pragma acc parallel copy(matrix[0:N][0:M]) worker, vector
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                matrix[i][j] -= (double)(i * j);
            }
        }
    }
}

/* Function with fully partitioned data and collapse clause */
void test_fully_partitioned(int argc) {
    int cube[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* Test 8: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(cube[0:N][0:M][0:P]) gang, worker, vector
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop worker vector
                for (int k = 0; k < P; k++) {
                    cube[i][j][k] += 42;
                    /* Call routine with partition specification */
                    increment_element(&cube[0][0][0], i, j, k, 0.5f);
                }
            }
        }
    }
    
    /* Conditional compute region with different partitioning */
    if (argc > 3) {
        #pragma acc parallel if(argc > 10) copy(cube[0:N][0:M][0:P]) gang(static:4), vector
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        cube[i][j][k] *= 2;
                    }
                }
            }
        }
    }
}

/* Function with sequential regions and data persistence */
void test_persistent_data(int argc) {
    static float persistent[N][M];
    
    /* Initialize on host */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            persistent[i][j] = (float)(i * j);
        }
    }
    
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(persistent[0:N][0:M]) gang
    
    /* Multiple compute regions with same persistent data */
    #pragma acc parallel present(persistent) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                persistent[i][j] += 10.0f;
            }
        }
    }
    
    /* Different partition type on same data */
    #pragma acc kernels present(persistent) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                persistent[i][j] *= 1.1f;
            }
        }
    }
    
    /* Exit data */
    #pragma acc exit data copyout(persistent[0:N][0:M])
    
    /* Verify on host */
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += persistent[i][j];
        }
    }
    printf("Persistent data sum: %f\n", sum);
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition mapping coverage\n");
    
    /* Execute all test functions with conditional paths */
    test_nested_partitions(argc);
    test_vector_partitions(argc);
    test_fully_partitioned(argc);
    test_persistent_data(argc);
    
    /* Additional mixed partitioning test */
    float mixed[N][M];
    #pragma acc parallel loop collapse(2) gang, vector copy(mixed[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            mixed[i][j] = (float)(i + j * 2);
        }
    }
    
    /* Worker-only partition */
    #pragma acc kernels copy(mixed[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                mixed[i][j] += 3.14f;
            }
        }
    }
    
    printf("All OpenACC partition tests completed\n");
    return 0;
}
