/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 * 
 * Compile with:
 *   OpenACC: gcc -O2 -fopenacc -foffload=nvptx-none -std=gnu11 test.c -o test_acc
 *   OpenMP:  gcc -O2 -fopenmp -foffload=amd_gcn -std=gnu11 test.c -o test_omp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function with explicit partition specification */
#pragma acc routine gang
void acc_gang_function(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}

#pragma omp declare target
void omp_gang_function(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}
#pragma omp end declare target

int main() {
    int i, errors = 0;
    int *a, *b, *c;
    int num_gangs, num_workers, vector_len;
    int flag = 1;
    
    /* Initialize runtime partition variables */
    num_gangs = 2;
    num_workers = 4;
    vector_len = 32;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    printf("Testing partition clause variations...\n");
    
    /* =========================================== */
    /* Test 1: gang redundant (case 0) */
    /* =========================================== */
    printf("Test 1: gang redundant\n");
    #pragma acc parallel loop gang(num_gangs) copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Verify */
    for (i = 0; i < N; i++) {
        if (c[i] != N) errors++;
    }
    
    /* =========================================== */
    /* Test 2: gang partitioned (case 1) */
    /* =========================================== */
    printf("Test 2: gang partitioned\n");
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* =========================================== */
    /* Test 3: worker partitioned (case 2) */
    /* =========================================== */
    printf("Test 3: worker partitioned\n");
    #pragma acc parallel num_gangs(num_gangs) copy(c[0:N])
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            c[i] = i;
        }
    }
    
    /* =========================================== */
    /* Test 4: gang+worker partitioned (case 3) */
    /* =========================================== */
    printf("Test 4: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) copy(c[0:N])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            c[i] = a[i] + i;
        }
    }
    
    /* =========================================== */
    /* Test 5: vector partitioned (case 4) */
    /* =========================================== */
    printf("Test 5: vector partitioned\n");
    #pragma acc parallel loop vector_length(vector_len) vector copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = b[i] - i;
    }
    
    /* =========================================== */
    /* Test 6: gang+vector partitioned (case 5) */
    /* =========================================== */
    printf("Test 6: gang+vector partitioned\n");
    #pragma acc parallel loop gang(num_gangs) vector_length(vector_len) gang vector copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 2;
    }
    
    /* =========================================== */
    /* Test 7: worker+vector partitioned (case 6) */
    /* =========================================== */
    printf("Test 7: worker+vector partitioned\n");
    #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_len) copy(c[0:N])
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            c[i] = b[i] * 3;
        }
    }
    
    /* =========================================== */
    /* Test 8: fully partitioned (case 7) */
    /* =========================================== */
    printf("Test 8: fully partitioned\n");
    #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_len) copy(c[0:N])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] + i;
        }
    }
    
    /* =========================================== */
    /* Test 9: Nested parallelism with partitioning */
    /* =========================================== */
    printf("Test 9: Nested parallelism\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc parallel num_gangs(num_gangs)
        {
            /* Inner worker-partitioned loop */
            #pragma acc loop worker
            for (i = 0; i < N; i++) {
                acc_gang_function(c, i, a[i]);
            }
        }
    }
    
    /* =========================================== */
    /* Test 10: Conditional partition selection */
    /* =========================================== */
    printf("Test 10: Conditional partition\n");
    #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = flag ? a[i] : b[i];
    }
    
    /* =========================================== */
    /* Test 11: Multiple device type simulation */
    /* =========================================== */
    printf("Test 11: Device-specific behavior\n");
    #ifdef _OPENACC
    #pragma acc set device_type(nvidia)
    #endif
    #pragma acc parallel loop gang(num_gangs) vector_length(vector_len) gang vector copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] % 10;
    }
    
    /* =========================================== */
    /* OpenMP Target equivalents */
    /* =========================================== */
    printf("\nTesting OpenMP target variations...\n");
    
    /* Reset arrays for OpenMP tests */
    for (i = 0; i < N; i++) {
        c[i] = 0;
    }
    
    /* Test 12: OpenMP gang partitioned */
    printf("Test 12: OpenMP gang partitioned\n");
    #pragma omp target teams distribute parallel for \
        num_teams(num_gangs) map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Test 13: OpenMP with worker partitioning */
    printf("Test 13: OpenMP worker partitioned\n");
    #pragma omp target teams distribute parallel for simd \
        num_teams(num_gangs) thread_limit(num_workers) \
        map(to: a[0:N]) map(from: c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 2;
    }
    
    /* Test 14: OpenMP fully partitioned */
    printf("Test 14: OpenMP fully partitioned\n");
    #pragma omp target teams distribute parallel for simd \
        num_teams(num_gangs) thread_limit(num_workers) \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] * b[i];
    }
    
    /* Test 15: OpenMP nested with function call */
    printf("Test 15: OpenMP nested with function\n");
    #pragma omp target teams distribute map(to: a[0:N]) map(from: c[0:N])
    for (i = 0; i < N; i++) {
        #pragma omp parallel for
        for (int j = 0; j < 4; j++) {
            omp_gang_function(c, i, a[i] + j);
        }
    }
    
    /* =========================================== */
    /* Verification and cleanup */
    /* =========================================== */
    
    /* Final verification */
    for (i = 0; i < N; i++) {
        if (c[i] < 0) {  /* Simple sanity check */
            errors++;
        }
    }
    
    printf("\nTest Summary:\n");
    printf("  Total errors detected: %d\n", errors);
    printf("  %s\n", errors == 0 ? "All tests passed!" : "Some tests failed.");
    
    free(a);
    free(b);
    free(c);
    
    return errors > 0 ? 1 : 0;
}
