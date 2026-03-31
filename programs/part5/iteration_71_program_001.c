/* 
 * Test program to exercise all partition type cases in omp-oacc-neuter-broadcast.cc
 * Specifically targets the switch statement at lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Helper function for reductions */
#pragma acc routine gang
int acc_reduction_sum(int *data, int n) {
    int sum = 0;
    #pragma acc loop gang reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}

#pragma omp declare target
int omp_reduction_sum(int *data, int n) {
    int sum = 0;
    #pragma omp teams distribute parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}
#pragma omp end declare target

int main() {
    int *a, *b, *c;
    int num_gangs, num_workers, vector_len;
    int flag = 1;
    int errors = 0;
    int total_tests = 0;
    
    /* Initialize runtime partition variables */
    num_gangs = 2;
    num_workers = 4;
    vector_len = 32;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    printf("Testing OpenACC/OpenMP partition type coverage...\n\n");
    
    /* =========================================== */
    /* Test 1: gang redundant (case 0) */
    /* =========================================== */
    printf("Test 1: gang redundant\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        /* Outer region - gang redundant */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            /* Inner loop - should inherit gang redundant */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    /* Verify results */
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != N) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 2: gang partitioned (case 1) */
    /* =========================================== */
    printf("Test 2: gang partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Explicit gang partitioning */
        #pragma acc parallel loop gang(num_gangs) independent
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 2) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 3: worker partitioned (case 2) */
    /* =========================================== */
    printf("Test 3: worker partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Worker partitioned loop inside gang region */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            #pragma acc loop worker(num_workers)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + 100;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i + 100) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 4: gang+worker partitioned (case 3) */
    /* =========================================== */
    printf("Test 4: gang+worker partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Combined gang and worker partitioning */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            #pragma acc loop gang worker(num_workers)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 3;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 3) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 5: vector partitioned (case 4) */
    /* =========================================== */
    printf("Test 5: vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Vector partitioned loop */
        #pragma acc parallel vector_length(vector_len)
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] - 50;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i - 50) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 6: gang+vector partitioned (case 5) */
    /* =========================================== */
    printf("Test 6: gang+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Combined gang and vector partitioning */
        #pragma acc parallel num_gangs(num_gangs) gang vector_length(vector_len)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + a[i];
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 2) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 7: worker+vector partitioned (case 6) */
    /* =========================================== */
    printf("Test 7: worker+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Combined worker and vector partitioning */
        #pragma acc parallel num_workers(num_workers) worker vector_length(vector_len)
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 4;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 4) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 8: fully partitioned (case 7) */
    /* =========================================== */
    printf("Test 8: fully partitioned\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Fully partitioned: gang, worker, and vector */
        #pragma acc parallel num_gangs(num_gangs) gang \
                            num_workers(num_workers) worker \
                            vector_length(vector_len)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 5;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 5) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 9: Conditional partition selection */
    /* =========================================== */
    printf("Test 9: Conditional partition selection\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Runtime conditional partition */
        #pragma acc parallel loop gang(num_gangs) \
                                worker(flag ? 2 : 4) \
                                vector_length(flag ? 16 : 32)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + 200;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i + 200) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 10: OpenMP target with partitioning */
    /* =========================================== */
    printf("Test 10: OpenMP target partitioning\n");
    total_tests++;
    
    #pragma omp target data map(to: a[0:N]) map(from: c[0:N])
    {
        /* OpenMP equivalent with teams and distribute */
        #pragma omp target teams distribute parallel for \
                     num_teams(num_gangs) thread_limit(num_workers*vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 6;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 6) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 11: Nested parallelism with partitioning */
    /* =========================================== */
    printf("Test 11: Nested parallelism\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Outer kernels region */
        #pragma acc kernels
        {
            /* Inner with explicit partitioning */
            #pragma acc loop gang(num_gangs) worker(num_workers)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 7;
            }
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 7) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Test 12: Device-specific behavior */
    /* =========================================== */
    printf("Test 12: Device-specific partitioning\n");
    total_tests++;
    
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 8;
        }
    }
    
    correct = 1;
    for (int i = 0; i < N; i++) {
        if (c[i] != i * 8) {
            correct = 0;
            break;
        }
    }
    if (!correct) errors++;
    printf("  Result: %s\n\n", correct ? "PASS" : "FAIL");
    
    /* =========================================== */
    /* Summary */
    /* =========================================== */
    printf("========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", total_tests - errors);
    printf("  Failed: %d\n", errors);
    printf("  Success rate: %.1f%%\n", 
           (float)(total_tests - errors) / total_tests * 100);
    
    if (errors == 0) {
        printf("\nAll partition type cases exercised successfully!\n");
    } else {
        printf("\nSome tests failed. Check implementation.\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return errors;
}
