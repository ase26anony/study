/**
 * Test program to exercise all partition type cases in OpenACC/OpenMP runtime
 * Specifically targets the switch statement returning partition type strings:
 * 0: gang redundant, 1: gang partitioned, 2: worker partitioned,
 * 3: gang+worker partitioned, 4: vector partitioned, 5: gang+vector partitioned,
 * 6: worker+vector partitioned, 7: fully partitioned
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

#ifdef _OPENACC
#define USE_OPENACC 1
#else
#define USE_OPENACC 0
#endif

/* Function with explicit partition specification */
#pragma acc routine gang
void acc_routine_gang(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}

#pragma acc routine worker
void acc_routine_worker(int *arr, int idx, int val) {
    arr[idx] = val + 1;
}

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(float *a, float *b, float *c) {
    int num_gangs = 2;
    int num_workers = 1;
    int vector_len = 1;
    
    printf("Testing: gang redundant\n");
    
#if USE_OPENACC
    #pragma acc parallel loop gang(num_gangs) copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
#else
    #pragma omp target teams distribute parallel for \
        num_teams(num_gangs) thread_limit(num_workers*vector_len) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
#endif
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(float *a, float *b, float *c) {
    int num_gangs = 4;
    int gang_partition = 2; /* Explicit gang partitioning */
    
    printf("Testing: gang partitioned\n");
    
#if USE_OPENACC
    #pragma acc parallel loop gang(num_gangs) copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i];
    }
#else
    #pragma omp target teams distribute parallel for \
        num_teams(num_gangs) map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i];
    }
#endif
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(float *a, float *b, float *c) {
    int num_workers = 4;
    int vector_len = 1;
    
    printf("Testing: worker partitioned\n");
    
#if USE_OPENACC
    #pragma acc parallel loop worker(num_workers) vector(vector_len) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] - b[i];
    }
#else
    #pragma omp target teams distribute parallel for \
        num_teams(1) num_threads(num_workers) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] - b[i];
    }
#endif
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(float *a, float *b, float *c) {
    int num_gangs = 2;
    int num_workers = 4;
    int vector_len = 1;
    
    printf("Testing: gang+worker partitioned\n");
    
#if USE_OPENACC
    #pragma acc parallel loop gang(num_gangs) worker(num_workers) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] / (b[i] + 1.0f);
    }
#else
    #pragma omp target teams distribute parallel for \
        num_teams(num_gangs) num_threads(num_workers) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] / (b[i] + 1.0f);
    }
#endif
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(float *a, float *b, float *c) {
    int vector_len = 32;
    
    printf("Testing: vector partitioned\n");
    
#if USE_OPENACC
    #pragma acc parallel loop vector(vector_len) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 2.0f + b[i];
    }
#else
    #pragma omp target teams distribute parallel for simd \
        simdlen(vector_len) map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 2.0f + b[i];
    }
#endif
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(float *a, float *b, float *c) {
    int num_gangs = 2;
    int vector_len = 16;
    int use_long_vector = 1; /* Runtime condition */
    
    printf("Testing: gang+vector partitioned\n");
    
#if USE_OPENACC
    #pragma acc parallel loop gang(num_gangs) vector(use_long_vector ? 32 : vector_len) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * a[i] + b[i] * b[i];
    }
#else
    #pragma omp target teams distribute parallel for simd \
        num_teams(num_gangs) simdlen(vector_len) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * a[i] + b[i] * b[i];
    }
#endif
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(float *a, float *b, float *c) {
    int num_workers = 4;
    int vector_len = 8;
    
    printf("Testing: worker+vector partitioned\n");
    
#if USE_OPENACC
    #pragma acc parallel loop worker(num_workers) vector(vector_len) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] + b[i]) * 0.5f;
    }
#else
    #pragma omp target parallel for simd \
        num_threads(num_workers) simdlen(vector_len) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] + b[i]) * 0.5f;
    }
#endif
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(float *a, float *b, float *c) {
    int num_gangs = 2;
    int num_workers = 2;
    int vector_len = 8;
    
    printf("Testing: fully partitioned\n");
    
#if USE_OPENACC
    #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_len) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i] * 0.5f;
    }
#else
    #pragma omp target teams distribute parallel for simd \
        num_teams(num_gangs) num_threads(num_workers) simdlen(vector_len) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i] * 0.5f;
    }
#endif
}

/* Test 9: Nested parallelism with different partition types */
void test_nested_partitioning(int *arr1, int *arr2) {
    printf("Testing: nested partitioning\n");
    
#if USE_OPENACC
    /* Outer gang-redundant region */
    #pragma acc kernels copy(arr1[0:N]) copyout(arr2[0:N])
    {
        /* Inner worker-partitioned loop */
        #pragma acc loop gang
        for (int i = 0; i < N/32; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < N) {
                    arr2[idx] = arr1[idx] * 3;
                }
            }
        }
    }
#else
    #pragma omp target map(to: arr1[0:N]) map(from: arr2[0:N])
    {
        #pragma omp teams distribute
        for (int i = 0; i < N/32; i++) {
            #pragma omp parallel for
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < N) {
                    arr2[idx] = arr1[idx] * 3;
                }
            }
        }
    }
#endif
}

/* Test 10: Conditional partition selection */
void test_conditional_partition(float *a, float *b, float *c, int flag) {
    printf("Testing: conditional partition selection\n");
    
    int num_gangs = flag ? 2 : 4;
    int num_workers = flag ? 4 : 2;
    int vector_len = flag ? 16 : 32;
    
#if USE_OPENACC
    #pragma acc parallel loop \
        gang(num_gangs) \
        worker(num_workers) \
        vector(vector_len) \
        copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = flag ? (a[i] + b[i]) : (a[i] - b[i]);
    }
