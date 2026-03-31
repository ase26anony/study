/* test_partition_coverage.c - Cover OpenACC partition mapping switch cases */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition specification */
#pragma acc routine seq
void init_array(int *arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value;
    }
}

/* Gang-partitioned routine */
#pragma acc routine gang
void gang_partitioned_op(int *arr, int size, int factor) {
    #pragma acc loop gang
    for (int i = 0; i < size; i++) {
        arr[i] *= factor;
    }
}

/* Worker-partitioned routine */
#pragma acc routine worker
void worker_partitioned_op(int *arr, int size, int increment) {
    #pragma acc loop worker
    for (int i = 0; i < size; i++) {
        arr[i] += increment;
    }
}

/* Vector-partitioned routine */
#pragma acc routine vector
void vector_partitioned_op(int *arr, int size, int divisor) {
    #pragma acc loop vector
    for (int i = 0; i < size; i++) {
        arr[i] /= divisor;
    }
}

/* Test 1: Gang redundant partitioning */
void test_gang_redundant(int argc) {
    int arr1d[N];
    int arr3d[N][M][P];
    
    /* Initialize arrays */
    init_array(arr1d, N, 1);
    
    /* Case 0: Gang redundant - entire array on each gang */
    #pragma acc parallel copy(arr1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] += i;
        }
    }
    
    /* Multi-dimensional with gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] = i + j + k;
                    }
                }
            }
        }
    }
}

/* Test 2: Gang partitioned */
void test_gang_partitioned(int argc) {
    int arr1d[N * M];
    int arr2d[N][M];
    
    /* Case 1: Gang partitioned across first dimension */
    #pragma acc parallel copy(arr1d[0:N*M]) gang(static:N)
    {
        #pragma acc loop gang
        for (int i = 0; i < N * M; i++) {
            arr1d[i] = i * 2;
        }
    }
    
    /* 2D array with gang partitioning */
    if (argc > 2) {
        #pragma acc kernels copy(arr2d[0:N][0:M]) gang(static:10)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    arr2d[i][j] = i * j;
                }
            }
        }
    }
}

/* Test 3: Worker partitioned */
void test_worker_partitioned(int argc) {
    int arr1d[N];
    int arr3d[10][20][30];
    
    /* Case 2: Worker partitioned */
    #pragma acc parallel copy(arr1d[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] * 3 + 1;
        }
    }
    
    /* Nested worker partitioning */
    if (argc > 3) {
        #pragma acc kernels create(arr3d[0:10][0:20][0:30]) worker
        {
            #pragma acc loop worker collapse(2)
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 20; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] = i * 100 + j * 10 + k;
                    }
                }
            }
        }
    }
}

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(int argc) {
    int arr2d[N][M];
    int arr1d[N * M];
    
    /* Case 3: Gang+worker partitioned */
    #pragma acc parallel copy(arr2d[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = (i + j) % 256;
            }
        }
    }
    
    /* Using routine with gang+worker context */
    if (argc > 4) {
        #pragma acc enter data copyin(arr1d[0:N*M]) gang worker
        
        #pragma acc parallel present(arr1d) gang worker
        {
            gang_partitioned_op(arr1d, N * M, 2);
            worker_partitioned_op(arr1d, N * M, 10);
        }
        
        #pragma acc exit data copyout(arr1d) gang worker
    }
}

/* Test 5: Vector partitioned */
void test_vector_partitioned(int argc) {
    int arr1d[N * M * P];
    
    /* Case 4: Vector partitioned */
    #pragma acc parallel copy(arr1d[0:N*M*P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N * M * P; i++) {
            arr1d[i] = i & 0xFF;
        }
    }
    
    /* Vector partitioned with routine */
    if (argc > 5) {
        #pragma acc kernels copy(arr1d[0:N*M*P]) vector
        {
            vector_partitioned_op(arr1d, N * M * P, 2);
        }
    }
}

