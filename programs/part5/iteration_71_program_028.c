/* 
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * Targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function prototypes with OpenACC routine directives */
#pragma acc routine seq
void verify_result(const char* test_name, int* host_arr, int* expected_arr, int size);
#pragma acc routine gang
void device_function_gang(int* arr, int idx, int val);

/* Global variables to force runtime evaluation */
int num_gangs_var = 2;
int num_workers_var = 4;
int vector_len_var = 32;
int conditional_flag = 1;

int main() {
    int *a, *b, *c, *d, *e;
    int i;
    int success_count = 0;
    int total_tests = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    e = (int*)malloc(N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
        d[i] = 0;
        e[i] = 0;
    }
    
    printf("=== Testing OpenACC/OpenMP Partition Clause Variations ===\n\n");
    
    /* ====================================================================
     * TEST 1: Gang Redundant (case 0)
     * Outer region with gang redundancy, inner with explicit gang partitioning
     * ==================================================================== */
    {
        total_tests++;
        printf("Test 1: Gang Redundant Partition\n");
        
        #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
        {
            /* Outer gang-redundant region */
            #pragma acc kernels gang(num_gangs_var)
            {
                /* Inner gang-partitioned loop - should trigger case 0 */
                #pragma acc loop gang
                for (i = 0; i < N; i++) {
                    c[i] = a[i] + b[i];
                }
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
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 2: Gang Partitioned (case 1)
     * Explicit gang partitioning with runtime-determined gang count
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 2: Gang Partitioned\n");
        
        int local_gangs = num_gangs_var;
        
        #pragma acc parallel loop gang(num_gangs(local_gangs)) copyin(a[0:N], b[0:N]) copyout(d[0:N])
        for (i = 0; i < N; i++) {
            d[i] = a[i] * 2 + b[i];
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (d[i] != a[i] * 2 + b[i]) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 3: Worker Partitioned (case 2)
     * Worker-only partitioning with conditional worker count
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 3: Worker Partitioned\n");
        
        #pragma acc data copyin(a[0:N]) copyout(c[0:N])
        {
            #pragma acc parallel num_workers(num_workers_var)
            {
                #pragma acc loop worker
                for (i = 0; i < N; i++) {
                    c[i] = a[i] * 3;
                }
            }
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (c[i] != a[i] * 3) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 4: Gang+Worker Partitioned (case 3)
     * Combined gang and worker partitioning
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 4: Gang+Worker Partitioned\n");
        
        #pragma acc parallel loop gang(num_gangs_var) worker(num_workers_var) \
                    copyin(a[0:N], b[0:N]) copyout(d[0:N])
        for (i = 0; i < N; i++) {
            d[i] = a[i] - b[i];
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (d[i] != a[i] - b[i]) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 5: Vector Partitioned (case 4)
     * Vector-only partitioning with runtime vector length
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 5: Vector Partitioned\n");
        
        int vec_len = vector_len_var;
        
        #pragma acc parallel loop vector_length(vec_len) copyin(a[0:N]) copyout(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] + 100;
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (c[i] != a[i] + 100) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 6: Gang+Vector Partitioned (case 5)
     * Combined gang and vector partitioning
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 6: Gang+Vector Partitioned\n");
        
        #pragma acc parallel loop gang(num_gangs_var) vector_length(vector_len_var) \
                    copyin(a[0:N], b[0:N]) copyout(d[0:N])
        for (i = 0; i < N; i++) {
            d[i] = a[i] * b[i];
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (d[i] != a[i] * b[i]) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 7: Worker+Vector Partitioned (case 6)
     * Combined worker and vector partitioning
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 7: Worker+Vector Partitioned\n");
        
        #pragma acc parallel loop worker(num_workers_var) vector_length(vector_len_var) \
                    copyin(a[0:N]) copyout(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] / 2;
        }
        
        /* Verify (integer division) */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (c[i] != a[i] / 2) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 8: Fully Partitioned (case 7)
     * Gang+Worker+Vector partitioning
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 8: Fully Partitioned (Gang+Worker+Vector)\n");
        
        #pragma acc parallel loop gang(num_gangs_var) worker(num_workers_var) \
                    vector_length(vector_len_var) copyin(a[0:N], b[0:N]) copyout(e[0:N])
        for (i = 0; i < N; i++) {
            e[i] = a[i] + b[i] * 2;
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (e[i] != a[i] + b[i] * 2) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 9: Conditional Partition Selection
     * Runtime conditional to select different partition configurations
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 9: Conditional Partition Selection\n");
        
        int workers = conditional_flag ? 2 : 4;
        int vectors = !conditional_flag ? 16 : 32;
        
        #pragma acc parallel loop gang(num_gangs_var) worker(workers) \
                    vector_length(vectors) copyin(a[0:N]) copyout(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 5;
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (c[i] != a[i] * 5) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 10: Nested Parallelism with Mixed Partitioning
     * Outer gang-redundant, inner worker-partitioned
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 10: Nested Parallelism with Mixed Partitioning\n");
        
        #pragma acc data copyin(a[0:N], b[0:N]) copyout(d[0:N])
        {
            /* Outer region - gang redundant */
            #pragma acc kernels gang(num_gangs_var)
            {
                /* Inner region - worker partitioned */
                #pragma acc loop worker(num_workers_var)
                for (i = 0; i < N; i++) {
                    d[i] = a[i] * 10 + b[i];
                }
            }
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (d[i] != a[i] * 10 + b[i]) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 11: OpenMP Target Version - Gang Partitioned
     * Using OpenMP target for comparison
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 11: OpenMP Target - Gang Partitioned\n");
        
        #pragma omp target teams distribute parallel for \
                    num_teams(num_gangs_var) map(to: a[0:N], b[0:N]) map(from: c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] + 50;
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (c[i] != a[i] + b[i] + 50) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 12: OpenMP Target - Fully Partitioned
     * Teams (gangs) + threads (workers/vectors)
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 12: OpenMP Target - Fully Partitioned\n");
        
        #pragma omp target teams distribute parallel for simd \
                    num_teams(num_gangs_var) thread_limit(num_workers_var * vector_len_var) \
                    map(to: a[0:N]) map(from: d[0:N])
        for (i = 0; i < N; i++) {
            d[i] = a[i] * 3 + 7;
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (d[i] != a[i] * 3 + 7) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 13: Device-specific behavior with set device_type
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 13: Device-specific Partitioning\n");
        
        #pragma acc set device_type(nvidia)
        
        #pragma acc parallel loop gang(num_gangs_var) vector_length(64) \
                    copyin(a[0:N]) copyout(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 2 + 25;
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (c[i] != a[i] * 2 + 25) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * TEST 14: Function call with gang routine directive
     * Exercises partition analysis for function calls
     * ==================================================================== */
    {
        total_tests++;
        printf("\nTest 14: Function Call with Gang Routine\n");
        
        #pragma acc data copyin(a[0:N]) copyout(e[0:N])
        {
            #pragma acc parallel loop gang(num_gangs_var)
            for (i = 0; i < N; i++) {
                device_function_gang(e, i, a[i] * 4);
            }
        }
        
        /* Verify */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (e[i] != a[i] * 4) {
                correct = 0;
                break;
            }
        }
        if (correct) {
            success_count++;
            printf("  PASSED\n");
        } else {
            printf("  FAILED\n");
        }
    }
    
    /* ====================================================================
     * Cleanup and summary
     * ==================================================================== */
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests run: %d\n", total_tests);
    printf("Tests passed: %d\n", success_count);
    printf("Tests failed: %d\n", total_tests - success_count);
    
    if (success_count == total_tests) {
        printf("\nALL TESTS PASSED!\n");
        return 0;
    } else {
        printf("\nSOME TESTS FAILED!\n");
        return 1;
    }
}

/* Implementation of device function with gang routine */
void device_function_gang(int* arr, int idx, int val) {
    arr[idx] = val;
}

/* Verification helper function */
void verify_result(const char* test_name, int* host_arr, int* expected_arr, int size) {
    int correct = 1;
    for (int i = 0; i < size; i++) {
        if (host_arr[i] != expected_arr[i]) {
            correct = 0;
            printf("  Mismatch at index %d: got %d, expected %d\n", 
                   i, host_arr[i], expected_arr[i]);
            break;
        }
    }
    if (!correct) {
        printf("  %s: FAILED\n", test_name);
    }
}
