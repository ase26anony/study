/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass (lines 335-343 of
 * omp-oacc-neuter-broadcast.cc).
 *
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test.c -o test.o
 * Or for more aggressive analysis: gcc -O3 -fopenacc -foffload=disable -c test.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Routine with explicit partition type */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

/* Routine with vector partition */
#pragma acc routine seq
void acc_routine_vector(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

/* Test 1: Basic partition combinations on multi-dimensional array */
void test_basic_partitions(int argc) {
    int arr3d[10][20][30];
    int i, j, k;
    
    /* Initialize */
    for (i = 0; i < 10; i++)
        for (j = 0; j < 20; j++)
            for (k = 0; k < 30; k++)
                arr3d[i][j][k] = i + j + k;
    
    /* Case 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) gang
        {
            #pragma acc loop gang
            for (i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (j = 0; j < 20; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 30; k++) {
                        arr3d[i][j][k] += 1;
                    }
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels create(arr3d[0:5][0:20][0:30]) gang(static:2)
        {
            #pragma acc loop gang
            for (i = 0; i < 5; i++) {
                for (j = 0; j < 20; j++) {
                    for (k = 0; k < 30; k++) {
                        arr3d[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 3) {
        #pragma acc parallel copy(arr3d[5:5][0:20][0:30]) worker
        {
            #pragma acc loop worker
            for (i = 5; i < 10; i++) {
                for (j = 0; j < 20; j++) {
                    for (k = 0; k < 30; k++) {
                        arr3d[i][j][k] -= 1;
                    }
                }
            }
        }
    }
}

/* Test 2: Combined partition types */
void test_combined_partitions(int argc) {
    int arr2d[N][M];
    int i, j;
    
    /* Initialize */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            arr2d[i][j] = i * j;
    
    /* Case 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc parallel copy(arr2d[0:N][0:M]) gang worker
        {
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    arr2d[i][j] += i;
                }
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 5) {
        #pragma acc kernels copy(arr2d[0:N][0:M/2]) vector
        {
            #pragma acc loop vector
            for (i = 0; i < N; i++) {
                for (j = 0; j < M/2; j++) {
                    arr2d[i][j] *= 3;
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (argc > 6) {
        #pragma acc parallel copy(arr2d[0:N][M/2:M/2]) gang vector
        {
            #pragma acc loop gang vector
            for (i = 0; i < N; i++) {
                for (j = M/2; j < M; j++) {
                    arr2d[i][j] /= 2;
                }
            }
        }
    }
}

/* Test 3: Nested regions and routine calls */
void test_nested_and_routines(int argc) {
    int arr1d[P];
    int i;
    
    /* Initialize */
    for (i = 0; i < P; i++)
        arr1d[i] = i;
    
    /* Case 6: worker+vector partitioned with routine call */
    if (argc > 7) {
        #pragma acc parallel copy(arr1d[0:P]) worker vector
        {
            #pragma acc loop worker vector
            for (i = 0; i < P; i++) {
                acc_routine_gang(&arr1d[i], 1);
            }
        }
    }
    
    /* Nested conditional region */
    if (argc > 8) {
        int condition = 1;
        #pragma acc parallel if(condition) copy(arr1d[0:P]) gang
        {
            #pragma acc loop gang
            for (i = 0; i < P; i++) {
                if (arr1d[i] > 10) {
                    #pragma acc kernels worker
                    {
                        #pragma acc loop worker
                        for (int j = 0; j < 2; j++) {
                            arr1d[i] += j;
                        }
                    }
                }
            }
        }
    }
}

/* Test 4: Persistent device data with partition clauses */
void test_persistent_data(int argc) {
    int persistent_arr[N][M];
    int i, j;
    
    /* Initialize */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            persistent_arr[i][j] = i - j;
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (argc > 9) {
        /* Establish device data region with partition */
        #pragma acc enter data copyin(persistent_arr[0:N][0:M]) gang worker vector
        
        /* Multiple compute regions accessing partitioned data */
        #pragma acc parallel present(persistent_arr) gang
        {
            #pragma acc loop gang
            for (i = 0; i < N; i += 2) {
                #pragma acc loop worker vector
                for (j = 0; j < M; j++) {
                    persistent_arr[i][j] += 100;
                }
            }
        }
        
        #pragma acc kernels present(persistent_arr) worker
        {
            #pragma acc loop worker
            for (i = 1; i < N; i += 2) {
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    persistent_arr[i][j] -= 50;
                }
            }
        }
        
        /* Exit data region */
        #pragma acc exit data copyout(persistent_arr[0:N][0:M])
    }
}

/* Test 5: Complex collapse with partition clauses */
void test_collapse_partitions(int argc) {
    int arr3d[8][16][32];
    int i, j, k;
    
    /* Initialize */
    for (i = 0; i < 8; i++)
        for (j = 0; j < 16; j++)
            for (k = 0; k < 32; k++)
                arr3d[i][j][k] = i * j * k;
    
    /* Various collapse combinations with partitions */
    if (argc > 10) {
        /* Collapse 2 with gang+worker partition */
        #pragma acc parallel loop collapse(2) gang worker copy(arr3d[0:8][0:16][0:32])
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 16; j++) {
                for (k = 0; k < 32; k++) {
                    arr3d[i][j][k] += 5;
                }
            }
        }
    }
    
    if (argc > 11) {
        /* Collapse 3 with vector partition */
        #pragma acc kernels loop collapse(3) vector copy(arr3d[0:4][0:8][0:16])
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 8; j++) {
                for (k = 0; k < 16; k++) {
                    arr3d[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    printf("Testing OpenACC partition type coverage...\n");
    
    /* Each test is conditionally executed based on argc
     * to prevent dead code elimination */
    test_basic_partitions(argc);
    test_combined_partitions(argc);
    test_nested_and_routines(argc);
    test_persistent_data(argc);
    test_collapse_partitions(argc);
    
    printf("Tests completed (compile-time coverage target reached).\n");
    return 0;
}
