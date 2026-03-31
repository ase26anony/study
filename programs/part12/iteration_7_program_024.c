/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass, specifically covering
 * the switch statement cases for all partition types.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Global arrays for testing persistent data environments */
int global_3d_arr[N][M][P];
float global_2d_arr[N][M];

/* Function prototypes */
void test_gang_redundant(void);
void test_gang_partitioned(void);
void test_worker_partitioned(void);
void test_gang_worker_partitioned(void);
void test_vector_partitioned(void);
void test_gang_vector_partitioned(void);
void test_worker_vector_partitioned(void);
void test_fully_partitioned(void);
void test_nested_regions(void);
void test_device_data_env(void);

/* OpenACC routine with gang partitioning */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i * 2;
    }
}

/* OpenACC routine with vector partitioning */
#pragma acc routine seq vector
void acc_routine_vector(float *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 1.5f;
    }
}

/* Test case 0: gang redundant */
void test_gang_redundant(void) {
    int arr1d[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    /* OpenACC parallel with gang redundant partitioning */
    #pragma acc parallel copy(arr1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] += 1;
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr1d[i] != i + 1) errors++;
    }
    if (errors == 0) {
        printf("test_gang_redundant: PASS\n");
    } else {
        printf("test_gang_redundant: FAIL (%d errors)\n", errors);
    }
}

/* Test case 1: gang partitioned */
void test_gang_partitioned(void) {
    int arr2d[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = i * M + j;
        }
    }
    
    /* OpenACC kernels with gang partitioned mapping */
    #pragma acc kernels copy(arr2d[0:N][0:M]) gang(static:4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2d[i][j] *= 2;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (arr2d[i][j] != (i * M + j) * 2) errors++;
        }
    }
    if (errors == 0) {
        printf("test_gang_partitioned: PASS\n");
    } else {
        printf("test_gang_partitioned: FAIL (%d errors)\n", errors);
    }
}

/* Test case 2: worker partitioned */
void test_worker_partitioned(void) {
    float arr1d[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr1d[i] = (float)i;
    }
    
    /* OpenACC parallel with worker partitioning */
    #pragma acc parallel copy(arr1d[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr1d[i] += 0.5f;
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr1d[i] != (float)i + 0.5f) errors++;
    }
    if (errors == 0) {
        printf("test_worker_partitioned: PASS\n");
    } else {
        printf("test_worker_partitioned: FAIL (%d errors)\n", errors);
    }
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    int arr3d[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* OpenACC parallel with gang+worker partitioning on 3D array */
    #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop worker
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] += 10;
                }
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                int expected = i * M * P + j * P + k + 10;
                if (arr3d[i][j][k] != expected) errors++;
            }
        }
    }
    if (errors == 0) {
        printf("test_gang_worker_partitioned: PASS\n");
    } else {
        printf("test_gang_worker_partitioned: FAIL (%d errors)\n", errors);
    }
}

/* Test case 4: vector partitioned */
void test_vector_partitioned(void) {
    float arr2d[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = (float)(i * M + j);
        }
    }
    
    /* OpenACC kernels with vector partitioning */
    #pragma acc kernels copy(arr2d[0:N][0:M]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                arr2d[i][j] /= 2.0f;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            float expected = (float)(i * M + j) / 2.0f;
            if (arr2d[i][j] != expected) errors++;
        }
    }
    if (errors == 0) {
        printf("test_vector_partitioned: PASS\n");
    } else {
        printf("test_vector_partitioned: FAIL (%d errors)\n", errors);
    }
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    int arr1d[N * M];
    
    /* Initialize */
    for (int i = 0; i < N * M; i++) {
        arr1d[i] = i;
    }
    
    /* OpenACC parallel with gang+vector partitioning */
    #pragma acc parallel copy(arr1d[0:N*M]) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N * M; i++) {
            arr1d[i] -= 5;
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N * M; i++) {
        if (arr1d[i] != i - 5) errors++;
    }
    if (errors == 0) {
        printf("test_gang_vector_partitioned: PASS\n");
    } else {
        printf("test_gang_vector_partitioned: FAIL (%d errors)\n", errors);
    }
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    float arr3d[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = (float)(i * M * P + j * P + k);
            }
        }
    }
    
    /* OpenACC kernels with worker+vector partitioning */
    #pragma acc kernels copy(arr3d[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] *= 1.1f;
                }
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                float expected = (float)(i * M * P + j * P + k) * 1.1f;
                if (arr3d[i][j][k] != expected) errors++;
            }
        }
    }
    if (errors == 0) {
        printf("test_worker_vector_partitioned: PASS\n");
    } else {
        printf("test_worker_vector_partitioned: FAIL (%d errors)\n", errors);
    }
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    int arr2d[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = i * M + j;
        }
    }
    
    /* OpenACC parallel with full partitioning */
    #pragma acc parallel copy(arr2d[0:N][0:M]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = arr2d[i][j] * 3 + 7;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int expected = (i * M + j) * 3 + 7;
            if (arr2d[i][j] != expected) errors++;
        }
    }
    if (errors == 0) {
        printf("test_fully_partitioned: PASS\n");
    } else {
        printf("test_fully_partitioned: FAIL (%d errors)\n", errors);
    }
}

