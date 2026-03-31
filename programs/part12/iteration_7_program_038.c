/* Test program to cover OpenACC partition type string mappings */
#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition specification */
#pragma acc routine vec gang
void increment_gang(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] += 1;
    }
}

#pragma acc routine worker
void multiply_worker(int *arr, int size, int factor) {
    for (int i = 0; i < size; i++) {
        arr[i] *= factor;
    }
}

/* Test function 1: Basic partition combinations */
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
    
    /* 1. Gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
    
    /* 2. Gang partitioned */
    if (argc > 2) {
        #pragma acc kernels copy(arr2[0:N][0:M]) gang(static:10)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    arr2[i][j] *= 2;
                }
            }
        }
    }
}

/* Test function 2: Worker and vector partitions */
void test_worker_vector_partitions(int argc) {
    int arr3[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i + j + k;
            }
        }
    }
    
    /* 3. Worker partitioned */
    if (argc > 3) {
        #pragma acc parallel copy(arr3[0:N][0:M][0:P]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] += 3;
                    }
                }
            }
        }
    }
    
    /* 4. Gang+worker partitioned */
    if (argc > 4) {
        #pragma acc kernels create(arr3[0:N][0:M][0:P]) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* 5. Vector partitioned */
    if (argc > 5) {
        #pragma acc parallel copy(arr3[0:N][0:M][0:P]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3[i][j][k] -= 1;
                    }
                }
            }
        }
    }
}

/* Test function 3: Combined partitions */
void test_combined_partitions(int argc) {
    int arr4[N][M];
    int arr5[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr4[i][j] = i * 2 + j;
            arr5[i][j] = i + j * 3;
        }
    }
    
    /* 6. Gang+vector partitioned */
    if (argc > 6) {
        #pragma acc parallel copy(arr4[0:N][0:M]) gang vector
        {
            #pragma acc loop gang vector collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr4[i][j] += i + j;
                }
            }
        }
    }
    
    /* 7. Worker+vector partitioned */
    if (argc > 7) {
        #pragma acc kernels copy(arr5[0:N][0:M]) worker vector
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr5[i][j] *= (i + 1);
                }
            }
        }
    }
}

/* Test function 4: Fully partitioned and nested regions */
void test_fully_partitioned(int argc) {
    int arr6[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr6[i][j][k] = i * j * k;
            }
        }
    }
    
    /* 8. Fully partitioned (gang+worker+vector) */
    if (argc > 8) {
        #pragma acc parallel copy(arr6[0:N][0:M][0:P]) gang worker vector
        {
            #pragma acc loop gang worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr6[i][j][k] = arr6[i][j][k] / 2 + 1;
                    }
                }
            }
        }
    }
    
    /* Nested region with different partition */
    if (argc > 9) {
        #pragma acc parallel if(argc > 10) gang copy(arr6[0:N][0:M][0:P])
        {
            /* Outer gang region */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Inner worker region */
                #pragma acc parallel worker present(arr6)
                {
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        for (int k = 0; k < P; k++) {
                            arr6[i][j][k] += 100;
                        }
                    }
                }
            }
        }
    }
}

/* Test function 5: Device data environment with partitions */
void test_device_data_partitions(int argc) {
    int arr7[N][M];
    int arr8[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr7[i][j] = i - j;
            arr8[i][j] = j - i;
        }
    }
    
    /* Enter data with gang partition */
    if (argc > 11) {
        #pragma acc enter data copyin(arr7[0:N][0:M]) gang
        
        /* Compute with worker partition on already-present data */
        #pragma acc parallel present(arr7) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr7[i][j] = abs(arr7[i][j]);
                }
            }
        }
        
        /* Exit data */
        #pragma acc exit data copyout(arr7[0:N][0:M])
    }
    
    /* Mixed partition types with routine calls */
    if (argc > 12) {
        #pragma acc enter data copyin(arr8[0:N][0:M]) vector
        
        #pragma acc parallel present(arr8) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Call gang routine */
                increment_gang(&arr8[i][0], M);
            }
        }
        
        #pragma acc parallel present(arr8) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                /* Call worker routine */
                multiply_worker(&arr8[i][0], M, 2);
            }
        }
        
        #pragma acc exit data copyout(arr8[0:N][0:M])
    }
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    printf("Testing OpenACC partition type coverage...\n");
    
    /* Execute all test functions with argc-based conditions
       to prevent dead code elimination */
    test_basic_partitions(argc);
    test_worker_vector_partitions(argc);
    test_combined_partitions(argc);
    test_fully_partitioned(argc);
    test_device_data_partitions(argc);
    
    /* Final validation region with mixed partitions */
    int final_arr[10][20][30];
    
    #pragma acc parallel copy(final_arr[0:10][0:20][0:30]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < 30; k++) {
                    final_arr[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
    
    /* Verify results */
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                if (final_arr[i][j][k] != i * 100 + j * 10 + k) {
                    errors++;
                }
            }
        }
    }
    
    printf("Test completed with %d errors\n", errors);
    return errors > 0 ? 1 : 0;
}
