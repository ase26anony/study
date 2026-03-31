/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting the uncovered switch statement in omp-oacc-neuter-broadcast.cc
 * lines 335-343 (partition type string lookup)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define NUM_TESTS 10

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
int verify_result(int *arr, int expected_value, int size, const char *test_name);

#pragma acc routine gang
void acc_gang_function(int *arr, int idx, int value);

#pragma acc routine worker
void acc_worker_function(int *arr, int idx, int value);

#pragma acc routine vector
void acc_vector_function(int *arr, int idx, int value);

/* Helper function to reset arrays */
void reset_array(int *arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value;
    }
}

int main() {
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    int success_count = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1;
    
    /* Initialize arrays */
    reset_array(a, N, 1);
    reset_array(b, N, 2);
    reset_array(c, N, 0);
    
    printf("Starting partition type coverage tests...\n\n");
    
    /* ===== TEST 1: Gang Redundant (case 0) ===== */
    printf("Test 1: Gang redundant\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    if (verify_result(c, 3, N, "Test 1")) success_count++;
    
    /* ===== TEST 2: Gang Partitioned (case 1) ===== */
    printf("\nTest 2: Gang partitioned\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    if (verify_result(c, 2, N, "Test 2")) success_count++;
    
    /* ===== TEST 3: Worker Partitioned (case 2) ===== */
    printf("\nTest 3: Worker partitioned\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    if (verify_result(c, -1, N, "Test 3")) success_count++;
    
    /* ===== TEST 4: Gang+Worker Partitioned (case 3) ===== */
    printf("\nTest 4: Gang+worker partitioned\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2;
        }
    }
    if (verify_result(c, 5, N, "Test 4")) success_count++;
    
    /* ===== TEST 5: Vector Partitioned (case 4) ===== */
    printf("\nTest 5: Vector partitioned\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop vector(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] - a[i];
        }
    }
    if (verify_result(c, 1, N, "Test 5")) success_count++;
    
    /* ===== TEST 6: Gang+Vector Partitioned (case 5) ===== */
    printf("\nTest 6: Gang+vector partitioned\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3 + b[i];
        }
    }
    if (verify_result(c, 5, N, "Test 6")) success_count++;
    
    /* ===== TEST 7: Worker+Vector Partitioned (case 6) ===== */
    printf("\nTest 7: Worker+vector partitioned\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] * 2 - a[i];
        }
    }
    if (verify_result(c, 3, N, "Test 7")) success_count++;
    
    /* ===== TEST 8: Fully Partitioned (case 7) ===== */
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] + i % 10;
        }
    }
    /* Complex verification for test 8 */
    int test8_success = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i] + i % 10) {
            test8_success = 0;
            break;
        }
    }
    if (test8_success) {
        printf("  Test 8 PASSED\n");
        success_count++;
    } else {
        printf("  Test 8 FAILED\n");
    }
    
    /* ===== TEST 9: Nested Parallelism with Partitioning ===== */
    printf("\nTest 9: Nested parallelism (outer gang-redundant, inner worker-partitioned)\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc kernels gang
        {
            #pragma acc loop worker(num_workers) worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * b[i] * 2;
            }
        }
    }
    if (verify_result(c, 4, N, "Test 9")) success_count++;
    
    /* ===== TEST 10: Conditional Partition Selection ===== */
    printf("\nTest 10: Conditional partition selection\n");
    reset_array(c, N, 0);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector(flag ? 16 : 32) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 4 + b[i] * 2;
        }
    }
    if (verify_result(c, 8, N, "Test 10")) success_count++;
    
    /* ===== OpenMP Target Tests ===== */
    printf("\n=== OpenMP Target Tests ===\n");
    
    /* OpenMP Test 1: Gang partitioned */
    printf("\nOpenMP Test 1: Gang partitioned\n");
    reset_array(c, N, 0);
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] + 10;
        }
    }
    if (verify_result(c, 13, N, "OpenMP Test 1")) success_count++;
    
    /* OpenMP Test 2: Worker partitioned */
    printf("\nOpenMP Test 2: Worker partitioned\n");
    reset_array(c, N, 0);
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(2) thread_limit(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i] * 3;
        }
    }
    if (verify_result(c, 6, N, "OpenMP Test 2")) success_count++;
    
    /* OpenMP Test 3: Vector partitioned */
    printf("\nOpenMP Test 3: Vector partitioned\n");
    reset_array(c, N, 0);
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for simd simdlen(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] - a[i] + 5;
        }
    }
    if (verify_result(c, 6, N, "OpenMP Test 3")) success_count++;
    
    /* ===== Device-Specific Behaviors ===== */
    printf("\n=== Device-Specific Tests ===\n");
    
    #ifdef _OPENACC
    printf("\nOpenACC device_type test\n");
    reset_array(c, N, 0);
    #pragma acc set device_type(nvidia)
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 10 + b[i];
        }
    }
    if (verify_result(c, 12, N, "Device-specific test")) success_count++;
    #endif
    
    /* ===== Summary ===== */
    printf("\n=== Test Summary ===\n");
    printf("Total tests attempted: %d\n", NUM_TESTS + 3); /* 10 ACC + 3 OMP */
    printf("Tests passed: %d\n", success_count);
    printf("Success rate: %.1f%%\n", (success_count * 100.0) / (NUM_TESTS + 3));
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return (success_count == (NUM_TESTS + 3)) ? 0 : 1;
}

/* Verification function */
int verify_result(int *arr, int expected_value, int size, const char *test_name) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected_value) {
            printf("  %s FAILED at index %d: got %d, expected %d\n", 
                   test_name, i, arr[i], expected_value);
            return 0;
        }
    }
    printf("  %s PASSED\n", test_name);
    return 1;
}

/* Routine functions to force partition analysis */
void acc_gang_function(int *arr, int idx, int value) {
    arr[idx] += value;
}

void acc_worker_function(int *arr, int idx, int value) {
    arr[idx] *= value;
}

void acc_vector_function(int *arr, int idx, int value) {
    arr[idx] -= value;
}
