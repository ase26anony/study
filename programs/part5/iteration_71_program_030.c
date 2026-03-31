/* 
 * Test program to exercise OpenACC/OpenMP partition clause logic
 * Specifically targets the switch statement in omp-oacc-neuter-broadcast.cc
 * covering cases 0-7 for different partition types
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function declarations with partition attributes */
#pragma acc routine gang
void acc_gang_function(int *arr, int start, int end);

#pragma acc routine worker
void acc_worker_function(int *arr, int idx);

#pragma acc routine vector
void acc_vector_function(int *arr, int idx);

/* Helper to verify results */
int verify_results(int *a, int *b, int size, const char *test_name) {
    for (int i = 0; i < size; i++) {
        if (a[i] != b[i]) {
            printf("Test %s failed at index %d: %d != %d\n", 
                   test_name, i, a[i], b[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    int passed_tests = 0;
    int total_tests = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1;
    
    /* Test arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *expected = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = (i + 1) % 100;
        expected[i] = a[i] + b[i];
    }
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* =========================================== */
    /* Test 1: Gang redundant (case 0) */
    /* =========================================== */
    total_tests++;
    printf("Test 1: Gang redundant\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            /* Inner loop - gang redundant */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    if (verify_results(c, expected, N, "Gang redundant")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test 2: Gang partitioned (case 1) */
    /* =========================================== */
    total_tests++;
    printf("\nTest 2: Gang partitioned\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Explicit gang partitioning */
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Update expected for multiplication */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] * b[i];
    }
    
    if (verify_results(c, expected, N, "Gang partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test 3: Worker partitioned (case 2) */
    /* =========================================== */
    total_tests++;
    printf("\nTest 3: Worker partitioned\n");
    
    /* Reset expected for addition */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i];
    }
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Worker partitioning with conditional */
        #pragma acc parallel num_gangs(num_gangs) num_workers(flag ? 2 : 4)
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    if (verify_results(c, expected, N, "Worker partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test 4: Gang+worker partitioned (case 3) */
    /* =========================================== */
    total_tests++;
    printf("\nTest 4: Gang+worker partitioned\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Nested gang and worker partitioning */
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] - b[i];
            }
        }
    }
    
    /* Update expected for subtraction */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] - b[i];
    }
    
    if (verify_results(c, expected, N, "Gang+worker partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test 5: Vector partitioned (case 4) */
    /* =========================================== */
    total_tests++;
    printf("\nTest 5: Vector partitioned\n");
    
    /* Reset expected for addition */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i];
    }
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Vector partitioning */
        #pragma acc parallel vector_length(vector_length)
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    if (verify_results(c, expected, N, "Vector partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test 6: Gang+vector partitioned (case 5) */
    /* =========================================== */
    total_tests++;
    printf("\nTest 6: Gang+vector partitioned\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Combined gang and vector partitioning */
        #pragma acc parallel num_gangs(num_gangs) vector_length(vector_length)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 2;
            }
        }
    }
    
    /* Update expected */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i] * 2;
    }
    
    if (verify_results(c, expected, N, "Gang+vector partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test 7: Worker+vector partitioned (case 6) */
    /* =========================================== */
    total_tests++;
    printf("\nTest 7: Worker+vector partitioned\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Worker and vector combined */
        #pragma acc parallel num_workers(num_workers) vector_length(vector_length)
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 3 + b[i];
            }
        }
    }
    
    /* Update expected */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] * 3 + b[i];
    }
    
    if (verify_results(c, expected, N, "Worker+vector partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test 8: Fully partitioned (case 7) */
    /* =========================================== */
    total_tests++;
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Full three-level partitioning */
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_length)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] + i;
            }
        }
    }
    
    /* Update expected */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i] + i;
    }
    
    if (verify_results(c, expected, N, "Fully partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* OpenMP Target equivalents */
    /* =========================================== */
    printf("\n=== OpenMP Target Tests ===\n");
    
    /* Test 9: OpenMP with gang partitioning */
    total_tests++;
    printf("\nTest 9: OpenMP target teams (gang partitioned)\n");
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_gangs) thread_limit(256)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Reset expected for addition */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i];
    }
    
    if (verify_results(c, expected, N, "OpenMP gang partitioned")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* Test 10: OpenMP with nested parallelism */
    total_tests++;
    printf("\nTest 10: OpenMP nested parallelism\n");
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for simd \
            num_teams(num_gangs) thread_limit(256) simdlen(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Update expected for subtraction */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] - b[i];
    }
    
    if (verify_results(c, expected, N, "OpenMP nested")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Test with device-specific behavior */
    /* =========================================== */
    total_tests++;
    printf("\nTest 11: Device-specific partitioning\n");
    
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Conditional partitioning based on runtime flag */
        int dynamic_workers = flag ? 2 : 8;
        #pragma acc parallel num_gangs(num_gangs) num_workers(dynamic_workers) vector_length(vector_length)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * b[i] / 2;
            }
        }
    }
    
    /* Update expected */
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] * b[i] / 2;
    }
    
    if (verify_results(c, expected, N, "Device-specific")) {
        passed_tests++;
        printf("  PASSED\n");
    }
    
    /* =========================================== */
    /* Summary */
    /* =========================================== */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Passed: %d/%d\n", passed_tests, total_tests);
    printf("  Coverage: %.1f%%\n", (passed_tests * 100.0) / total_tests);
    
    if (passed_tests == total_tests) {
        printf("\nSUCCESS: All tests passed!\n");
    } else {
        printf("\nWARNING: Some tests failed\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(expected);
    
    return (passed_tests == total_tests) ? 0 : 1;
}

/* Partition-attributed function implementations */
void acc_gang_function(int *arr, int start, int end) {
    #pragma acc loop gang
    for (int i = start; i < end; i++) {
        arr[i] *= 2;
    }
}

void acc_worker_function(int *arr, int idx) {
    #pragma acc loop worker
    for (int i = 0; i < 16; i++) {
        arr[idx + i] += i;
    }
}

void acc_vector_function(int *arr, int idx) {
    #pragma acc loop vector
    for (int i = 0; i < vector_length; i++) {
        arr[idx + i] -= 1;
    }
}
