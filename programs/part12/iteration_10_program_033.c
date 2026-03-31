/* Test program to exercise all OpenACC partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper function to verify results */
int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Mismatch at index %d: got %f, expected %f\n", i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

/* Test 0: gang redundant - scalar reduction, no data partitioning across gangs */
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
    if (fabs(sum - expected_sum) > 1e-6) {
        printf("  FAIL: sum = %f, expected %f\n", sum, expected_sum);
    } else {
        printf("  PASS\n");
    }
}

/* Test 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2.0f;
    }
    
    if (verify_array(arr, N, 0.0f)) {  // Check initialization
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    float arr[M];
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = i * 3.0f;
    }
    
    if (verify_array(arr, M, 0.0f)) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    float arr[N][M];
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    // Verify a sample
    int passed = 1;
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:N][0:M]) copy(passed)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (arr[i][j] != i * M + j) {
                passed = 0;
            }
        }
    }
    
    if (passed) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = sinf(i * 0.1f);
    }
    
    // Simple validation
    int passed = 1;
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N]) copy(passed)
    for (int i = 1; i < N; i++) {
        if (arr[i] > 1.0f || arr[i] < -1.0f) {
            passed = 0;
        }
    }
    
    if (passed) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop gang vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * i * 0.01f;
    }
    
    // Check monotonic increase
    int passed = 1;
    #pragma acc parallel loop gang vector copy(arr[0:N]) copy(passed)
    for (int i = 1; i < N; i++) {
        if (arr[i] <= arr[i-1]) {
            passed = 0;
        }
    }
    
    if (passed) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    float arr[N];
    
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = cosf(i * 0.05f);
    }
    
    // Range validation
    int passed = 1;
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) copy(arr[0:N]) copy(passed)
    for (int i = 0; i < N; i++) {
        if (arr[i] > 1.0f || arr[i] < -1.0f) {
            passed = 0;
        }
    }
    
    if (passed) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    float arr[N][M][P];
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    // Verify pattern
    int passed = 1;
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:N][0:M][0:P]) copy(passed)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                if (arr[i][j][k] != i * M * P + j * P + k) {
                    passed = 0;
                }
            }
        }
    }
    
    if (passed) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test default case: This would normally require compiler internals or invalid codes
   We'll create a complex scenario that might trigger edge cases */
void test_edge_cases() {
    printf("Testing edge cases (potential default path triggers)\n");
    
    /* Complex data mapping that might confuse partitioning analysis */
    float *ptr1, *ptr2;
    ptr1 = (float*)malloc(N * sizeof(float));
    ptr2 = (float*)malloc(N * sizeof(float));
    
    #pragma acc enter data create(ptr1[0:N], ptr2[0:N])
    
    /* Mixed directives with runtime parameters */
    int dyn_size = N;
    #pragma acc parallel loop gang vector copyin(dyn_size) present(ptr1[0:N], ptr2[0:N])
    for (int i = 0; i < dyn_size; i++) {
        ptr1[i] = i;
        ptr2[i] = N - i;
    }
    
    /* Triangular loop pattern */
    #pragma acc parallel loop gang
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < i; j++) {
            ptr1[i] += ptr2[j];
        }
    }
    
    #pragma acc exit data copyout(ptr1[0:N], ptr2[0:N]) delete(ptr1[0:N], ptr2[0:N])
    
    free(ptr1);
    free(ptr2);
    printf("  Edge cases executed (no verification)\n");
}

int main() {
    printf("Starting OpenACC partition mapping tests...\n\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_edge_cases();
    
    printf("\nAll partition tests completed.\n");
    printf("To verify coverage of lines 335-343 in omp-oacc-neuter-broadcast.cc:\n");
    printf("1. Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -fdump-tree-omplower\n");
    printf("2. Check compiler dumps for partition code generation\n");
    printf("3. Look for all 8 partition types (0-7) in the intermediate representation\n");
    
    return 0;
}
