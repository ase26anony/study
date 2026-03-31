/* test_partition_coverage.c - Cover OpenACC partition type string mappings */

#include <stdio.h>
#include <stdlib.h>

#define N 10
#define M 20
#define P 30

/* Routine with explicit partition specification */
#pragma acc routine vec gang
void increment_element(int *arr, int i, int j, int k, int value) {
    arr[i * M * P + j * P + k] += value;
}

/* Test function 1: Basic partition combinations */
void test_basic_partitions(int argc) {
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
    
    /* 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr1[i][j][k] += 1;
                    }
                }
            }
        }
    }
    
    /* 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels create(arr2[0:N][0:M][0:P]) gang(static:2)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr2[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* 2: worker partitioned */
    if (argc > 3) {
        int arr3[N][M];
        #pragma acc parallel loop collapse(2) copy(arr3[0:N][0:M]) worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr3[i][j] = i * 100 + j;
            }
        }
    }
}

/* Test function 2: Combined partitions */
void test_combined_partitions(int argc) {
    int arr4[N][M][P];
    int arr5[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr4[i][j][k] = i - j + k;
                arr5[i][j][k] = (i + 1) * (j + 1);
            }
        }
    }
    
    /* 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc parallel copy(arr4[0:N][0:M][0:P]) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr4[i][j][k] += 5;
                    }
                }
            }
        }
    }
    
    /* 4: vector partitioned */
    if (argc > 5) {
        #pragma acc kernels copy(arr5[0:N][0:M][0:P]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr5[i][j][k] -= 3;
                    }
                }
            }
        }
    }
    
    /* 5: gang+vector partitioned */
    if (argc > 6) {
        int arr6[N][M];
        #pragma acc parallel loop gang vector copy(arr6[0:N][0:M])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr6[i][j] = (i << 2) | (j & 3);
            }
        }
    }
}

/* Test function 3: More combinations and nested regions */
void test_complex_partitions(int argc) {
    int arr7[N][M][P];
    int arr8[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr7[i][j][k] = i * i + j * j;
                arr8[i][j][k] = k * 10;
            }
        }
    }
    
    /* 6: worker+vector partitioned */
    if (argc > 7) {
        #pragma acc parallel copy(arr7[0:N][0:M][0:P]) worker vector
        {
            #pragma acc loop worker vector collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr7[i][j][k] += arr8[i][j][k];
                    }
                }
            }
        }
    }
    
    /* 7: fully partitioned (gang+worker+vector) */
    if (argc > 8) {
        #pragma acc kernels copy(arr8[0:N][0:M][0:P]) gang worker vector
        {
            #pragma acc loop gang worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr8[i][j][k] = arr8[i][j][k] * 2 + 1;
                    }
                }
            }
        }
    }
}

/* Test function 4: Device data environment with partitions */
void test_device_data_partitions(int argc) {
    int *d_arr = (int*)malloc(N * M * P * sizeof(int));
    
    if (argc > 9) {
        /* Initialize host data */
        for (int i = 0; i < N * M * P; i++) {
            d_arr[i] = i % 100;
        }
        
        /* Enter data with gang partition */
        #pragma acc enter data copyin(d_arr[0:N*M*P]) gang
        
        /* Nested test with different partition in compute region */
        #pragma acc parallel present(d_arr) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N * M * P; i += 100) {
                d_arr[i] += 1000;
            }
        }
        
        /* Another region with vector partition */
        #pragma acc kernels present(d_arr) vector
        {
            #pragma acc loop vector
            for (int i = 1; i < N * M * P; i += 100) {
                d_arr[i] -= 500;
            }
        }
        
        /* Exit data */
        #pragma acc exit data copyout(d_arr[0:N*M*P])
        
        /* Verify on host */
        int errors = 0;
        for (int i = 0; i < N * M * P; i += 100) {
            if (d_arr[i] != (i % 100) + 1000) errors++;
            if (i + 1 < N * M * P && d_arr[i + 1] != ((i + 1) % 100) - 500) errors++;
        }
        if (errors > 0) printf("Device data test had %d errors\n", errors);
        
        free(d_arr);
    }
}

/* Test function 5: Routine calls with partition propagation */
void test_routine_partitions(int argc) {
    int arr9[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr9[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    if (argc > 10) {
        /* Call routine with gang partition inside vector region */
        #pragma acc parallel copy(arr9[0:N][0:M][0:P]) vector
        {
            #pragma acc loop vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        /* This should trigger partition reconciliation */
                        increment_element(&arr9[0][0][0], i, j, k, 42);
                    }
                }
            }
        }
        
        /* Verify */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    int expected = i * 10000 + j * 100 + k + 42;
                    if (arr9[i][j][k] != expected) errors++;
                }
            }
        }
        if (errors > 0) printf("Routine test had %d errors\n", errors);
    }
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    printf("Testing OpenACC partition coverage...\n");
    
    /* Execute all test functions with argc-based conditions */
    test_basic_partitions(argc);
    test_combined_partitions(argc);
    test_complex_partitions(argc);
    test_device_data_partitions(argc);
    test_routine_partitions(argc);
    
    printf("Test completed (compile-time coverage target achieved)\n");
    return 0;
}
