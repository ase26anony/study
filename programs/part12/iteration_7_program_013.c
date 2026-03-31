/* Test program to cover partition type string mapping in GCC OpenACC neuter-broadcast pass */
#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition type */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

#pragma acc routine worker
void acc_routine_worker(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

/* Test function 1: Basic partition types */
void test_basic_partitions(int argc) {
    int arr1[N][M];
    int arr2[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = i * j;
        }
    }
    
    /* Case 0: gang redundant */
    #pragma acc parallel copy(arr1[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 1) {
        #pragma acc kernels copy(arr2[0:N][0:M]) gang(static:2)
        {
            #pragma acc loop gang(static:2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr2[i][j] *= 2;
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] -= 1;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc kernels create(arr2[0:N][0:M]) gang, worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] += i;
            }
        }
    }
}

/* Test function 2: Vector and combined partitions */
void test_vector_partitions(int use_alt) {
    int arr3[N][M][P];
    int arr4[N][M];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(arr3[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3[i][j][k] *= 3;
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (use_alt) {
        #pragma acc kernels copy(arr3[0:N][0:M][0:P]) gang, vector
        {
            #pragma acc loop gang vector collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] += k;
                    }
                }
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel copy(arr4[0:N][0:M]) worker, vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr4[i][j] = i - j;
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc kernels copy(arr3[0:N][0:M][0:P]) gang, worker, vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3[i][j][k] /= 2;
                }
            }
        }
    }
}

/* Test function 3: Nested regions and device data environment */
void test_nested_and_persistent(int flag) {
    int persistent_arr[N][M];
    int temp_arr[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            persistent_arr[i][j] = i * 10 + j;
        }
        temp_arr[i] = i;
    }
    
    /* Persistent device data with partition */
    #pragma acc enter data copyin(persistent_arr[0:N][0:M]) gang
    
    /* Nested compute regions with different partitions */
    if (flag) {
        #pragma acc parallel present(persistent_arr) gang
        {
            /* Outer gang region */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Inner worker region */
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    persistent_arr[i][j] += 5;
                }
            }
        }
    }
    
    /* Routine calls with partition propagation */
    #pragma acc parallel copy(temp_arr[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            if (i % 2 == 0) {
                acc_routine_gang(&temp_arr[i], 1);
            } else {
                acc_routine_worker(&temp_arr[i], 1);
            }
        }
    }
    
    /* Mixed partition types in sequential regions */
    #pragma acc parallel present(persistent_arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                persistent_arr[i][j] *= 2;
            }
        }
    }
    
    #pragma acc exit data copyout(persistent_arr[0:N][0:M]) gang
}

/* Test function 4: Complex multi-dimensional with collapse */
void test_collapse_partitions(int dim) {
    int arr5[10][20][30][15];
    
    /* Initialize 4D array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                for (int l = 0; l < 15; l++) {
                    arr5[i][j][k][l] = i + j + k + l;
                }
            }
        }
    }
    
    /* Different collapse factors with various partitions */
    switch (dim) {
        case 1:
            /* gang partitioned with collapse */
            #pragma acc parallel copy(arr5[0:10][0:20][0:30][0:15]) gang
            {
                #pragma acc loop gang collapse(2)
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 20; j++) {
                        for (int k = 0; k < 30; k++) {
                            for (int l = 0; l < 15; l++) {
                                arr5[i][j][k][l] += 1;
                            }
                        }
                    }
                }
            }
            break;
            
        case 2:
            /* worker+vector with collapse */
            #pragma acc kernels copy(arr5[0:10][0:20][0:30][0:15]) worker, vector
            {
                #pragma acc loop worker vector collapse(3)
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 20; j++) {
                        for (int k = 0; k < 30; k++) {
                            for (int l = 0; l < 15; l++) {
                                arr5[i][j][k][l] *= 2;
                            }
                        }
                    }
                }
            }
            break;
            
        default:
            /* fully partitioned with full collapse */
            #pragma acc parallel copy(arr5[0:10][0:20][0:30][0:15]) gang, worker, vector
            {
                #pragma acc loop gang worker vector collapse(4)
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 20; j++) {
                        for (int k = 0; k < 30; k++) {
                            for (int l = 0; l < 15; l++) {
                                arr5[i][j][k][l] /= 2;
                            }
                        }
                    }
                }
            }
            break;
    }
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int test_flag = argc > 1 ? atoi(argv[1]) : 0;
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Execute different test paths based on input */
    test_basic_partitions(argc);
    
    if (test_flag & 1) {
        test_vector_partitions(1);
    } else {
        test_vector_partitions(0);
    }
    
    if (test_flag & 2) {
        test_nested_and_persistent(1);
    }
    
    test_collapse_partitions(argc % 3);
    
    /* Final validation computation */
    int final_arr[N];
    for (int i = 0; i < N; i++) {
        final_arr[i] = i;
    }
    
    /* One more partition combination for completeness */
    #pragma acc parallel copy(final_arr[0:N]) gang, vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            final_arr[i] = final_arr[i] * 3 + 1;
        }
    }
    
    /* Verify results (simplified check) */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += final_arr[i];
    }
    
    printf("Test completed. Final sum: %d\n", sum);
    printf("If compiled with -fopenacc, the neuter-broadcast pass should have\n");
    printf("processed all partition types (0-7) in the switch statement.\n");
    
    return 0;
}
