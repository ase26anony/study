/* test_omp_acc_partitions.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_omp_acc_partitions.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 16
#define P 8

/* Routine with explicit partition specification */
#pragma acc routine vec gang
void increment_element(int *arr, int i, int j, int k) {
    arr[i * M * P + j * P + k] += 1;
}

/* Function with nested compute regions */
void test_nested_partitions(int argc) {
    int arr1[N][M][P];
    int arr2[N][M][P];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr1[i][j][k] = i + j + k;
                arr2[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Test 1: Gang redundant partitioning */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr1[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* Test 2: Gang partitioned */
    if (argc > 2) {
        #pragma acc kernels create(arr2[0:N][0:M][0:P]) gang(static:2)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr2[i][j][k] += arr1[i][j][k];
                    }
                }
            }
        }
    }
    
    /* Test 3: Worker partitioned */
    if (argc > 3) {
        int arr3[N][M];
        #pragma acc enter data create(arr3[0:N][0:M]) worker
        
        #pragma acc parallel present(arr3) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr3[i][j] = i * 100 + j;
                }
            }
        }
        
        #pragma acc exit data copyout(arr3[0:N][0:M]) worker
    }
}

/* Function with vector partitioning variations */
void test_vector_partitions(int argc) {
    int arr4[N][M][P];
    int arr5[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr4[i][j][k] = 1;
                arr5[i][j][k] = 2;
            }
        }
    }
    
    /* Test 4: Vector partitioned */
    if (argc > 4) {
        #pragma acc parallel copy(arr4[0:N][0:M][0:P]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr4[i][j][k] <<= 1;
                    }
                }
            }
        }
    }
    
    /* Test 5: Gang+vector partitioned */
    if (argc > 5) {
        #pragma acc kernels copy(arr5[0:N][0:M][0:P]) gang, vector
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                #pragma acc loop gang vector
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr5[i][j][k] = arr4[i][j][k] + arr5[i][j][k];
                    }
                }
            }
        }
    }
    
    /* Test 6: Worker+vector partitioned */
    if (argc > 6) {
        int arr6[N][M];
        #pragma acc enter data create(arr6[0:N][0:M]) worker, vector
        
        #pragma acc parallel present(arr6) worker, vector
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr6[i][j] = (i + j) % 256;
                }
            }
        }
        
        #pragma acc exit data copyout(arr6[0:N][0:M]) worker, vector
    }
}

/* Function with complex combined partitions */
void test_combined_partitions(int argc) {
    int arr7[N][M][P];
    int arr8[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr7[i][j][k] = i * M * P + j * P + k;
                arr8[i][j][k] = 0;
            }
        }
    }
    
    /* Test 7: Gang+worker partitioned */
    if (argc > 7) {
        #pragma acc parallel copy(arr7[0:N][0:M][0:P]) gang, worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        increment_element(&arr7[0][0][0], i, j, k);
                    }
                }
            }
        }
    }
    
    /* Test 8: Fully partitioned (gang+worker+vector) */
    if (argc > 8) {
        #pragma acc kernels copyin(arr7[0:N][0:M][0:P]) copyout(arr8[0:N][0:M][0:P]) \
                gang, worker, vector
        {
            #pragma acc loop gang worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr8[i][j][k] = arr7[i][j][k] * 3;
                    }
                }
            }
        }
    }
    
    /* Test 9: Sequential regions with different partitions */
    if (argc > 9) {
        int temp[N];
        
        /* First region: gang partitioned */
        #pragma acc parallel copy(temp[0:N]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                temp[i] = i * 10;
            }
        }
        
        /* Second region: vector partitioned */
        #pragma acc parallel copy(temp[0:N]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                temp[i] += 5;
            }
        }
        
        /* Third region: worker partitioned */
        #pragma acc parallel copy(temp[0:N]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                temp[i] *= 2;
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Each test function uses argc for conditional execution
     * to prevent dead code elimination */
    test_nested_partitions(argc);
    test_vector_partitions(argc);
    test_combined_partitions(argc);
    
    /* Additional test with if clause on directive */
    int arr9[10][20][30];
    
    #pragma acc parallel if(argc > 10) copy(arr9[0:10][0:20][0:30]) gang, worker, vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < 30; k++) {
                    arr9[i][j][k] = i + j * 2 + k * 3;
                }
            }
        }
    }
    
    /* Verify some results to ensure computation happens */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                sum += arr9[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", sum);
    printf("Test completed. Compile with OpenACC to trigger partition mapping.\n");
    
    return 0;
}
