/**
 * Test program to exercise all partition type cases in omp-oacc-neuter-broadcast.cc
 * Specifically targets the switch statement at lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Helper function to verify array results */
int verify_results(float* a, float* b, int size, float expected_value) {
    for (int i = 0; i < size; i++) {
        if (a[i] != expected_value || b[i] != expected_value) {
            return 0;
        }
    }
    return 1;
}

/* Function with explicit partition specification for OpenACC */
#pragma acc routine gang
void acc_gang_function(float* arr, int n, float val) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] = val;
    }
}

/* Function with worker partition for nested testing */
#pragma acc routine worker
void acc_worker_function(float* arr, int n, float val) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = val;
    }
}

int main() {
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1;
    int success_count = 0;
    int total_tests = 0;
    
    /* Test arrays */
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    float* c = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = 0.0f;
        b[i] = 0.0f;
        c[i] = 1.0f;
    }
    
    printf("Starting partition type coverage tests...\n\n");
    
    /*******************************************************************
     * TEST 1: Gang Redundant (case 0)
     * Outer region with gang-redundant partitioning
     *******************************************************************/
    printf("Test 1: Gang Redundant\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            /* This should trigger gang redundant case */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                a[i] = 1.0f;
            }
        }
    }
    
    if (verify_results(a, a, N, 1.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 2: Gang Partitioned (case 1)
     * Explicit gang partitioning with runtime variable
     *******************************************************************/
    printf("\nTest 2: Gang Partitioned\n");
    total_tests++;
    
    #pragma acc data copy(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) copyin(c[0:N])
        for (int i = 0; i < N; i++) {
            b[i] = c[i] * 2.0f;
        }
    }
    
    if (verify_results(b, b, N, 2.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 3: Worker Partitioned (case 2)
     * Worker-only partitioning within gang-redundant region
     *******************************************************************/
    printf("\nTest 3: Worker Partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N])
    {
        #pragma acc parallel num_gangs(1) num_workers(num_workers) gang
        {
            /* Outer gang-redundant, inner worker-partitioned */
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                a[i] = 3.0f;
            }
        }
    }
    
    if (verify_results(a, a, N, 3.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 4: Gang+Worker Partitioned (case 3)
     * Combined gang and worker partitioning
     *******************************************************************/
    printf("\nTest 4: Gang+Worker Partitioned\n");
    total_tests++;
    
    #pragma acc data copy(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            b[i] = 4.0f;
        }
    }
    
    if (verify_results(b, b, N, 4.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 5: Vector Partitioned (case 4)
     * Vector-only partitioning
     *******************************************************************/
    printf("\nTest 5: Vector Partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N])
    {
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(vector_length)
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                a[i] = 5.0f;
            }
        }
    }
    
    if (verify_results(a, a, N, 5.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 6: Gang+Vector Partitioned (case 5)
     * Combined gang and vector partitioning
     *******************************************************************/
    printf("\nTest 6: Gang+Vector Partitioned\n");
    total_tests++;
    
    #pragma acc data copy(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            b[i] = 6.0f;
        }
    }
    
    if (verify_results(b, b, N, 6.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 7: Worker+Vector Partitioned (case 6)
     * Combined worker and vector partitioning
     *******************************************************************/
    printf("\nTest 7: Worker+Vector Partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N])
    {
        #pragma acc parallel num_gangs(1) num_workers(num_workers) vector_length(vector_length)
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                a[i] = 7.0f;
            }
        }
    }
    
    if (verify_results(a, a, N, 7.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 8: Fully Partitioned (case 7)
     * Gang, worker, and vector all partitioned
     *******************************************************************/
    printf("\nTest 8: Fully Partitioned\n");
    total_tests++;
    
    #pragma acc data copy(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            b[i] = 8.0f;
        }
    }
    
    if (verify_results(b, b, N, 8.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 9: Conditional Partition Selection
     * Runtime selection of partition counts
     *******************************************************************/
    printf("\nTest 9: Conditional Partition Selection\n");
    total_tests++;
    
    int dynamic_gangs = flag ? 2 : 4;
    int dynamic_workers = flag ? 4 : 2;
    
    #pragma acc data copy(a[0:N])
    {
        #pragma acc parallel loop gang(dynamic_gangs) worker(dynamic_workers) vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            a[i] = 9.0f;
        }
    }
    
    if (verify_results(a, a, N, 9.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 10: Nested Parallelism with Partitioning
     * Outer kernels with inner partitioned loops
     *******************************************************************/
    printf("\nTest 10: Nested Parallelism with Partitioning\n");
    total_tests++;
    
    #pragma acc data copy(b[0:N])
    {
        #pragma acc kernels
        {
            /* Outer gang-redundant region */
            #pragma acc loop gang
            for (int i = 0; i < N/vector_length; i++) {
                /* Inner worker-partitioned loop */
                #pragma acc loop worker
                for (int j = 0; j < vector_length; j++) {
                    int idx = i * vector_length + j;
                    b[idx] = 10.0f;
                }
            }
        }
    }
    
    if (verify_results(b, b, N, 10.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 11: OpenMP Target Equivalent Tests
     * Using OpenMP to trigger the same partition logic
     *******************************************************************/
    printf("\nTest 11: OpenMP Target Tests\n");
    total_tests++;
    
    #pragma omp target data map(tofrom: a[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers)
        for (int i = 0; i < N; i++) {
            a[i] = 11.0f;
        }
    }
    
    if (verify_results(a, a, N, 11.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * TEST 12: Device-Specific Partition Behavior
     * Using device_type to potentially trigger different paths
     *******************************************************************/
    printf("\nTest 12: Device-Specific Behavior\n");
    total_tests++;
    
    #pragma acc data copy(b[0:N])
    {
        #pragma acc set device_type(nvidia)
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            b[i] = 12.0f;
        }
    }
    
    if (verify_results(b, b, N, 12.0f)) {
        printf("  ✓ PASS\n");
        success_count++;
    } else {
        printf("  ✗ FAIL\n");
    }
    
    /*******************************************************************
     * Cleanup and Summary
     *******************************************************************/
    free(a);
    free(b);
    free(c);
    
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", success_count);
    printf("  Failed: %d\n", total_tests - success_count);
    printf("  Success rate: %.1f%%\n", 
           (float)success_count / total_tests * 100.0f);
    
    if (success_count == total_tests) {
        printf("\n✅ All tests passed! All partition types should be covered.\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed. Check implementation.\n");
        return 1;
    }
}