/* Test 6: Gang+vector partitioned */
void test_gang_vector_partitioned(int argc) {
    int arr3d[10][20][30];
    
    /* Case 5: Gang+vector partitioned */
    #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) gang vector
    {
        #pragma acc loop gang vector collapse(3)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < 30; k++) {
                    arr3d[i][j][k] = i * 400 + j * 20 + k;
                }
            }
        }
    }
    
    /* Nested regions with different partitions */
    if (argc > 6) {
        #pragma acc parallel if(argc > 10) gang
        {
            int local_arr[100];
            #pragma acc loop gang
            for (int i = 0; i < 100; i++) {
                local_arr[i] = i;
            }
            
            #pragma acc parallel vector
            {
                #pragma acc loop vector
                for (int i = 0; i < 100; i++) {
                    local_arr[i] *= 2;
                }
            }
        }
    }
}

/* Test 7: Worker+vector partitioned */
void test_worker_vector_partitioned(int argc) {
    int arr2d[N][M];
    
    /* Case 6: Worker+vector partitioned */
    #pragma acc kernels copy(arr2d[0:N][0:M]) worker vector
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = (i << 8) | j;
            }
        }
    }
    
    /* Sequential compute regions with different partitions */
    if (argc > 7) {
        int temp[N];
        #pragma acc enter data create(temp[0:N]) worker
        
        #pragma acc parallel present(temp) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                temp[i] = i * i;
            }
        }
        
        #pragma acc parallel present(temp) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                temp[i] += 1;
            }
        }
        
        #pragma acc exit data copyout(temp) worker vector
    }
}

/* Test 8: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int argc) {
    int arr3d[8][16][32];
    
    /* Case 7: Fully partitioned */
    #pragma acc parallel copy(arr3d[0:8][0:16][0:32]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 16; j++) {
                for (int k = 0; k < 32; k++) {
                    arr3d[i][j][k] = i * 1024 + j * 32 + k;
                }
            }
        }
    }
    
    /* Complex nested partitioning */
    if (argc > 8) {
        int arr4d[4][8][16][32];
        #pragma acc enter data create(arr4d[0:4][0:8][0:16][0:32]) gang worker vector
        
        #pragma acc parallel present(arr4d) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < 4; i++) {
                #pragma acc parallel worker vector
                {
                    #pragma acc loop worker vector collapse(3)
                    for (int j = 0; j < 8; j++) {
                        for (int k = 0; k < 16; k++) {
                            for (int l = 0; l < 32; l++) {
                                arr4d[i][j][k][l] = (i << 24) | (j << 16) | (k << 8) | l;
                            }
                        }
                    }
                }
            }
        }
        
        #pragma acc exit data copyout(arr4d) gang worker vector
    }
}

/* Test illegal/default case through edge conditions */
void test_edge_cases(int argc) {
    /* This may trigger default case through compiler internals */
    int small_arr[1];
    
    /* Minimal compute region */
    #pragma acc parallel copy(small_arr[0:1])
    {
        small_arr[0] = 42;
    }
    
    /* Empty data clause */
    if (argc > 9) {
        int another_arr[2];
        #pragma acc kernels
        {
            another_arr[0] = 1;
            another_arr[1] = 2;
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition coverage...\n");
    
    /* Execute all test cases with conditional paths based on argc */
    test_gang_redundant(argc);
    test_gang_partitioned(argc);
    test_worker_partitioned(argc);
    test_gang_worker_partitioned(argc);
    test_vector_partitioned(argc);
    test_gang_vector_partitioned(argc);
    test_worker_vector_partitioned(argc);
    test_fully_partitioned(argc);
    test_edge_cases(argc);
    
    /* Final validation */
    int final_check = 0;
    int test_arr[10];
    
    #pragma acc parallel copy(test_arr[0:10]) gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 10; i++) {
            test_arr[i] = i * argc;
        }
    }
    
    for (int i = 0; i < 10; i++) {
        final_check += test_arr[i];
    }
    
    printf("Final check sum: %d\n", final_check);
    printf("Partition coverage test completed.\n");
    
    return final_check == (45 * argc) ? 0 : 1;
}
