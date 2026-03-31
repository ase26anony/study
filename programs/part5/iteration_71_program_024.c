/*
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * Targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define NUM_TESTS 10

/* Function with gang partition for OpenACC routine */
#pragma acc routine gang
void acc_gang_routine(int *arr, int idx, int val) {
    arr[idx] = val;
}

/* Function with vector partition for OpenACC routine */
#pragma acc routine vector
void acc_vector_routine(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}

int main() {
    int *a, *b, *c, *d;
    int num_gangs = 2;
    int num_workers = 4;
    int vector_len = 32;
    int flag = 1;
    int test_passed = 0;
    int total_tests = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
        d[i] = 0;
    }
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* Test 1: Gang redundant (case 0) - Outer kernels with inner gang loop */
    printf("Test 1: Gang redundant\n");
    total_tests++;
    #pragma acc kernels copy(a[0:N], c[0:N])
    {
        #pragma acc loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + 1;
        }
    }
    
    /* Verify results */
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + 1) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 2: Gang partitioned (case 1) */
    printf("\nTest 2: Gang partitioned\n");
    total_tests++;
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(num_gangs) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 3: Worker partitioned (case 2) */
    printf("\nTest 3: Worker partitioned\n");
    total_tests++;
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
        num_workers(num_workers) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 2) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 4: Gang+worker partitioned (case 3) */
    printf("\nTest 4: Gang+worker partitioned\n");
    total_tests++;
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(num_gangs) num_workers(num_workers) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] - b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 5: Vector partitioned (case 4) */
    printf("\nTest 5: Vector partitioned\n");
    total_tests++;
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
        vector_length(vector_len) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / 2;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] / 2) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 6: Gang+vector partitioned (case 5) */
    printf("\nTest 6: Gang+vector partitioned\n");
    total_tests++;
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(num_gangs) vector_length(vector_len) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * b[i]) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 7: Worker+vector partitioned (case 6) */
    printf("\nTest 7: Worker+vector partitioned\n");
    total_tests++;
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
        num_workers(num_workers) vector_length(vector_len) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + 100;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + 100) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 8: Fully partitioned (case 7) */
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    total_tests++;
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(num_gangs) num_workers(num_workers) \
        vector_length(vector_len) gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = (a[i] + b[i]) * 3;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != (a[i] + b[i]) * 3) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 9: Conditional partition selection with ternary operator */
    printf("\nTest 9: Conditional partition selection\n");
    total_tests++;
    int dynamic_workers = flag ? 2 : 4;
    int dynamic_vector = flag ? 16 : 32;
    
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
        num_gangs(num_gangs) \
        num_workers(dynamic_workers) \
        vector_length(dynamic_vector) \
        gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 5;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 5) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 10: Nested parallelism with data clauses */
    printf("\nTest 10: Nested parallelism with data clauses\n");
    total_tests++;
    #pragma acc data copy(a[0:N], d[0:N])
    {
        /* Outer region */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            /* Inner worker-partitioned loop */
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                d[i] = a[i] * 10;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] * 10) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 11: OpenMP target equivalent for gang partitioned */
    printf("\nTest 11: OpenMP target - gang partitioned\n");
    total_tests++;
    #pragma omp target teams distribute parallel for \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        num_teams(num_gangs) thread_limit(256)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] + 50;
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i] + 50) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 12: Device-specific behavior simulation */
    printf("\nTest 12: Device-specific partition behavior\n");
    total_tests++;
    #pragma acc set device_type(nvidia)
    
    #pragma acc parallel copyin(a[0:N]) copyout(c[0:N]) \
        num_gangs(num_gangs) num_workers(num_workers) \
        vector_length(vector_len) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            acc_gang_routine(c, i, a[i] + 20);
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + 20) {
            correct = 0;
            break;
        }
    }
    if (correct) {
        test_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    
    /* Summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests run: %d\n", total_tests);
    printf("  Tests passed:    %d\n", test_passed);
    printf("  Tests failed:    %d\n", total_tests - test_passed);
    printf("  Success rate:    %.1f%%\n", 
           (float)test_passed / total_tests * 100);
    
    if (test_passed == total_tests) {
        printf("\nALL TESTS PASSED!\n");
    } else {
        printf("\nSOME TESTS FAILED!\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (test_passed == total_tests) ? 0 : 1;
}
