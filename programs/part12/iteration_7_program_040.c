/* test_partition_coverage.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_partition_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Test function with routine directive */
#pragma acc routine vec gang
void increment_element(int *arr, int idx, int value) {
    arr[idx] += value;
}

/* Test function with worker partitioning */
#pragma acc routine worker
void scale_element(float *arr, int idx, float factor) {
    arr[idx] *= factor;
}

/* Test 1: Basic partition combinations */
void test_basic_partitions() {
    int arr1[N][M];
    float arr2[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = (float)(i * j);
        }
    }
    
    /* Case 0: gang redundant */
    #pragma acc parallel copy(arr1[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr1[i][j] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc kernels create(arr2[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] += 2.0f;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] *= 2;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc kernels copy(arr2[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] /= 2.0f;
            }
        }
    }
}

/* Test 2: Vector partition combinations */
void test_vector_partitions(int use_vector) {
    int arr3[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (use_vector > 0) {
        #pragma acc parallel copy(arr3[0:N][0:M][0:P]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] += 3;
                    }
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc kernels copy(arr3[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr3[i][j][k] -= 1;
                }
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel copy(arr3[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 3: Fully partitioned and nested regions */
void test_fully_partitioned(int condition) {
    double arr4[N][M];
    double arr5[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr4[i][j] = (double)(i + j);
            arr5[i][j] = (double)(i * j);
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr4[0:N][0:M]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr4[i][j] = arr4[i][j] * 1.5;
            }
        }
    }
    
    /* Nested conditional region */
    if (condition) {
        /* Enter data with partition clause */
        #pragma acc enter data copyin(arr5[0:N][0:M]) gang
        
        /* Present data with different partition */
        #pragma acc parallel present(arr5[0:N][0:M]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr5[i][j] += 10.0;
                }
            }
        }
        
        /* Exit data */
        #pragma acc exit data copyout(arr5[0:N][0:M])
    }
    
    /* Mixed partition in same function */
    #pragma acc parallel if(condition > 1) copy(arr4[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Inner parallel region with different partition */
            #pragma acc parallel vector private(j)
            {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    increment_element(&arr4[i][0], j, 5);
                }
            }
        }
    }
}

/* Test 4: Routine directive with partition propagation */
void test_routine_partitions() {
    int arr6[N*M];
    float arr7[N*M];
    
    /* Initialize linear arrays */
    for (int i = 0; i < N*M; i++) {
        arr6[i] = i % 100;
        arr7[i] = (float)(i % 50);
    }
    
    /* Use gang-partitioned routine */
    #pragma acc parallel copy(arr6[0:N*M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                increment_element(arr6, idx, 1);
            }
        }
    }
    
    /* Use worker-partitioned routine */
    #pragma acc parallel copy(arr7[0:N*M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N*M; i += 2) {
            scale_element(arr7, i, 2.0f);
        }
    }
}

/* Test 5: Complex multi-dimensional partitioning */
void test_complex_partitioning(int argc) {
    int arr8[N][M][P];
    int arr9[N][M][P];
    
    /* Initialize with different patterns based on argc */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr8[i][j][k] = (argc > 1) ? (i + j + k) : (i * j * k);
                arr9[i][j][k] = (argc > 2) ? (i - j - k) : (i + j - k);
            }
        }
    }
    
    /* Multiple consecutive regions with different partitions */
    
    /* Region 1: gang partitioned on first dimension */
    #pragma acc parallel copy(arr8[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr8[i][j][k] += i;
                }
            }
        }
    }
    
    /* Region 2: worker partitioned on second dimension */
    #pragma acc kernels copy(arr8[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            for (int i = 0; i < N; i++) {
                for (int k = 0; k < P; k++) {
                    arr8[i][j][k] += j;
                }
            }
        }
    }
    
    /* Region 3: vector partitioned on third dimension */
    #pragma acc parallel copy(arr8[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int k = 0; k < P; k++) {
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr8[i][j][k] += k;
                }
            }
        }
    }
    
    /* Region 4: mixed partitioning with collapse */
    #pragma acc kernels copy(arr9[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr9[i][j][k] = arr8[i][j][k] + arr9[i][j][k];
                }
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    printf("Starting partition coverage test...\n");
    
    /* Execute all test cases with different conditions */
    test_basic_partitions();
    
    /* Use argc to create conditional paths */
    test_vector_partitions(argc > 0);
    test_fully_partitioned(argc);
    test_routine_partitions();
    test_complex_partitioning(argc);
    
    /* Final validation region with all partition types */
    int final_arr[10][20][30];
    
    #pragma acc parallel copy(final_arr[0:10][0:20][0:30]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < 30; k++) {
                    final_arr[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    printf("Partition coverage test completed.\n");
    
    /* Simple validation */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                sum += final_arr[i][j][k];
            }
        }
    }
    printf("Validation sum: %d\n", sum);
    
    return 0;
}
