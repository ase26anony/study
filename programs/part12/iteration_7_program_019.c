/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass, specifically covering
 * the switch statement cases for all partition types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 32
#define M 16
#define P 8

/* Global arrays to test various partition mappings */
int array_3d[N][M][P];
int array_2d[N][M];
int array_1d[N];

/* Function prototypes for routine directives */
#pragma acc routine vec
void increment_vector(int *arr, int idx);
#pragma acc routine gang
void process_gang(int arr[][M], int i);
#pragma acc routine worker
void process_worker(int arr[][M][P], int i, int j);

/* Vector partitioned routine */
#pragma acc routine vec
void increment_vector(int *arr, int idx) {
    arr[idx] += 1;
}

/* Gang partitioned routine */
#pragma acc routine gang
void process_gang(int arr[][M], int i) {
    for (int j = 0; j < M; j++) {
        arr[i][j] *= 2;
    }
}

/* Worker partitioned routine */
#pragma acc routine worker
void process_worker(int arr[][M][P], int i, int j) {
    for (int k = 0; k < P; k++) {
        arr[i][j][k] += k;
    }
}

/* Test 1: Basic partition combinations on 3D array */
void test_basic_partitions(int argc) {
    /* Case 0: gang redundant (implicit) */
    #pragma acc parallel copy(array_3d[0:N][0:M][0:P]) if(argc > 1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(array_3d[0:N][0:M][0:P]) gang if(argc > 2)
    {
        #pragma acc loop gang(static:2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    #pragma acc kernels copy(array_2d[0:N][0:M]) worker if(argc > 3)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                array_2d[i][j] = i * j;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(array_3d[0:N][0:M][0:P]) gang worker if(argc > 4)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 2: Vector and combined partitions with collapse */
void test_vector_partitions(int argc) {
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(array_1d[0:N]) vector if(argc > 5)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            array_1d[i] = i * 2;
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc kernels copy(array_2d[0:N][0:M]) gang vector if(argc > 6)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                array_2d[i][j] += array_1d[i];
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel copy(array_3d[0:N][0:M][0:P]) worker vector if(argc > 7)
    {
        #pragma acc loop worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] -= k;
                }
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc kernels create(array_3d[0:N][0:M][0:P]) gang worker vector if(argc > 8)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] /= 2;
                }
            }
        }
    }
}

/* Test 3: Nested regions and routine calls */
void test_nested_routines(int argc) {
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(array_2d[0:N][0:M]) gang if(argc > 9)
    
    /* Nested region with worker partitioning */
    #pragma acc parallel present(array_2d) worker if(argc > 10)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Call gang-partitioned routine */
            process_gang(array_2d, i);
            
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                /* Call worker-partitioned routine from within worker loop */
                process_worker(array_3d, i, j);
                
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    /* Call vector-partitioned routine */
                    increment_vector(&array_3d[i][j][k], k);
                }
            }
        }
    }
    
    /* Exit data with vector partitioning */
    #pragma acc exit data copyout(array_2d[0:N][0:M]) vector if(argc > 11)
}

/* Test 4: Sequential regions with different partition types */
void test_sequential_regions(int argc) {
    int temp[N][M];
    
    /* First region: gang partitioned */
    #pragma acc parallel copyin(array_2d[0:N][0:M]) copyout(temp[0:N][0:M]) gang if(argc > 12)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                temp[i][j] = array_2d[i][j];
            }
        }
    }
    
    /* Second region: worker partitioned (nested conceptually) */
    #pragma acc kernels copy(temp[0:N][0:M]) worker if(argc > 13)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                temp[i][j] += 10;
            }
        }
    }
    
    /* Third region: vector partitioned */
    #pragma acc parallel copy(temp[0:N][0:M]) vector if(argc > 14)
    {
        #pragma acc loop vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                array_2d[i][j] = temp[i][j];
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array_1d[i] = 0;
        for (int j = 0; j < M; j++) {
            array_2d[i][j] = 0;
            for (int k = 0; k < P; k++) {
                array_3d[i][j][k] = 0;
            }
        }
    }
    
    /* Execute tests based on argc to prevent dead code elimination */
    test_basic_partitions(argc);
    test_vector_partitions(argc);
    test_nested_routines(argc);
    test_sequential_regions(argc);
    
    /* Simple validation */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += array_1d[i];
        for (int j = 0; j < M; j++) {
            sum += array_2d[i][j];
            for (int k = 0; k < P; k++) {
                sum += array_3d[i][j][k];
            }
        }
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
