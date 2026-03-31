/* test_openacc_partitions.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition */
#pragma acc routine vec gang
void increment_element(int *arr, int idx, int val) {
    arr[idx] += val;
}

/* Function with nested compute regions */
void test_nested_partitions(int condition) {
    int arr3d[10][20][30];
    
    /* Initialize */
    #pragma acc parallel loop collapse(3) gang, vector copy(arr3d[0:10][0:20][0:30])
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            for (int k = 0; k < 30; k++)
                arr3d[i][j][k] = i + j + k;
    
    /* Conditional offloading with gang partitioned */
    #pragma acc parallel if(condition) gang copy(arr3d[0:10][0:20][0:30])
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            /* Nested worker partitioned region */
            #pragma acc kernels worker
            {
                #pragma acc loop worker
                for (int j = 0; j < 20; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] *= 2;
                    }
                }
            }
        }
    }
}

/* Test function for gang redundant partitioning */
void test_gang_redundant() {
    int arr[N];
    
    #pragma acc parallel copy(arr[0:N]) gang
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2;
    }
    
    /* Validate on host */
    #pragma acc update self(arr[0:N])
    for (int i = 0; i < N; i++) {
        if (arr[i] != i * 2) {
            printf("Error in gang redundant test\n");
            break;
        }
    }
}

/* Test function for gang partitioned */
void test_gang_partitioned() {
    int arr[N][M];
    
    #pragma acc parallel loop collapse(2) gang copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
}

/* Test function for worker partitioned */
void test_worker_partitioned() {
    int arr[N];
    
    #pragma acc kernels create(arr[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr[i] = i * 3;
        }
    }
}

/* Test function for gang+worker partitioned */
void test_gang_worker_partitioned() {
    int arr[N][M];
    
    #pragma acc parallel loop gang worker copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * j;
        }
    }
}

/* Test function for vector partitioned */
void test_vector_partitioned() {
    int arr[N];
    
    #pragma acc parallel loop vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 4;
    }
}

/* Test function for gang+vector partitioned */
void test_gang_vector_partitioned() {
    int arr[N][M][P];
    
    #pragma acc parallel loop collapse(3) gang vector copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Use routine directive inside */
    #pragma acc parallel loop gang vector copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                increment_element(&arr[i][j][k], 0, 1);
            }
        }
    }
}

/* Test function for worker+vector partitioned */
void test_worker_vector_partitioned() {
    int arr[N][M];
    
    #pragma acc kernels loop worker vector copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        #pragma acc loop vector
        for (int j = 0; j < M; j++) {
            arr[i][j] = i - j;
        }
    }
}

/* Test function for fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    int arr[N][M][P];
    
    #pragma acc parallel loop collapse(3) gang worker vector copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * j * k;
            }
        }
    }
}

/* Test with device data environment and persistent partitions */
void test_persistent_partitions() {
    int persistent_arr[N][M];
    
    /* Establish device data region with gang partitioning */
    #pragma acc enter data copyin(persistent_arr[0:N][0:M]) gang
    
    /* Multiple compute regions accessing partitioned data */
    #pragma acc parallel present(persistent_arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                persistent_arr[i][j] = i + j;
            }
        }
    }
    
    #pragma acc parallel present(persistent_arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                persistent_arr[i][j] *= 2;
            }
        }
    }
    
    /* Clean up */
    #pragma acc exit data copyout(persistent_arr[0:N][0:M])
}

/* Main test driver */
int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Use argc to create conditional execution paths */
    int test_condition = (argc > 1);
    
    /* Test all partition combinations */
    test_gang_redundant();
    
    if (test_condition) {
        test_gang_partitioned();
        test_worker_partitioned();
    } else {
        test_gang_worker_partitioned();
        test_vector_partitioned();
    }
    
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Test nested regions */
    test_nested_partitions(test_condition);
    
    /* Test persistent data regions */
    test_persistent_partitions();
    
    /* Additional complex case with mixed directives */
    int mixed_arr[N][M];
    #pragma acc data copy(mixed_arr[0:N][0:M]) gang
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                mixed_arr[i][j] = (i + j) % 256;
            }
        }
        
        #pragma acc update self(mixed_arr[0:N][0:M])
    }
    
    printf("All partition tests completed (compile-time coverage target).\n");
    
    /* Simple runtime validation */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) gang vector copy(mixed_arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += mixed_arr[i][j];
        }
    }
    
    printf("Final checksum: %d\n", sum);
    
    return 0;
}
