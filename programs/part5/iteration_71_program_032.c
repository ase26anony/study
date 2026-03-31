/* 
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * Targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 * Cases: 0-gang redundant, 1-gang partitioned, 2-worker partitioned,
 *        3-gang+worker partitioned, 4-vector partitioned,
 *        5-gang+vector partitioned, 6-worker+vector partitioned,
 *        7-fully partitioned
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function with explicit partition specification */
#pragma acc routine gang
void acc_routine_gang(int *arr, int idx, int val) {
    arr[idx] = val;
}

#pragma omp declare target
void omp_routine_gang(int *arr, int idx, int val) {
    arr[idx] = val;
}
#pragma omp end declare target

int main() {
    int i;
    int success_count = 0;
    int total_tests = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1;
    
    /* Test arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    int *results = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(2 * i);
        c[i] = 0.0f;
        results[i] = 0;
    }
    
    printf("Testing OpenACC/OpenMP partition clause variations...\n");
    
    /* ========== TEST 1: Gang Redundant (Case 0) ========== */
    total_tests++;
    printf("\nTest 1: Gang redundant\n");
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verify results */
    int correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 2: Gang Partitioned (Case 1) ========== */
    total_tests++;
    printf("\nTest 2: Gang partitioned\n");
    #pragma acc data copyin(a[0:N]) copy(results[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (i = 0; i < N; i++) {
            results[i] = (int)a[i];
            /* Call routine with gang partition */
            acc_routine_gang(results, i, (int)a[i]);
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (results[i] != (int)a[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 3: Worker Partitioned (Case 2) ========== */
    total_tests++;
    printf("\nTest 3: Worker partitioned\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) worker
        for (i = 0; i < N; i++) {
            c[i] = (float)i * 2.0f;
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != (float)i * 2.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 4: Gang+Worker Partitioned (Case 3) ========== */
    total_tests++;
    printf("\nTest 4: Gang+worker partitioned\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] * b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 5: Vector Partitioned (Case 4) ========== */
    total_tests++;
    printf("\nTest 5: Vector partitioned\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] / (b[i] + 1.0f)) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 6: Gang+Vector Partitioned (Case 5) ========== */
    total_tests++;
    printf("\nTest 6: Gang+vector partitioned\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] - b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 7: Worker+Vector Partitioned (Case 6) ========== */
    total_tests++;
    printf("\nTest 7: Worker+vector partitioned\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i] * 0.5f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 8: Fully Partitioned (Case 7) ========== */
    total_tests++;
    printf("\nTest 8: Fully partitioned\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length) gang worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f + b[i];
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] * 2.0f + b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 9: Conditional Partition Selection ========== */
    total_tests++;
    printf("\nTest 9: Conditional partition selection\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(flag ? 16 : 32)
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f;
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] * 3.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 10: Nested Parallelism ========== */
    total_tests++;
    printf("\nTest 10: Nested parallelism with partitioning\n");
    #pragma acc data copy(c[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc kernels gang(num_gangs)
        {
            /* Inner worker-partitioned loop */
            #pragma acc loop worker(num_workers) worker
            for (i = 0; i < N; i++) {
                c[i] = a[i] * a[i];
            }
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] * a[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 11: OpenMP Target Version ========== */
    total_tests++;
    printf("\nTest 11: OpenMP target with partition variations\n");
    
    /* Reset array */
    for (i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(num_gangs) num_threads(num_workers) \
            simdlen(vector_length)
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i] * 2.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== TEST 12: Device-specific behavior ========== */
    total_tests++;
    printf("\nTest 12: Device-specific partition behavior\n");
    
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] / 2.0f;
        }
    }
    
    correct = 1;
    for (i = 0; i < N; i++) {
        if (c[i] != a[i] / 2.0f) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ========== SUMMARY ========== */
    printf("\n========== TEST SUMMARY ==========\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", success_count);
    printf("Failed: %d\n", total_tests - success_count);
    
    if (success_count == total_tests) {
        printf("\nALL TESTS PASSED!\n");
    } else {
        printf("\nSOME TESTS FAILED!\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(results);
    
    return (success_count == total_tests) ? 0 : 1;
}
