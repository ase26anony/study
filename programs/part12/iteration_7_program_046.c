/* test_openacc_partitions.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_openacc_partitions.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Function prototypes */
void test_gang_redundant(int argc);
void test_gang_partitioned(int argc);
void test_worker_partitioned(int argc);
void test_gang_worker_partitioned(int argc);
void test_vector_partitioned(int argc);
void test_gang_vector_partitioned(int argc);
void test_worker_vector_partitioned(int argc);
void test_fully_partitioned(int argc);
void test_mixed_regions(int argc);
void test_nested_partitions(int argc);
void test_device_data_env(int argc);

/* Routines with explicit partition directives */
#pragma acc routine vec
void vec_multiply(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
}

#pragma acc routine gang
void gang_reduction(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    arr[0] = sum;
}

#pragma acc routine worker
void worker_transform(float *arr, int n, float scale) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * scale;
    }
}

/* Test 1: Gang redundant partitioning */
void test_gang_redundant(int argc) {
    if (argc < 2) return;
    
    int arr3d[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* OpenACC region with gang redundant partitioning */
    #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                if (arr3d[i][j][k] != i * 100 + j * 10 + k + 1) {
                    errors++;
                }
            }
        }
    }
    if (errors > 0) printf("Test 1: %d errors\n", errors);
}

/* Test 2: Gang partitioned */
void test_gang_partitioned(int argc) {
    if (argc < 3) return;
    
    float matrix[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    /* Kernels with gang partitioned data */
    #pragma acc kernels copy(matrix[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                matrix[i][j] *= 2.0f;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (matrix[i][j] != (float)((i * M + j) * 2)) {
                errors++;
            }
        }
    }
    if (errors > 0) printf("Test 2: %d errors\n", errors);
}

/* Test 3: Worker partitioned */
void test_worker_partitioned(int argc) {
    if (argc < 4) return;
    
    double arr[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (double)(i + j);
        }
    }
    
    /* Parallel region with worker partitioning */
    #pragma acc parallel copy(arr[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr[i][j] += 1.5;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (arr[i][j] != (double)(i + j) + 1.5) {
                errors++;
            }
        }
    }
    if (errors > 0) printf("Test 3: %d errors\n", errors);
}

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(int argc) {
    if (argc < 5) return;
    
    int arr3d[10][20][30];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                arr3d[i][j][k] = i * 400 + j * 20 + k;
            }
        }
    }
    
    /* Collapsed loop with gang+worker partitioning */
    #pragma acc parallel loop collapse(3) gang worker copy(arr3d[0:10][0:20][0:30])
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                arr3d[i][j][k] -= 5;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                if (arr3d[i][j][k] != i * 400 + j * 20 + k - 5) {
                    errors++;
                }
            }
        }
    }
    if (errors > 0) printf("Test 4: %d errors\n", errors);
}

/* Test 5: Vector partitioned */
void test_vector_partitioned(int argc) {
    if (argc < 6) return;
    
    float vec1[N], vec2[N], result[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        vec1[i] = (float)i;
        vec2[i] = (float)(i * 2);
        result[i] = 0.0f;
    }
    
    /* Vector partitioned computation with routine call */
    #pragma acc parallel copyin(vec1[0:N], vec2[0:N]) copyout(result[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = vec1[i] + vec2[i];
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != (float)(i * 3)) {
            errors++;
        }
    }
    if (errors > 0) printf("Test 5: %d errors\n", errors);
}

/* Test 6: Gang+vector partitioned */
void test_gang_vector_partitioned(int argc) {
    if (argc < 7) return;
    
    int matrix[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = i * M + j;
        }
    }
    
    /* Gang+vector partitioning with conditional offloading */
    int condition = (argc > 7);
    #pragma acc parallel if(condition) copy(matrix[0:N][0:M]) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                matrix[i][j] *= 3;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (matrix[i][j] != (i * M + j) * 3) {
                errors++;
            }
        }
    }
    if (errors > 0) printf("Test 6: %d errors\n", errors);
}

