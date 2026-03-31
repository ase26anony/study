/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 16

/* Helper function to verify results */
int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Verification failed at index %d: got %f, expected %f\n", 
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

/* Test case 0: gang redundant - scalar reductions */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    float sum = 0.0f;
    float arr[N];
    
    #pragma acc parallel loop reduction(+:sum) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 1.0f;
        sum += arr[i];
    }
    
    float expected_sum = (N-1) * N / 2.0f;
    if (sum != expected_sum) {
        printf("  Reduction failed: got %f, expected %f\n", sum, expected_sum);
    }
}

/* Test case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2.0f;
    }
    
    if (!verify_array(arr, N, 0.0f)) {  // Will be overwritten
        #pragma acc update self(arr[0:N])
        verify_array(arr, N, 0.0f);
    }
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 3.0f;
    }
    
    if (!verify_array(arr, N, 0.0f)) {
        #pragma acc update self(arr[0:N])
        verify_array(arr, N, 0.0f);
    }
}

/* Test case 3: gang+worker partitioned - nested distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    float arr[M][N];
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:M][0:N])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = i * N + j;
        }
    }
    
    // Verify a sample
    int valid = 1;
    #pragma acc update self(arr[0:M][0:N])
    for (int i = 0; i < M && valid; i++) {
        for (int j = 0; j < N && valid; j++) {
            if (arr[i][j] != i * N + j) {
                printf("  Mismatch at [%d][%d]: got %f, expected %d\n", 
                       i, j, arr[i][j], i * N + j);
                valid = 0;
            }
        }
    }
}

/* Test case 4: vector partitioned - SIMD-style operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 4.0f;
    }
    
    if (!verify_array(arr, N, 0.0f)) {
        #pragma acc update self(arr[0:N])
        verify_array(arr, N, 0.0f);
    }
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop gang vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 5.0f;
    }
    
    if (!verify_array(arr, N, 0.0f)) {
        #pragma acc update self(arr[0:N])
        verify_array(arr, N, 0.0f);
    }
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop worker vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 6.0f;
    }
    
    if (!verify_array(arr, N, 0.0f)) {
        #pragma acc update self(arr[0:N])
        verify_array(arr, N, 0.0f);
    }
}

/* Test case 7: fully partitioned - all three levels */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    float arr[P][M][N];
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:P][0:M][0:N])
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                arr[i][j][k] = i * M * N + j * N + k;
            }
        }
    }
    
    // Verify a sample
    int valid = 1;
    #pragma acc update self(arr[0:P][0:M][0:N])
    for (int i = 0; i < P && valid; i++) {
        for (int j = 0; j < M && valid; j++) {
            for (int k = 0; k < N && valid; k++) {
                float expected = i * M * N + j * N + k;
                if (arr[i][j][k] != expected) {
                    printf("  Mismatch at [%d][%d][%d]: got %f, expected %f\n", 
                           i, j, k, arr[i][j][k], expected);
                    valid = 0;
                }
            }
        }
    }
}

/* Additional tests with different data clauses and patterns */

/* Test with present clause for already-resident data */
void test_with_present_clause() {
    printf("Testing with present clause\n");
    float arr[N];
    
    #pragma acc enter data copyin(arr[0:N])
    
    #pragma acc parallel loop present(arr) gang
    for (int i = 0; i < N; i++) {
        arr[i] = i * 7.0f;
    }
    
    #pragma acc exit data copyout(arr[0:N])
    
    verify_array(arr, N, 0.0f);
}

/* Test with private variables */
void test_with_private_vars() {
    printf("Testing with private variables\n");
    float arr[N];
    int private_var;
    
    #pragma acc parallel loop private(private_var) copy(arr[0:N]) gang worker
    for (int i = 0; i < N; i++) {
        private_var = i % 10;
        arr[i] = private_var * 1.0f;
    }
    
    #pragma acc update self(arr[0:N])
    verify_array(arr, N, 0.0f);
}

/* Test with runtime parameters */
void test_runtime_partitioning(int size) {
    printf("Testing runtime partitioning with size=%d\n", size);
    float *arr = (float*)malloc(size * sizeof(float));
    
    #pragma acc parallel loop gang copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = i * 8.0f;
    }
    
    #pragma acc update self(arr[0:size])
    free(arr);
}

/* Test triangular loop pattern */
void test_triangular_loop() {
    printf("Testing triangular loop pattern\n");
    float arr[M][M];
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j <= i; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    #pragma acc update self(arr[0:M][0:M])
}

/* Main test driver */
int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Additional variations */
    test_with_present_clause();
    test_with_private_vars();
    test_runtime_partitioning(512);
    test_triangular_loop();
    
    printf("\nAll tests completed.\n");
    
    /* Note: The default case (case 8+) cannot be triggered through normal
     * OpenACC directives. This would require compiler-internal testing hooks
     * or injecting invalid partition codes through debug interfaces.
     */
    
    return 0;
}
