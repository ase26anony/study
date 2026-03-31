/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper for verification */
static int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) return 0;
    }
    return 1;
}

/* Case 0: gang redundant - scalar reduction, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    float sum = 0.0f;
    float data[N];
    
    for (int i = 0; i < N; i++) data[i] = 1.0f;
    
    #pragma acc parallel copyin(data[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("  Sum = %f (expected %f)\n", sum, (float)N);
    assert(sum == (float)N);
}

/* Case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
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
        if (result[i] != (float)i * 2.0f) correct = 0;
    }
    assert(correct);
    printf("  Gang partitioned test passed\n");
}

/* Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    float arr[M];
    float result[M];
    
    for (int i = 0; i < M; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] + 10.0f;
        }
    }
    
    int correct = 1;
    for (int i = 0; i < M; i++) {
        if (result[i] != (float)i + 10.0f) correct = 0;
    }
    assert(correct);
    printf("  Worker partitioned test passed\n");
}

/* Case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    float matrix[N][M];
    float result[N][M];
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    #pragma acc parallel copyin(matrix[0:N][0:M]) copyout(result[0:N][0:M])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = matrix[i][j] * 3.0f;
            }
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (result[i][j] != (float)(i * M + j) * 3.0f) correct = 0;
        }
    }
    assert(correct);
    printf("  Gang+worker partitioned test passed\n");
}

/* Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
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
        if (result[i] != (float)(i * i)) correct = 0;
    }
    assert(correct);
    printf("  Vector partitioned test passed\n");
}

/* Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = sqrtf(arr[i] + 1.0f);
        }
    }
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        float expected = sqrtf((float)i + 1.0f);
        if (result[i] - expected > 0.0001f) correct = 0;
    }
    assert(correct);
    printf("  Gang+vector partitioned test passed\n");
}

/* Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    float arr[M];
    float result[M];
    
    for (int i = 0; i < M; i++) arr[i] = (float)i;
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = sinf(arr[i] * 0.1f);
        }
    }
    
    int correct = 1;
    for (int i = 0; i < M; i++) {
        float expected = sinf((float)i * 0.1f);
        if (result[i] - expected > 0.0001f) correct = 0;
    }
    assert(correct);
    printf("  Worker+vector partitioned test passed\n");
}

/* Case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    float arr3d[P][M][N];
    float result3d[P][M][N];
    
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                arr3d[i][j][k] = (float)(i * M * N + j * N + k);
            }
        }
    }
    
    #pragma acc parallel copyin(arr3d[0:P][0:M][0:N]) copyout(result3d[0:P][0:M][0:N])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < N; k++) {
                    result3d[i][j][k] = arr3d[i][j][k] * 0.5f + 1.0f;
                }
            }
        }
    }
    
    int correct = 1;
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                float expected = (float)(i * M * N + j * N + k) * 0.5f + 1.0f;
                if (result3d[i][j][k] - expected > 0.0001f) correct = 0;
            }
        }
    }
    assert(correct);
    printf("  Fully partitioned test passed\n");
}

/* Test with runtime parameters to influence partitioning decisions */
void test_runtime_partitioning(int size) {
    printf("Testing runtime partitioning with size=%d\n", size);
    float *dynamic_arr = (float*)malloc(size * sizeof(float));
    float *dynamic_result = (float*)malloc(size * sizeof(float));
    
    for (int i = 0; i < size; i++) dynamic_arr[i] = (float)i;
    
    /* This may produce different partition codes based on runtime size */
    #pragma acc parallel copyin(dynamic_arr[0:size]) copyout(dynamic_result[0:size]) \
        num_gangs(size/256) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < size; i++) {
            dynamic_result[i] = dynamic_arr[i] * 2.0f;
        }
    }
    
    int correct = 1;
    for (int i = 0; i < size; i++) {
        if (dynamic_result[i] != (float)i * 2.0f) correct = 0;
    }
    assert(correct);
    
    free(dynamic_arr);
    free(dynamic_result);
    printf("  Runtime partitioning test passed\n");
}

/* Test with present clause for already-resident data */
void test_present_clause() {
    printf("Testing with present clause\n");
    float present_arr[N];
    
    for (int i = 0; i < N; i++) present_arr[i] = (float)i;
    
    #pragma acc enter data copyin(present_arr[0:N])
    
    #pragma acc parallel present(present_arr[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            present_arr[i] += 100.0f;
        }
    }
    
    #pragma acc update host(present_arr[0:N])
    
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (present_arr[i] != (float)i + 100.0f) correct = 0;
    }
    assert(correct);
    
    #pragma acc exit data delete(present_arr[0:N])
    printf("  Present clause test passed\n");
}

/* Main function executing all test cases */
int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all 8 partition scenarios */
    test_gang_redundant();           /* Case 0 */
    test_gang_partitioned();         /* Case 1 */
    test_worker_partitioned();       /* Case 2 */
    test_gang_worker_partitioned();  /* Case 3 */
    test_vector_partitioned();       /* Case 4 */
    test_gang_vector_partitioned();  /* Case 5 */
    test_worker_vector_partitioned();/* Case 6 */
    test_fully_partitioned();        /* Case 7 */
    
    /* Additional tests to influence partitioning decisions */
    test_runtime_partitioning(512);
    test_runtime_partitioning(2048);
    test_present_clause();
    
    printf("\nAll partition mapping tests completed successfully!\n");
    printf("All 8 partition codes (0-7) should have been generated during compilation.\n");
    
    return 0;
}
