/**
 * Test program to exercise OpenACC/OpenMP partition clause logic
 * Specifically targets the switch statement in omp-oacc-neuter-broadcast.cc
 * lines 335-343 covering all partition type cases (0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function prototypes with partition hints */
#pragma acc routine gang
void acc_gang_function(int *arr, int n);

#pragma acc routine worker
void acc_worker_function(int *arr, int n);

#pragma acc routine vector
void acc_vector_function(int *arr, int n);

/* Helper functions */
void initialize_array(int *arr, int n, int base) {
    for (int i = 0; i < n; i++) {
        arr[i] = base + i;
    }
}

int verify_array(int *arr, int *ref, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i] != ref[i]) {
            printf("Mismatch at index %d: %d != %d\n", i, arr[i], ref[i]);
            return 0;
        }
    }
    return 1;
}

/* Partitioned function implementations */
void acc_gang_function(int *arr, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

void acc_worker_function(int *arr, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] += 2;
    }
}

void acc_vector_function(int *arr, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] += 3;
    }
}

int main() {
    int *a, *b, *c, *d, *e, *f, *g, *h;
    int *a_ref, *b_ref, *c_ref, *d_ref, *e_ref, *f_ref, *g_ref, *h_ref;
    int success_count = 0;
    int total_tests = 0;
    
    /* Runtime partition configuration variables */
    int num_gangs = 4;
    int num_workers = 2;
    int vector_length = 32;
    int flag = 1;
    
    /* Allocate host arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    e = (int*)malloc(N * sizeof(int));
    f = (int*)malloc(N * sizeof(int));
    g = (int*)malloc(N * sizeof(int));
    h = (int*)malloc(N * sizeof(int));
    
    a_ref = (int*)malloc(N * sizeof(int));
    b_ref = (int*)malloc(N * sizeof(int));
    c_ref = (int*)malloc(N * sizeof(int));
    d_ref = (int*)malloc(N * sizeof(int));
    e_ref = (int*)malloc(N * sizeof(int));
    f_ref = (int*)malloc(N * sizeof(int));
    g_ref = (int*)malloc(N * sizeof(int));
    h_ref = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    initialize_array(a, N, 0);
    initialize_array(b, N, 10);
    initialize_array(c, N, 20);
    initialize_array(d, N, 30);
    initialize_array(e, N, 40);
    initialize_array(f, N, 50);
    initialize_array(g, N, 60);
    initialize_array(h, N, 70);
    
    memcpy(a_ref, a, N * sizeof(int));
    memcpy(b_ref, b, N * sizeof(int));
    memcpy(c_ref, c, N * sizeof(int));
    memcpy(d_ref, d, N * sizeof(int));
    memcpy(e_ref, e, N * sizeof(int));
    memcpy(f_ref, f, N * sizeof(int));
    memcpy(g_ref, g, N * sizeof(int));
    memcpy(h_ref, h, N * sizeof(int));
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* =========================================== */
    /* TEST 1: Gang redundant (case 0) */
    /* =========================================== */
    printf("Test 1: Gang redundant\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N])
    {
        /* Outer region with gang redundancy */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            /* Inner loop - gang redundant */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                a[i] = a[i] * 2;
            }
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        a_ref[i] = a_ref[i] * 2;
    }
    
    if (verify_array(a, a_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 2: Gang partitioned (case 1) */
    /* =========================================== */
    printf("\nTest 2: Gang partitioned\n");
    total_tests++;
    
    #pragma acc data copy(b[0:N])
    {
        /* Explicit gang partitioning */
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            b[i] = b[i] + i;
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        b_ref[i] = b_ref[i] + i;
    }
    
    if (verify_array(b, b_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 3: Worker partitioned (case 2) */
    /* =========================================== */
    printf("\nTest 3: Worker partitioned\n");
    total_tests++;
    
    #pragma acc data copy(c[0:N])
    {
        /* Worker partitioned loop */
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = c[i] - 5;
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        c_ref[i] = c_ref[i] - 5;
    }
    
    if (verify_array(c, c_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 4: Gang+worker partitioned (case 3) */
    /* =========================================== */
    printf("\nTest 4: Gang+worker partitioned\n");
    total_tests++;
    
    #pragma acc data copy(d[0:N])
    {
        /* Nested parallelism: outer gang, inner worker */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            #pragma acc loop worker(num_workers)
            for (int i = 0; i < N; i++) {
                d[i] = d[i] * 3;
            }
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        d_ref[i] = d_ref[i] * 3;
    }
    
    if (verify_array(d, d_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 5: Vector partitioned (case 4) */
    /* =========================================== */
    printf("\nTest 5: Vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(e[0:N])
    {
        /* Vector partitioned loop */
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            e[i] = e[i] / 2;
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        e_ref[i] = e_ref[i] / 2;
    }
    
    if (verify_array(e, e_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 6: Gang+vector partitioned (case 5) */
    /* =========================================== */
    printf("\nTest 6: Gang+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(f[0:N])
    {
        /* Gang and vector partitioning */
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            f[i] = f[i] + 100;
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        f_ref[i] = f_ref[i] + 100;
    }
    
    if (verify_array(f, f_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 7: Worker+vector partitioned (case 6) */
    /* =========================================== */
    printf("\nTest 7: Worker+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copy(g[0:N])
    {
        /* Worker and vector partitioning with conditional */
        #pragma acc parallel loop \
                worker(flag ? 2 : 4) \
                vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            g[i] = g[i] - 50;
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        g_ref[i] = g_ref[i] - 50;
    }
    
    if (verify_array(g, g_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 8: Fully partitioned (case 7) */
    /* =========================================== */
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    total_tests++;
    
    #pragma acc data copy(h[0:N])
    {
        /* Fully partitioned: gang, worker, and vector */
        #pragma acc parallel loop \
                gang(num_gangs) \
                worker(num_workers) \
                vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            h[i] = h[i] * 2 + 1;
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        h_ref[i] = h_ref[i] * 2 + 1;
    }
    
    if (verify_array(h, h_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    /* =========================================== */
    /* TEST 9: OpenMP target equivalent */
    /* =========================================== */
    printf("\nTest 9: OpenMP target with partition clauses\n");
    total_tests++;
    
    int *omp_arr = (int*)malloc(N * sizeof(int));
    int *omp_ref = (int*)malloc(N * sizeof(int));
    initialize_array(omp_arr, N, 100);
    memcpy(omp_ref, omp_arr, N * sizeof(int));
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
            map(tofrom: omp_arr[0:N]) \
            num_teams(num_gangs) \
            thread_limit(num_workers * vector_length)
    for (int i = 0; i < N; i++) {
        omp_arr[i] = omp_arr[i] * 3;
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        omp_ref[i] = omp_ref[i] * 3;
    }
    
    if (verify_array(omp_arr, omp_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    free(omp_arr);
    free(omp_ref);
    
    /* =========================================== */
    /* TEST 10: Device-specific behavior */
    /* =========================================== */
    printf("\nTest 10: Device-specific partition configuration\n");
    total_tests++;
    
    #ifdef _OPENACC
    #pragma acc set device_type(nvidia)
    #endif
    
    int *dev_arr = (int*)malloc(N * sizeof(int));
    int *dev_ref = (int*)malloc(N * sizeof(int));
    initialize_array(dev_arr, N, 200);
    memcpy(dev_ref, dev_arr, N * sizeof(int));
    
    #pragma acc data copy(dev_arr[0:N])
    {
        /* Complex conditional partitioning */
        int dyn_gangs = flag ? 2 : 8;
        int dyn_workers = flag ? 4 : 2;
        
        #pragma acc parallel loop \
                gang(dyn_gangs) \
                worker(dyn_workers) \
                vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            dev_arr[i] = dev_arr[i] + i % 10;
        }
    }
    
    /* Update reference */
    for (int i = 0; i < N; i++) {
        dev_ref[i] = dev_ref[i] + i % 10;
    }
    
    if (verify_array(dev_arr, dev_ref, N)) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL\n");
    }
    
    free(dev_arr);
    free(dev_ref);
    
    /* =========================================== */
    /* Summary */
    /* =========================================== */
    printf("\n===========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", success_count);
    printf("  Failed: %d\n", total_tests - success_count);
    printf("===========================================\n");
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    free(a_ref); free(b_ref); free(c_ref); free(d_ref);
    free(e_ref); free(f_ref); free(g_ref); free(h_ref);
    
    return (success_count == total_tests) ? 0 : 1;
}
