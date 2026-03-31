/* test_openacc_partitions.c - Comprehensive test for OpenACC partition type coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 64
#define M 32
#define P 16

/* Function prototypes */
void test_gang_redundant(int arr[N][M][P]);
void test_gang_partitioned(int arr[N][M][P]);
void test_worker_partitioned(int arr[N][M][P]);
void test_gang_worker_partitioned(int arr[N][M][P]);
void test_vector_partitioned(int arr[N][M][P]);
void test_gang_vector_partitioned(int arr[N][M][P]);
void test_worker_vector_partitioned(int arr[N][M][P]);
void test_fully_partitioned(int arr[N][M][P]);
void test_nested_regions(int arr[N][M][P]);
void test_persistent_data(int arr[N][M][P]);

/* Routine with explicit partition specification */
#pragma acc routine seq
int compute_element(int x, int y, int z);

#pragma acc routine gang
void gang_routine(int *arr, int size);

#pragma acc routine worker
void worker_routine(int *arr, int size);

#pragma acc routine vector
void vector_routine(int *arr, int size);

/* Main test function */
int main(int argc, char *argv[]) {
    /* Multi-dimensional arrays for testing */
    int array1[N][M][P];
    int array2[N][M][P];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                array1[i][j][k] = i * M * P + j * P + k;
                array2[i][j][k] = 0;
            }
        }
    }
    
    /* Use argc to create conditional execution paths */
    if (argc > 1) {
        /* Test 0: Gang redundant (default) */
        test_gang_redundant(array1);
        
        /* Test 1: Gang partitioned */
        test_gang_partitioned(array2);
        
        /* Test 2: Worker partitioned */
        test_worker_partitioned(array1);
    }
    
    if (argc > 2) {
        /* Test 3: Gang+worker partitioned */
        test_gang_worker_partitioned(array2);
        
        /* Test 4: Vector partitioned */
        test_vector_partitioned(array1);
    }
    
    if (argc > 3) {
        /* Test 5: Gang+vector partitioned */
        test_gang_vector_partitioned(array2);
        
        /* Test 6: Worker+vector partitioned */
        test_worker_vector_partitioned(array1);
    }
    
    if (argc > 4) {
        /* Test 7: Fully partitioned */
        test_fully_partitioned(array2);
        
        /* Additional complex tests */
        test_nested_regions(array1);
        test_persistent_data(array2);
    }
    
    /* Verify results */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                if (array1[i][j][k] != i * M * P + j * P + k + 1) {
                    errors++;
                }
            }
        }
    }
    
    printf("Test completed with %d errors\n", errors);
    return errors > 0 ? 1 : 0;
}

/* Test 0: Gang redundant */
void test_gang_redundant(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 1: Gang partitioned */
void test_gang_partitioned(int arr[N][M][P]) {
    #pragma acc kernels create(arr[0:N][0:M][0:P]) gang(static:2)
    {
        #pragma acc loop gang(static:2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = compute_element(i, j, k);
                }
            }
        }
    }
}

/* Test 2: Worker partitioned */
void test_worker_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 3: Gang+worker partitioned */
void test_gang_worker_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 4: Vector partitioned */
void test_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 5: Gang+vector partitioned */
void test_gang_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 6: Worker+vector partitioned */
void test_worker_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test with nested regions */
void test_nested_regions(int arr[N][M][P]) {
    int temp[N];
    
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) create(temp[0:N]) gang
    
    #pragma acc parallel present(arr, temp) gang if(N > 32)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            temp[i] = 0;
            
            #pragma acc parallel loop worker present(arr) if(i % 2 == 0)
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    temp[i] += arr[i][j][k];
                }
            }
            
            /* Call partitioned routines */
            if (i == 0) {
                gang_routine(&temp[i], 1);
            } else if (i == 1) {
                worker_routine(&temp[i], 1);
            } else {
                vector_routine(&temp[i], 1);
            }
        }
    }
    
    #pragma acc exit data copyout(temp[0:N]) delete(arr) gang
}

/* Test with persistent device data */
void test_persistent_data(int arr[N][M][P]) {
    /* Create persistent device data with partition */
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) gang worker
    
    /* Multiple compute regions using the same partitioned data */
    #pragma acc parallel present(arr) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] *= 2;
                }
            }
        }
    }
    
    #pragma acc kernels present(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 3;
                }
            }
        }
    }
    
    #pragma acc parallel present(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 1;
                }
            }
        }
    }
    
    #pragma acc exit data copyout(arr[0:N][0:M][0:P]) gang worker
}

/* Partitioned routine implementations */
int compute_element(int x, int y, int z) {
    return x * y * z + 1;
}

void gang_routine(int *arr, int size) {
    #pragma acc routine gang
    for (int i = 0; i < size; i++) {
        arr[i] += 100;
    }
}

void worker_routine(int *arr, int size) {
    #pragma acc routine worker
    for (int i = 0; i < size; i++) {
        arr[i] += 200;
    }
}

void vector_routine(int *arr, int size) {
    #pragma acc routine vector
    for (int i = 0; i < size; i++) {
        arr[i] += 300;
    }
}
