/* test_openacc_partitions.c */
#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Routine with explicit partition type */
#pragma acc routine vec gang
void increment_element(int *arr, int idx, int val) {
    arr[idx] += val;
}

/* Function with nested compute regions */
void test_nested_partitions(int argc) {
    int arr1[N][M];
    int arr2[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i * M + j;
            arr2[i][j] = 0;
        }
    }
    
    /* Case 0: gang redundant (implicit) */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr1[i][j] *= 2;
            }
        }
    }
    
    /* Nested region inside conditional */
    if (argc > 2) {
        /* Case 2: worker partitioned */
        #pragma acc kernels copy(arr2[0:N][0:M]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr2[i][j] = arr1[i][j] + i + j;
                }
            }
        }
    }
}

/* Function with multi-dimensional array and collapse */
void test_multi_dim_partitions(int argc) {
    int arr3d[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 3) {
        #pragma acc kernels copy(arr3d[0:N][0:M][0:P]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] *= 3;
                    }
                }
            }
        }
    }
}

/* Function with device data environment */
void test_persistent_data(int argc) {
    int persistent_arr[N * M];
    
    /* Initialize */
    for (int i = 0; i < N * M; i++) {
        persistent_arr[i] = i;
    }
    
    /* Case 5: gang+vector partitioned with enter/exit data */
    #pragma acc enter data copyin(persistent_arr[0:N*M]) gang vector
    
    if (argc > 4) {
        /* Compute region with present clause */
        #pragma acc parallel present(persistent_arr[0:N*M]) gang vector
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N * M; i++) {
                persistent_arr[i] += 10;
                /* Call routine with partition type */
                increment_element(persistent_arr, i, 5);
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel present(persistent_arr[0:N*M]) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N * M; i += 2) {
            persistent_arr[i] *= 2;
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc kernels present(persistent_arr[0:N*M]) gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N * M; i++) {
            if (persistent_arr[i] > 1000) {
                persistent_arr[i] = 1000;
            }
        }
    }
    
    #pragma acc exit data copyout(persistent_arr[0:N*M])
    
    /* Verify results */
    int errors = 0;
    for (int i = 0; i < N * M; i++) {
        if (persistent_arr[i] < 0) {
            errors++;
        }
    }
    if (errors > 0) {
        printf("Found %d errors in persistent array\n", errors);
    }
}

/* Additional test with mixed partition types */
void test_mixed_partitions(int argc) {
    int arr4[N][M];
    int arr5[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr4[i][j] = i + j;
            arr5[i][j] = i * j;
        }
    }
    
    /* Sequential regions with different partitions */
    #pragma acc parallel copy(arr4[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr4[i][j] += arr5[i][j];
            }
        }
    }
    
    if (argc > 5) {
        #pragma acc kernels copy(arr5[0:N][0:M]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    arr5[i][j] = arr4[i][j] - arr5[i][j];
                }
            }
        }
    }
    
    /* Region with if clause */
    int condition = (argc > 6);
    #pragma acc parallel copy(arr4[0:N][0:M]) if(condition) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr4[i][j] /= 2;
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition combinations...\n");
    
    /* Execute all test functions with argc for conditional execution */
    test_nested_partitions(argc);
    test_multi_dim_partitions(argc);
    test_persistent_data(argc);
    test_mixed_partitions(argc);
    
    /* Final validation */
    int final_arr[10];
    #pragma acc parallel copy(final_arr[0:10]) gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 10; i++) {
            final_arr[i] = i * argc;
        }
    }
    
    /* Check results */
    int valid = 1;
    for (int i = 0; i < 10; i++) {
        if (final_arr[i] != i * argc) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        printf("All tests completed (compilation coverage target achieved)\n");
    } else {
        printf("Result validation failed\n");
    }
    
    return 0;
}