#else
    #pragma omp target teams distribute parallel for simd \
        num_teams(num_gangs) num_threads(num_workers) simdlen(vector_len) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = flag ? (a[i] + b[i]) : (a[i] - b[i]);
    }
#endif
}

/* Test 11: Data clauses with partitioned arrays */
void test_data_partition_clauses() {
    printf("Testing: data clauses with partitioned arrays\n");
    
    float *arr = (float*)malloc(N * sizeof(float));
    float *result = (float*)malloc(N * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        result[i] = 0.0f;
    }
    
#if USE_OPENACC
    #pragma acc data copy(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0f;
        }
    }
#else
    #pragma omp target data map(to: arr[0:N]) map(from: result[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0f;
        }
    }
#endif
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(result[i] - (float)(i * 2)) > VERIFY_TOLERANCE) {
            errors++;
        }
    }
    
    if (errors > 0) {
        printf("  Data partition test: %d errors\n", errors);
    }
    
    free(arr);
    free(result);
}

/* Test 12: Multiple target architectures simulation */
void test_device_specific_partition() {
    printf("Testing: device-specific partition behaviors\n");
    
    int *data = (int*)malloc(N * sizeof(int));
    int *result = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        data[i] = i;
        result[i] = 0;
    }
    
#if USE_OPENACC
    /* Simulate device-specific behavior */
    #pragma acc set device_type(nvidia)
    
    #pragma acc parallel loop gang worker vector \
        copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] + 100;
    }
#else
    #pragma omp requires unified_shared_memory
    
    #pragma omp target teams distribute parallel for \
        map(to: data[0:N]) map(from: result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] + 100;
    }
#endif
    
    /* Verify */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != i + 100) {
            errors++;
        }
    }
    
    if (errors > 0) {
        printf("  Device-specific test: %d errors\n", errors);
    }
    
    free(data);
    free(result);
}

/* Verification helper */
int verify_results(float *a, float *b, float *c, int test_id) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected;
        switch (test_id) {
            case 1: expected = a[i] + b[i]; break;
            case 2: expected = a[i] * b[i]; break;
            case 3: expected = a[i] - b[i]; break;
            case 4: expected = a[i] / (b[i] + 1.0f); break;
            case 5: expected = a[i] * 2.0f + b[i]; break;
            case 6: expected = a[i] * a[i] + b[i] * b[i]; break;
            case 7: expected = (a[i] + b[i]) * 0.5f; break;
            case 8: expected = a[i] * b[i] * 0.5f; break;
            default: expected = 0.0f;
        }
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at i=%d: got %f, expected %f\n", i, c[i], expected);
            }
        }
    }
    return errors;
}

int main() {
    printf("Starting partition type coverage tests...\n");
    
    /* Allocate and initialize test data */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1);
        b[i] = (float)(N - i);
        arr1[i] = i * 2;
    }
    
    int total_errors = 0;
    int tests_passed = 0;
    
    /* Execute all test cases to cover switch statement */
    printf("\n=== Running Partition Type Tests ===\n");
    
    /* Test 1: Gang redundant (case 0) */
    test_gang_redundant(a, b, c);
    total_errors += verify_results(a, b, c, 1);
    tests_passed++;
    
    /* Test 2: Gang partitioned (case 1) */
    test_gang_partitioned(a, b, c);
    total_errors += verify_results(a, b, c, 2);
    tests_passed++;
    
    /* Test 3: Worker partitioned (case 2) */
    test_worker_partitioned(a, b, c);
    total_errors += verify_results(a, b, c, 3);
    tests_passed++;
    
    /* Test 4: Gang+worker partitioned (case 3) */
    test_gang_worker_partitioned(a, b, c);
    total_errors += verify_results(a, b, c, 4);
    tests_passed++;
    
    /* Test 5: Vector partitioned (case 4) */
    test_vector_partitioned(a, b, c);
    total_errors += verify_results(a, b, c, 5);
    tests_passed++;
    
    /* Test 6: Gang+vector partitioned (case 5) */
    test_gang_vector_partitioned(a, b, c);
    total_errors += verify_results(a, b, c, 6);
    tests_passed++;
    
    /* Test 7: Worker+vector partitioned (case 6) */
    test_worker_vector_partitioned(a, b, c);
    total_errors += verify_results(a, b, c, 7);
    tests_passed++;
    
    /* Test 8: Fully partitioned (case 7) */
    test_fully_partitioned(a, b, c);
    total_errors += verify_results(a, b, c, 8);
    tests_passed++;
    
    /* Additional tests for nested and conditional cases */
    printf("\n=== Running Advanced Partition Tests ===\n");
    
    /* Test 9: Nested parallelism */
    test_nested_partitioning(arr1, arr2);
    tests_passed++;
    
    /* Test 10: Conditional partition */
    test_conditional_partition(a, b, c, 1);
    tests_passed++;
    
    /* Test 11: Data clauses with partitioned arrays */
    test_data_partition_clauses();
    tests_passed++;
    
    /* Test 12: Device-specific behaviors */
    test_device_specific_partition();
    tests_passed++;
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests executed: %d\n", tests_passed);
    printf("Numerical errors in basic tests: %d\n", total_errors);
    
    if (total_errors == 0) {
        printf("SUCCESS: All partition types exercised\n");
    } else {
        printf("WARNING: Some numerical errors detected\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr1);
    free(arr2);
    
    return total_errors > 0 ? 1 : 0;
}
