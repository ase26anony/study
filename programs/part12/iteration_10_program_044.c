/* Test program to exercise all OpenACC partition mapping cases
 * Target: omp-oacc-neuter-broadcast.cc lines 335-343
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 16

/* Helper for verification */
static int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) return 0;
    }
    return 1;
}

/* Test case 0: gang redundant - scalar reduction, no data partitioning */
void test_gang_redundant() {
    float sum = 0.0f;
    float data[N];
    
    for (int i = 0; i < N; i++) data[i] = 1.0f;
    
    #pragma acc parallel copy(data[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("Test 0 (gang redundant): sum = %f\n", sum);
}

/* Test case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0f;
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 2.0f) correct = 0;
    }
    printf("Test 1 (gang partitioned): %s\n", correct ? "PASS" : "FAIL");
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] + 1.0f;
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] + 1.0f) correct = 0;
    }
    printf("Test 2 (worker partitioned): %s\n", correct ? "PASS" : "FAIL");
}

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    float matrix[M][M];
    float result[M][M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    #pragma acc parallel copyin(matrix) copyout(result)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = matrix[i][j] * 3.0f;
            }
        }
    }
    
    int correct = 1;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (result[i][j] != matrix[i][j] * 3.0f) correct = 0;
        }
    }
    printf("Test 3 (gang+worker partitioned): %s\n", correct ? "PASS" : "FAIL");
}

/* Test case 4: vector partitioned - SIMD-style vector operations */
void test_vector_partitioned() {
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * arr[i];
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * arr[i]) correct = 0;
    }
    printf("Test 4 (vector partitioned): %s\n", correct ? "PASS" : "FAIL");
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = sqrtf(arr[i]);
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != sqrtf(arr[i])) correct = 0;
    }
    printf("Test 5 (gang+vector partitioned): %s\n", correct ? "PASS" : "FAIL");
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(8) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = sinf(arr[i]);
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (result[i] != sinf(arr[i])) correct = 0;
    }
    printf("Test 6 (worker+vector partitioned): %s\n", correct ? "PASS" : "FAIL");
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    float arr3d[P][M][N/P];
    float result3d[P][M][N/P];
    
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N/P; k++) {
                arr3d[i][j][k] = (float)(i * M * (N/P) + j * (N/P) + k);
            }
        }
    }
    
    #pragma acc parallel copyin(arr3d) copyout(result3d)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < N/P; k++) {
                    result3d[i][j][k] = arr3d[i][j][k] * 2.0f + 1.0f;
                }
            }
        }
    }
    
    int correct = 1;
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N/P; k++) {
                if (result3d[i][j][k] != arr3d[i][j][k] * 2.0f + 1.0f) correct = 0;
            }
        }
    }
    printf("Test 7 (fully partitioned): %s\n", correct ? "PASS" : "FAIL");
}

/* Test default case - using runtime parameters to potentially trigger edge cases */
void test_edge_cases() {
    int size = 100;
    float *dynamic_arr = (float*)malloc(size * sizeof(float));
    float *dynamic_result = (float*)malloc(size * sizeof(float));
    
    for (int i = 0; i < size; i++) dynamic_arr[i] = (float)i;
    
    /* Variable loop bounds and runtime parameters */
    int gang_size = 2;
    int worker_size = 4;
    int vector_size = 8;
    
    #pragma acc parallel copyin(dynamic_arr[0:size]) copyout(dynamic_result[0:size]) \
        num_gangs(gang_size) num_workers(worker_size) vector_length(vector_size)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < size; i++) {
            dynamic_result[i] = dynamic_arr[i] / 2.0f;
        }
    }
    
    int correct = 1;
    for (int i = 0; i < size; i++) {
        if (dynamic_result[i] != dynamic_arr[i] / 2.0f) correct = 0;
    }
    printf("Test edge cases (runtime params): %s\n", correct ? "PASS" : "FAIL");
    
    free(dynamic_arr);
    free(dynamic_result);
}

int main() {
    printf("=== OpenACC Partition Mapping Test ===\n");
    printf("Testing all 8 partition cases from omp-oacc-neuter-broadcast.cc\n\n");
    
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_edge_cases();
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
