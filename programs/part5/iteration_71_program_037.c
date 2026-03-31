/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 * 
 * Compile with:
 *   OpenACC: gcc -O2 -fopenacc -foffload=nvptx-none -std=gnu11 -o test_acc test.c
 *   OpenMP:  gcc -O2 -fopenmp -foffload=amd_gcn -std=gnu11 -o test_omp test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function declarations with partition attributes */
#pragma acc routine gang
void acc_gang_function(int *arr, int n, int val);

#pragma acc routine worker
void acc_worker_function(int *arr, int n, int val);

#pragma acc routine vector
void acc_vector_function(int *arr, int n, int val);

/* Helper function to verify results */
int verify_results(int *a, int *b, int n, const char *test_name) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            printf("Test %s failed at index %d: %d != %d\n", 
                   test_name, i, a[i], b[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    int *a, *b, *c, *d;
    int success_count = 0;
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
    
    /* Runtime partition configuration variables */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1;
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /*******************************************************************
     * TEST 1: Gang redundant (case 0)
     * Outer region with gang redundancy, inner with gang partitioning
     *******************************************************************/
    total_tests++;
    printf("Test 1: Gang redundant\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            /* Inner gang-partitioned loop */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    /* Verify results */
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i];
    }
    if (verify_results(c, d, N, "Gang redundant")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 2: Gang partitioned (case 1)
     * Simple gang-partitioned parallel loop
     *******************************************************************/
    total_tests++;
    printf("\nTest 2: Gang partitioned\n");
    
    #pragma acc parallel loop gang(num_gangs) copy(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 2;
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * 2;
    }
    if (verify_results(c, d, N, "Gang partitioned")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 3: Worker partitioned (case 2)
     * Worker-partitioned loop with conditional worker count
     *******************************************************************/
    total_tests++;
    printf("\nTest 3: Worker partitioned\n");
    
    #pragma acc parallel num_gangs(1) num_workers(flag ? 2 : 4) \
                copy(c[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] - b[i];
    }
    if (verify_results(c, d, N, "Worker partitioned")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 4: Gang+worker partitioned (case 3)
     * Nested gang and worker partitioning
     *******************************************************************/
    total_tests++;
    printf("\nTest 4: Gang+worker partitioned\n");
    
    #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) \
                copy(c[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + i;
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + i;
    }
    if (verify_results(c, d, N, "Gang+worker partitioned")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 5: Vector partitioned (case 4)
     * Vector-partitioned loop with explicit vector length
     *******************************************************************/
    total_tests++;
    printf("\nTest 5: Vector partitioned\n");
    
    #pragma acc parallel vector_length(vector_length) copy(c[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] * 3;
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = b[i] * 3;
    }
    if (verify_results(c, d, N, "Vector partitioned")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 6: Gang+vector partitioned (case 5)
     * Combined gang and vector partitioning
     *******************************************************************/
    total_tests++;
    printf("\nTest 6: Gang+vector partitioned\n");
    
    #pragma acc parallel num_gangs(num_gangs) vector_length(vector_length) \
                copy(c[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * b[i];
    }
    if (verify_results(c, d, N, "Gang+vector partitioned")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 7: Worker+vector partitioned (case 6)
     * Combined worker and vector partitioning
     *******************************************************************/
    total_tests++;
    printf("\nTest 7: Worker+vector partitioned\n");
    
    #pragma acc parallel num_workers(num_workers) vector_length(vector_length) \
                copy(c[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1);  /* Avoid division by zero */
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] / (b[i] + 1);
    }
    if (verify_results(c, d, N, "Worker+vector partitioned")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 8: Fully partitioned (case 7)
     * Gang, worker, and vector all partitioned
     *******************************************************************/
    total_tests++;
    printf("\nTest 8: Fully partitioned\n");
    
    #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) \
                vector_length(vector_length) copy(c[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] + i;
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] + i;
    }
    if (verify_results(c, d, N, "Fully partitioned")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 9: OpenMP target equivalent
     * Using OpenMP target teams distribute parallel for
     *******************************************************************/
    total_tests++;
    printf("\nTest 9: OpenMP target with partition variations\n");
    
    #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers) \
                map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] - b[i] * 2;
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] - b[i] * 2;
    }
    if (verify_results(c, d, N, "OpenMP target")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 10: Nested parallelism with device-specific behavior
     * Testing neuter broadcast logic
     *******************************************************************/
    total_tests++;
    printf("\nTest 10: Nested parallelism with data clauses\n");
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Outer region with gang partitioning */
        #pragma acc parallel num_gangs(num_gangs)
        {
            /* Inner region with worker partitioning */
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * a[i];
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * a[i];
    }
    if (verify_results(c, d, N, "Nested parallelism")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * TEST 11: Conditional partition selection
     * Potentially exercising default "<illegal>" case
     *******************************************************************/
    total_tests++;
    printf("\nTest 11: Conditional partition selection\n");
    
    int dynamic_gangs = flag ? 2 : 1;
    int dynamic_workers = flag ? 4 : 2;
    
    #pragma acc parallel num_gangs(dynamic_gangs) \
                num_workers(dynamic_workers) \
                vector_length(flag ? 32 : 64) \
                copy(c[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] % (b[i] + 1);
        }
    }
    
    for (int i = 0; i < N; i++) {
        d[i] = a[i] % (b[i] + 1);
    }
    if (verify_results(c, d, N, "Conditional partition")) {
        success_count++;
        printf("  PASSED\n");
    }
    
    /*******************************************************************
     * Cleanup and summary
     *******************************************************************/
    free(a);
    free(b);
    free(c);
    free(d);
    
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", success_count);
    printf("  Failed: %d\n", total_tests - success_count);
    
    if (success_count == total_tests) {
        printf("\nAll tests passed! All partition cases should be covered.\n");
        return 0;
    } else {
        printf("\nSome tests failed. Check output for details.\n");
        return 1;
    }
}

/* Partitioned function implementations */
void acc_gang_function(int *arr, int n, int val) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] += val;
    }
}

void acc_worker_function(int *arr, int n, int val) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] -= val;
    }
}

void acc_vector_function(int *arr, int n, int val) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] *= val;
    }
}
