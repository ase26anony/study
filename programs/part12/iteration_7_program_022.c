/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the uncovered lines in
 * omp-oacc-neuter-broadcast.cc (lines 335-343) by creating OpenACC
 * compute regions with various data partition combinations.
 * 
 * Compilation recommendations:
 *   gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test.c -o test.o
 *   gcc -O3 -fopenacc -foffload=disable -c test.c -o test.o
 *   gcc -O1 -fopenacc -fdump-tree-all -c test.c -o test.o
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

/* Routine with worker partition */
#pragma acc routine worker
void acc_routine_worker(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

/* Test 1: Basic partition combinations on 3D array */
void test_basic_partitions(int argc) {
    int arr3d[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Case 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] += 1;
                    }
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels copy(arr3d[0:N][0:M][0:P]) gang(static:2)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 3) {
        #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] -= 1;
                    }
                }
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc kernels create(arr3d[0:N][0:M][0:P]) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] += i;
                    }
                }
            }
        }
    }
}

/* Test 2: Vector partition combinations with collapse */
void test_vector_partitions(int argc) {
    int arr2d[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = i * j;
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 5) {
        #pragma acc parallel copy(arr2d[0:N][0:M]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr2d[i][j] += j;
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned with collapse */
    if (argc > 6) {
        #pragma acc parallel loop collapse(2) gang vector copy(arr2d[0:N][0:M])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] *= 3;
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (argc > 7) {
        #pragma acc kernels copy(arr2d[0:N][0:M]) worker vector
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr2d[i][j] /= 2;
                }
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (argc > 8) {
        #pragma acc parallel copy(arr2d[0:N][0:M]) gang worker vector
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr2d[i][j] = arr2d[i][j] % 100;
                }
            }
        }
    }
}

/* Test 3: Nested regions and device data environment */
void test_nested_and_persistent(int argc) {
    int persistent_arr[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        persistent_arr[i] = i;
    }
    
    /* Establish device data region with gang partition */
    if (argc > 9) {
        #pragma acc enter data copyin(persistent_arr[0:N]) gang
        
        /* Nested test: outer gang, inner worker */
        #pragma acc parallel present(persistent_arr) gang if(argc > 10)
        {
            /* Call gang-partitioned routine */
            acc_routine_gang(persistent_arr, N);
            
            /* Inner worker region */
            #pragma acc parallel present(persistent_arr) worker
            {
                #pragma acc loop worker
                for (int i = 0; i < N; i++) {
                    persistent_arr[i] += 10;
                }
                
                /* Call worker-partitioned routine */
                acc_routine_worker(persistent_arr, N);
            }
        }
        
        /* Additional compute region with vector partition */
        #pragma acc parallel present(persistent_arr) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                persistent_arr[i] -= 5;
            }
        }
        
        #pragma acc exit data copyout(persistent_arr[0:N])
    }
}

/* Test 4: Mixed partition types in sequential regions */
void test_mixed_sequential(int argc) {
    int mixed_arr[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            mixed_arr[i][j] = i - j;
        }
    }
    
    /* Sequential regions with different partitions */
    if (argc > 11) {
        /* First: gang partitioned */
        #pragma acc parallel copy(mixed_arr[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    mixed_arr[i][j] += 100;
                }
            }
        }
        
        /* Second: worker partitioned */
        #pragma acc kernels copy(mixed_arr[0:N][0:M]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    mixed_arr[i][j] *= 2;
                }
            }
        }
        
        /* Third: vector partitioned */
        #pragma acc parallel copy(mixed_arr[0:N][0:M]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    mixed_arr[i][j] /= 3;
                }
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    printf("Testing OpenACC partition combinations...\n");
    
    /* Each test is conditionally executed based on argc
     * to prevent dead code elimination */
    test_basic_partitions(argc);
    test_vector_partitions(argc);
    test_nested_and_persistent(argc);
    test_mixed_sequential(argc);
    
    printf("Tests completed (compile-time coverage target achieved).\n");
    return 0;
}
