/* test_openacc_partitions.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_openacc_partitions.c -o test.o
 * Or for debugging: gcc -O1 -fopenacc -fdump-tree-all -c test_openacc_partitions.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Routine with explicit partition specification */
#pragma acc routine seq
void init_array(int *arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value;
    }
}

/* Gang-partitioned routine */
#pragma acc routine gang
void gang_operation(int *arr, int size, int increment) {
    #pragma acc loop gang
    for (int i = 0; i < size; i++) {
        arr[i] += increment;
    }
}

/* Vector-partitioned routine */
#pragma acc routine vector
void vector_operation(int *arr, int size, int multiplier) {
    #pragma acc loop vector
    for (int i = 0; i < size; i++) {
        arr[i] *= multiplier;
    }
}

/* Test function 1: Various partition combinations on multi-dimensional arrays */
void test_multi_dimensional_partitions(int argc) {
    int arr3d[N][M][P];
    int arr2d[N][M];
    
    /* Initialize arrays */
    #pragma acc parallel loop collapse(3) gang, worker, vector copy(arr3d[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Test 1: Gang redundant partitioning */
    if (argc > 1) {
        #pragma acc parallel copy(arr2d[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop gang
                for (int j = 0; j < M; j++) {
                    arr2d[i][j] = i * j;
                }
            }
        }
    }
    
    /* Test 2: Gang partitioned */
    #pragma acc kernels create(arr2d[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang
            for (int j = 0; j < M; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
    
    /* Test 3: Worker partitioned */
    #pragma acc parallel loop collapse(2) worker copy(arr2d[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] *= 2;
        }
    }
    
    /* Test 4: Gang+worker partitioned */
    #pragma acc parallel gang, worker copy(arr3d[0:N][0:M][0:P])
    {
        #pragma acc loop gang, worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang, worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop gang, worker
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] = arr3d[i][j][k] % 100;
                }
            }
        }
    }
    
    /* Test 5: Vector partitioned */
    #pragma acc kernels loop vector copy(arr2d[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = arr2d[i][j] << 1;
        }
    }
    
    /* Test 6: Gang+vector partitioned */
    #pragma acc parallel loop gang, vector copy(arr3d[0:10][0:20][0:30])
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                arr3d[i][j][k] += i * j * k;
            }
        }
    }
    
    /* Test 7: Worker+vector partitioned */
    #pragma acc parallel worker, vector copy(arr2d[0:N][0:M])
    {
        #pragma acc loop worker, vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker, vector
            for (int j = 0; j < M; j++) {
                arr2d[i][j] -= 5;
            }
        }
    }
    
    /* Test 8: Fully partitioned (gang+worker+vector) */
    #pragma acc parallel gang, worker, vector copy(arr3d[0:N][0:M][0:P])
    {
        #pragma acc loop gang, worker, vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang, worker, vector
            for (int j = 0; j < M; j++) {
                #pragma acc loop gang, worker, vector
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] = arr3d[i][j][k] / 2;
                }
            }
        }
    }
}

/* Test function 2: Nested and sequential compute regions */
void test_nested_regions(int *data, int size, int argc) {
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(data[0:size]) gang
    
    /* Conditional region based on argc */
    #pragma acc parallel if(argc > 2) present(data) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < size; i++) {
            data[i] = i * 2;
        }
        
        /* Nested-like region using routine */
        gang_operation(data, size, 10);
    }
    
    /* Sequential region with different partition */
    #pragma acc kernels present(data) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < size; i++) {
            data[i] += 5;
        }
    }
    
    /* Another region with vector partitioning */
    #pragma acc parallel loop vector present(data)
    for (int i = 0; i < size; i++) {
        data[i] *= 3;
    }
    
    /* Exit data */
    #pragma acc exit data copyout(data[0:size]) gang
}

/* Test function 3: Complex routine interactions */
void test_routine_interactions(int *arr1, int *arr2, int size) {
    /* Mixed partitioning with routine calls */
    #pragma acc parallel gang, vector copy(arr1[0:size], arr2[0:size])
    {
        /* Call gang-partitioned routine */
        gang_operation(arr1, size, 2);
        
        /* Inline vector operations */
        #pragma acc loop vector
        for (int i = 0; i < size; i++) {
            arr2[i] = arr1[i] * 3;
        }
        
        /* Call vector-partitioned routine */
        vector_operation(arr2, size, 2);
    }
    
    /* Follow with worker-partitioned region */
    #pragma acc kernels worker copy(arr1[0:size])
    {
        #pragma acc loop worker
        for (int i = 0; i < size; i++) {
            arr1[i] = arr1[i] % 100;
        }
    }
}

/* Test function 4: Multi-level data environments */
void test_persistent_data(int argc) {
    int persistent_data[N * M];
    
    /* Establish persistent device data with partitioning */
    #pragma acc enter data create(persistent_data[0:N*M]) gang, vector
    
    /* Multiple compute regions accessing persistent data */
    for (int iter = 0; iter < 3; iter++) {
        if (iter % 2 == 0) {
            #pragma acc parallel present(persistent_data) gang
            {
                #pragma acc loop gang
                for (int i = 0; i < N*M; i++) {
                    persistent_data[i] += iter;
                }
            }
        } else {
            #pragma acc parallel present(persistent_data) worker, vector
            {
                #pragma acc loop worker, vector
                for (int i = 0; i < N*M; i++) {
                    persistent_data[i] *= 2;
                }
            }
        }
    }
    
    /* Final region with full partitioning */
    #pragma acc parallel present(persistent_data) gang, worker, vector
    {
        #pragma acc loop gang, worker, vector
        for (int i = 0; i < N*M; i++) {
            persistent_data[i] = persistent_data[i] % 256;
        }
    }
    
    #pragma acc exit data copyout(persistent_data[0:N*M]) gang, vector
}

int main(int argc, char **argv) {
    int test_data1[N * M];
    int test_data2[N * M];
    
    /* Initialize test data */
    init_array(test_data1, N * M, 1);
    init_array(test_data2, N * M, 2);
    
    /* Test 1: Multi-dimensional arrays with various partitions */
    test_multi_dimensional_partitions(argc);
    
    /* Test 2: Nested and sequential regions */
    test_nested_regions(test_data1, N * M, argc);
    
    /* Test 3: Routine interactions */
    test_routine_interactions(test_data1, test_data2, N * M);
    
    /* Test 4: Persistent data environments */
    test_persistent_data(argc);
    
    /* Verification (simple check to prevent optimization) */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) copyin(test_data1[0:N*M]) gang
    for (int i = 0; i < N * M; i++) {
        sum += test_data1[i];
    }
    
    printf("Final sum: %d\n", sum);
    printf("Partition testing complete.\n");
    
    return 0;
}
