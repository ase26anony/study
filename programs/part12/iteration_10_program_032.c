/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

/* Helper for verification */
static int verify_sum = 0;
static int test_count = 0;

/* Case 0: gang redundant - scalar reductions, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    
    int sum = 0;
    int arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
    }
    
    /* Scalar reduction - gang redundant */
    #pragma acc parallel loop reduction(+:sum) copyin(arr[0:N])
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    
    /* Verify */
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += arr[i];
    }
    
    if (sum == expected) {
        printf("  PASS: sum = %d (expected %d)\n", sum, expected);
        verify_sum += sum;
        test_count++;
    } else {
        printf("  FAIL: sum = %d (expected %d)\n", sum, expected);
    }
}

/* Case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    
    int arr[N];
    int result[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        result[i] = 0;
    }
    
    /* Gang partitioned loop */
    #pragma acc parallel loop gang copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] * 2;
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 2) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("  PASS: All %d elements correct\n", N);
        test_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
}

/* Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    
    int arr[M];
    int result[M];
    
    /* Initialize */
    for (int i = 0; i < M; i++) {
        arr[i] = i * 3;
        result[i] = 0;
    }
    
    /* Worker partitioned loop */
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M]) copyout(result[0:M])
    for (int i = 0; i < M; i++) {
        result[i] = arr[i] + 100;
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (result[i] != arr[i] + 100) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("  PASS: All %d elements correct\n", M);
        test_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
}

/* Case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    int arr[M][P];
    int result[M][P];
    
    /* Initialize 2D array */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr[i][j] = i * P + j;
            result[i][j] = 0;
        }
    }
    
    /* Gang+worker partitioned with collapse */
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:M][0:P]) copyout(result[0:M][0:P])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            result[i][j] = arr[i][j] * 3;
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            if (result[i][j] != arr[i][j] * 3) {
                errors++;
            }
        }
    }
    
    if (errors == 0) {
        printf("  PASS: All %d elements correct\n", M * P);
        test_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
}

/* Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    
    float arr[N];
    float result[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr[i] = i * 1.5f;
        result[i] = 0.0f;
    }
    
    /* Vector partitioned loop */
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] * 2.0f;
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 2.0f) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("  PASS: All %d elements correct\n", N);
        test_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
}

/* Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    
    int arr[N];
    int result[N];
    
    /* Initialize with stride-1 access pattern */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        result[i] = 0;
    }
    
    /* Gang+vector partitioned */
    #pragma acc parallel loop gang vector copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] + arr[i % 16];  /* Mixed access pattern */
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] + arr[i % 16]) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("  PASS: All %d elements correct\n", N);
        test_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
}

/* Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    float arr[M];
    float result[M];
    
    /* Initialize */
    for (int i = 0; i < M; i++) {
        arr[i] = i * 2.5f;
        result[i] = 0.0f;
    }
    
    /* Worker+vector partitioned */
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) \
        copy(arr[0:M]) copyout(result[0:M])
    for (int i = 0; i < M; i++) {
        result[i] = arr[i] / 2.0f;
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (result[i] != arr[i] / 2.0f) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("  PASS: All %d elements correct\n", M);
        test_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
}

/* Case 7: fully partitioned - gang, worker, and vector all used */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    int arr[M][P][8];
    int result[M][P][8];
    
    /* Initialize 3D array */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < 8; k++) {
                arr[i][j][k] = (i * 1000) + (j * 10) + k;
                result[i][j][k] = 0;
            }
        }
    }
    
    /* Fully partitioned with collapse(3) */
    #pragma acc parallel loop gang worker vector collapse(3) \
        copy(arr[0:M][0:P][0:8]) copyout(result[0:M][0:P][0:8])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < 8; k++) {
                result[i][j][k] = arr[i][j][k] * 2;
            }
        }
    }
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < 8; k++) {
                if (result[i][j][k] != arr[i][j][k] * 2) {
                    errors++;
                }
            }
        }
    }
    
    if (errors == 0) {
        printf("  PASS: All %d elements correct\n", M * P * 8);
        test_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
}

/* Helper to potentially trigger default case through compiler internals */
#ifdef TEST_DEFAULT_CASE
/* This would require compiler-specific hooks to test the default case */
void test_default_case() {
    printf("Testing default case (requires compiler internals)\n");
    /* In practice, this would use compiler debugging interfaces
     * to pass invalid partition codes to the mapping function */
}
#endif

int main() {
    printf("=== OpenACC Partition Coverage Test ===\n");
    printf("Aiming to trigger all 8 partition mapping cases\n\n");
    
    /* Run all test cases */
    test_gang_redundant();           /* Case 0 */
    test_gang_partitioned();         /* Case 1 */
    test_worker_partitioned();       /* Case 2 */
    test_gang_worker_partitioned();  /* Case 3 */
    test_vector_partitioned();       /* Case 4 */
    test_gang_vector_partitioned();  /* Case 5 */
    test_worker_vector_partitioned();/* Case 6 */
    test_fully_partitioned();        /* Case 7 */
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/8\n", test_count);
    
    if (test_count == 8) {
        printf("SUCCESS: All partition cases covered\n");
        return 0;
    } else {
        printf("FAILURE: Some partition cases not covered\n");
        return 1;
    }
}