/* Test 7: Worker+vector partitioned */
void test_worker_vector_partitioned(int argc) {
    if (argc < 8) return;
    
    float arr3d[8][16][32];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                arr3d[i][j][k] = (float)(i * 512 + j * 32 + k);
            }
        }
    }
    
    /* Worker+vector partitioning */
    #pragma acc kernels copy(arr3d[0:8][0:16][0:32]) worker vector
    {
        #pragma acc loop worker
        for (int i = 0; i < 8; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 16; j++) {
                for (int k = 0; k < 32; k++) {
                    arr3d[i][j][k] /= 2.0f;
                }
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                if (arr3d[i][j][k] != (float)(i * 512 + j * 32 + k) / 2.0f) {
                    errors++;
                }
            }
        }
    }
    if (errors > 0) printf("Test 7: %d errors\n", errors);
}

/* Test 8: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int argc) {
    if (argc < 9) return;
    
    double arr4d[4][8][16][32];
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                for (int l = 0; l < 32; l++) {
                    arr4d[i][j][k][l] = (double)(i * 4096 + j * 512 + k * 32 + l);
                }
            }
        }
    }
    
    /* Fully partitioned 4D array */
    #pragma acc parallel copy(arr4d[0:4][0:8][0:16][0:32]) gang worker vector
    {
        #pragma acc loop gang
        for (int i = 0; i < 4; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 8; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 16; k++) {
                    for (int l = 0; l < 32; l++) {
                        arr4d[i][j][k][l] += 100.0;
                    }
                }
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                for (int l = 0; l < 32; l++) {
                    if (arr4d[i][j][k][l] != (double)(i * 4096 + j * 512 + k * 32 + l) + 100.0) {
                        errors++;
                    }
                }
            }
        }
    }
    if (errors > 0) printf("Test 8: %d errors\n", errors);
}

/* Test 9: Mixed regions with different partition types */
void test_mixed_regions(int argc) {
    if (argc < 10) return;
    
    int data[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    /* Sequence of regions with different partition types */
    #pragma acc parallel copy(data[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] += 10;
        }
    }
    
    #pragma acc kernels copy(data[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] *= 2;
        }
    }
    
    #pragma acc parallel copy(data[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] -= 5;
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != ((i + 10) * 2) - 5) {
            errors++;
        }
    }
    if (errors > 0) printf("Test 9: %d errors\n", errors);
}

/* Test 10: Device data environment with partitioning */
void test_device_data_env(int argc) {
    if (argc < 11) return;
    
    float persistent[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            persistent[i][j] = (float)(i * M + j);
        }
    }
    
    /* Create device data with gang partitioning */
    #pragma acc enter data copyin(persistent[0:N][0:M]) gang
    
    /* Multiple compute regions using the persistent data */
    #pragma acc parallel present(persistent) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                persistent[i][j] += 1.0f;
            }
        }
    }
    
    #pragma acc kernels present(persistent) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                persistent[i][j] *= 1.5f;
            }
        }
    }
    
    /* Copy back and verify */
    #pragma acc exit data copyout(persistent[0:N][0:M])
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            float expected = ((float)(i * M + j) + 1.0f) * 1.5f;
            if (persistent[i][j] != expected) {
                errors++;
            }
        }
    }
    if (errors > 0) printf("Test 10: %d errors\n", errors);
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    
    /* Run all partition tests with argc-based conditions */
    test_gang_redundant(argc);
    test_gang_partitioned(argc);
    test_worker_partitioned(argc);
    test_gang_worker_partitioned(argc);
    test_vector_partitioned(argc);
    test_gang_vector_partitioned(argc);
    test_worker_vector_partitioned(argc);
    test_fully_partitioned(argc);
    test_mixed_regions(argc);
    test_device_data_env(argc);
    
    /* Additional complex test with routine directives */
    if (argc > 1) {
        float a[N], b[N], c[N];
        
        for (int i = 0; i < N; i++) {
            a[i] = (float)i;
            b[i] = (float)(i + 1);
            c[i] = 0.0f;
        }
        
        /* Call routine with vector partitioning */
        #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * b[i];
            }
        }
        
        /* Verify routine results */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (c[i] != (float)(i * (i + 1))) {
                errors++;
            }
        }
        if (errors > 0) printf("Routine test: %d errors\n", errors);
    }
    
    printf("All tests completed (compile-time coverage is the primary goal)\n");
    return 0;
}