/* Test nested regions with different partition types */
void test_nested_regions(void) {
    int arr[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    /* First region with gang partitioning */
    #pragma acc parallel copy(arr[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i] += 100;
        }
    }
    
    /* Second region with vector partitioning */
    #pragma acc parallel copy(arr[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] -= 50;
        }
    }
    
    /* Call routine with gang partitioning */
    #pragma acc parallel copy(arr[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i += 8) {
            acc_routine_gang(&arr[i], 8);
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        int expected = i + 100 - 50 + (i % 8) * 2;
        if (arr[i] != expected) errors++;
    }
    if (errors == 0) {
        printf("test_nested_regions: PASS\n");
    } else {
        printf("test_nested_regions: FAIL (%d errors)\n", errors);
    }
}

/* Test device data environment with partition clauses */
void test_device_data_env(void) {
    /* Initialize global arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_2d_arr[i][j] = (float)(i * M + j);
            for (int k = 0; k < P; k++) {
                global_3d_arr[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(global_3d_arr[0:N][0:M][0:P]) gang
    
    /* Enter data with worker partitioning */
    #pragma acc enter data copyin(global_2d_arr[0:N][0:M]) worker
    
    /* Compute region 1: gang partitioned access */
    #pragma acc parallel present(global_3d_arr) gang
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop worker
                for (int k = 0; k < P; k++) {
                    global_3d_arr[i][j][k] += 1000;
                }
            }
        }
    }
    
    /* Compute region 2: worker partitioned access */
    #pragma acc parallel present(global_2d_arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                global_2d_arr[i][j] *= 2.0f;
            }
        }
    }
    
    /* Call vector-partitioned routine */
    #pragma acc parallel present(global_2d_arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            acc_routine_vector(&global_2d_arr[i][0], M);
        }
    }
    
    /* Exit data */
    #pragma acc exit data copyout(global_3d_arr[0:N][0:M][0:P])
    #pragma acc exit data copyout(global_2d_arr[0:N][0:M])
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            float expected = (float)(i * M + j) * 2.0f * 1.5f;
            if (global_2d_arr[i][j] != expected) errors++;
            
            for (int k = 0; k < P; k++) {
                int expected3d = i * M * P + j * P + k + 1000;
                if (global_3d_arr[i][j][k] != expected3d) errors++;
            }
        }
    }
    if (errors == 0) {
        printf("test_device_data_env: PASS\n");
    } else {
        printf("test_device_data_env: FAIL (%d errors)\n", errors);
    }
}

int main(int argc, char **argv) {
    printf("Starting OpenACC partition coverage tests...\n\n");
    
    /* Use argc to prevent dead code elimination */
    int test_mask = (argc > 1) ? atoi(argv[1]) : 0xFF;
    
    if (test_mask & 0x01) test_gang_redundant();
    if (test_mask & 0x02) test_gang_partitioned();
    if (test_mask & 0x04) test_worker_partitioned();
    if (test_mask & 0x08) test_gang_worker_partitioned();
    if (test_mask & 0x10) test_vector_partitioned();
    if (test_mask & 0x20) test_gang_vector_partitioned();
    if (test_mask & 0x40) test_worker_vector_partitioned();
    if (test_mask & 0x80) test_fully_partitioned();
    
    /* Additional complex tests */
    if (test_mask & 0x100) test_nested_regions();
    if (test_mask & 0x200) test_device_data_env();
    
    printf("\nAll tests completed.\n");
    return 0;
}
