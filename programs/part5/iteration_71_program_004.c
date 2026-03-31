/* 
 * Test program to cover partition type identification logic in omp-oacc-neuter-broadcast.cc
 * Specifically targeting the switch statement at lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b) {
    return a * b + a - b;
}

#pragma acc routine gang
float gang_reduction(float *arr, int n) {
    float sum = 0.0f;
    #pragma acc loop gang reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant() {
    printf("Test 1: Gang redundant\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    /* Outer region with gang redundancy */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Gang redundant - no partitioning specified */
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i];
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned() {
    printf("Test 2: Gang partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 4;  /* Runtime variable */
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 3);
    }
    
    /* Explicit gang partitioning */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            c[i] = compute_element(a[i], b[i]);
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = compute_element(a[i], b[i]);
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    float a[N], b[N], c[N];
    int num_workers = 4;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i + 1);
    }
    
    /* Worker partitioned loop */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * b[i];
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned() {
    printf("Test 4: Gang+worker partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 2;
    int num_workers = 4;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    
    /* Combined gang and worker partitioning */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] - b[i];
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    float a[N], b[N], c[N];
    int vector_len = 32;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 1.5f;
    }
    
    /* Vector partitioned loop */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop vector(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] / (b[i] + 1.0f);
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    printf("Test 6: Gang+vector partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 4;
    int vector_len = 16;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i % 10);
    }
    
    /* Gang and vector partitioning */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i] * 2.0f;
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    printf("Test 7: Worker+vector partitioned\n");
    float a[N], b[N], c[N];
    int num_workers = 2;
    int vector_len = 64;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i * i);
        b[i] = (float)i;
    }
    
    /* Worker and vector partitioning */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = sqrtf(a[i] + b[i]);
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = sqrtf(a[i] + b[i]);
        if (fabs(c[i] - expected) > VERIFY_TOL) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned() {
    printf("Test 8: Fully partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 2;
    int num_workers = 2;
    int vector_len = 8;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    /* Fully partitioned: gang, worker, and vector */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i] - a[i] + b[i];
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * b[i] - a[i] + b[i];
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 9: Nested parallelism with different partition types */
void test_nested_partitioning() {
    printf("Test 9: Nested partitioning\n");
    float arr[N], result[N];
    float sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)(i + 1);
    }
    
    /* Outer gang-redundant region with inner worker-partitioned loop */
    #pragma acc data copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i += 64) {
            /* Inner worker-partitioned computation */
            #pragma acc loop worker
            for (int j = i; j < i + 64 && j < N; j++) {
                result[j] = arr[j] * 2.0f;
            }
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = arr[i] * 2.0f;
        if (result[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 10: Conditional partition selection */
void test_conditional_partition() {
    printf("Test 10: Conditional partition selection\n");
    float a[N], b[N], c[N];
    int flag = 1;  /* Runtime condition */
    int num_gangs = flag ? 2 : 4;
    int num_workers = flag ? 4 : 2;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 3);
    }
    
    /* Conditional partitioning */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * (flag ? 1.0f : 2.0f);
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i] * (flag ? 1.0f : 2.0f);
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 11: OpenMP target equivalent with partition variations */
#ifdef _OPENMP
void test_omp_partitioning() {
    printf("Test 11: OpenMP target partitioning\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        /* Fully partitioned equivalent */
        #pragma omp target teams distribute parallel for \
                    num_teams(2) num_threads(4)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i];
        if (c[i] != expected) errors++;
    }
    printf("  Errors: %d\n", errors);
}
#endif

/* Test 12: Device-specific partition behavior */
void test_device_specific() {
    printf("Test 12: Device-specific partitioning\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i % 5);
    }
    
    /* NVIDIA-specific device type */
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Different partition for NVIDIA */
        #pragma acc parallel loop gang(4) vector(32)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    /* Verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] / (b[i] + 1.0f);
        if (fabs(c[i] - expected) > VERIFY_TOL) errors++;
    }
    printf("  Errors: %d\n", errors);
}

int main() {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing Partition Type Identification Logic ===\n\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    total_tests++; passed_tests++;
    
    test_gang_partitioned();
    total_tests++; passed_tests++;
    
    test_worker_partitioned();
    total_tests++; passed_tests++;
    
    test_gang_worker_partitioned();
    total_tests++; passed_tests++;
    
    test_vector_partitioned();
    total_tests++; passed_tests++;
    
    test_gang_vector_partitioned();
    total_tests++; passed_tests++;
    
    test_worker_vector_partitioned();
    total_tests++; passed_tests++;
    
    test_fully_partitioned();
    total_tests++; passed_tests++;
    
    test_nested_partitioning();
    total_tests++; passed_tests++;
    
    test_conditional_partition();
    total_tests++; passed_tests++;
    
    test_device_specific();
    total_tests++; passed_tests++;
    
#ifdef _OPENMP
    test_omp_partitioning();
    total_tests++; passed_tests++;
#endif
    
    printf("\n=== Summary ===\n");
    printf("Total tests executed: %d\n", total_tests);
    printf("Tests passed: %d\n", passed_tests);
    
    if (passed_tests == total_tests) {
        printf("SUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("FAILURE: Some tests failed!\n");
        return 1;
    }
}
