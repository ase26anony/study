/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 * 
 * This program creates all 8 partition type combinations:
 * 0: gang redundant
 * 1: gang partitioned
 * 2: worker partitioned
 * 3: gang+worker partitioned
 * 4: vector partitioned
 * 5: gang+vector partitioned
 * 6: worker+vector partitioned
 * 7: fully partitioned
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

/* Function declarations with partition specifications */
#pragma acc routine gang
void acc_gang_function(float *a, float *b, float *c, int n);

#pragma acc routine worker
void acc_worker_function(float *a, float *b, float *c, int n);

#pragma acc routine vector
void acc_vector_function(float *a, float *b, float *c, int n);

/* Implementation of partitioned functions */
void acc_gang_function(float *a, float *b, float *c, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

void acc_worker_function(float *a, float *b, float *c, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
}

void acc_vector_function(float *a, float *b, float *c, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        c[i] = a[i] - b[i];
    }
}

/* Test 0: Gang redundant (default) */
void test_gang_redundant(float *a, float *b, float *c, int *errors) {
    printf("Test 0: Gang redundant\n");
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc kernels gang
        {
            /* Inner loop - gang redundant by default */
            #pragma acc loop
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i])) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* Test 1: Gang partitioned */
void test_gang_partitioned(float *a, float *b, float *c, int *errors) {
    printf("Test 1: Gang partitioned\n");
    
    int num_gangs = 4;
    int gang_size = N / num_gangs;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            int gang_id = i / gang_size;
            c[i] = a[i] + b[i] + gang_id * 0.1f;
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        int gang_id = i / gang_size;
        if (fabs(c[i] - (a[i] + b[i] + gang_id * 0.1f)) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* Test 2: Worker partitioned */
void test_worker_partitioned(float *a, float *b, float *c, int *errors) {
    printf("Test 2: Worker partitioned\n");
    
    int num_workers = 8;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Outer gang-redundant region with inner worker partitioning */
        #pragma acc parallel num_gangs(1)
        {
            #pragma acc loop worker(num_workers)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * b[i];
            }
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * b[i])) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* Test 3: Gang+Worker partitioned */
void test_gang_worker_partitioned(float *a, float *b, float *c, int *errors) {
    printf("Test 3: Gang+Worker partitioned\n");
    
    int num_gangs = 2;
    int num_workers = 4;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] - b[i])) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* Test 4: Vector partitioned */
void test_vector_partitioned(float *a, float *b, float *c, int *errors) {
    printf("Test 4: Vector partitioned\n");
    
    int vector_length = 32;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Using conditional to select vector length */
        int use_long_vector = 1;
        int actual_vlen = use_long_vector ? vector_length : 16;
        
        #pragma acc parallel loop vector_length(actual_vlen)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] / (b[i] + 1.0f))) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* Test 5: Gang+Vector partitioned */
void test_gang_vector_partitioned(float *a, float *b, float *c, int *errors) {
    printf("Test 5: Gang+Vector partitioned\n");
    
    int num_gangs = 4;
    int vector_length = 64;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Nested parallelism: outer gang, inner vector */
        #pragma acc kernels num_gangs(num_gangs) vector_length(vector_length)
        {
            #pragma acc loop gang
            for (int g = 0; g < num_gangs; g++) {
                int start = g * (N / num_gangs);
                int end = (g + 1) * (N / num_gangs);
                
                #pragma acc loop vector
                for (int i = start; i < end && i < N; i++) {
                    c[i] = a[i] * 2.0f + b[i];
                }
            }
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * 2.0f + b[i])) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* Test 6: Worker+Vector partitioned */
void test_worker_vector_partitioned(float *a, float *b, float *c, int *errors) {
    printf("Test 6: Worker+Vector partitioned\n");
    
    int num_workers = 8;
    int vector_length = 32;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        /* Conditional partition selection */
        int flag = 1;
        int actual_workers = flag ? num_workers : 4;
        
        #pragma acc parallel loop worker(actual_workers) vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        float expected = sqrtf(a[i] * a[i] + b[i] * b[i]);
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* Test 7: Fully partitioned (Gang+Worker+Vector) */
void test_fully_partitioned(float *a, float *b, float *c, int *errors) {
    printf("Test 7: Fully partitioned\n");
    
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 16;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i] * 0.5f)) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

/* OpenMP Target versions for cross-runtime testing */
#ifdef _OPENMP
void test_omp_gang_partitioned(float *a, float *b, float *c, int *errors) {
    printf("OMP Test: Gang partitioned\n");
    
    int num_teams = 4;
    int thread_limit = 256;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_teams) thread_limit(thread_limit)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i])) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}

void test_omp_fully_partitioned(float *a, float *b, float *c, int *errors) {
    printf("OMP Test: Fully partitioned with device type\n");
    
    #pragma omp requires unified_shared_memory
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        /* Simulate device-specific behavior */
        #ifdef __NVPTX__
        #pragma omp target teams distribute parallel for collapse(2)
        #else
        #pragma omp target teams distribute parallel for simd
        #endif
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verification */
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * b[i])) > VERIFY_TOLERANCE) {
            (*errors)++;
            break;
        }
    }
}
#endif

int main() {
    float *a, *b, *c;
    int errors = 0;
    int total_tests = 0;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1);
        b[i] = (float)(N - i);
        c[i] = 0.0f;
    }
    
    printf("=== OpenACC Partition Clause Coverage Tests ===\n\n");
    
    /* Execute all 8 partition type tests */
    test_gang_redundant(a, b, c, &errors);
    total_tests++;
    
    test_gang_partitioned(a, b, c, &errors);
    total_tests++;
    
    test_worker_partitioned(a, b, c, &errors);
    total_tests++;
    
    test_gang_worker_partitioned(a, b, c, &errors);
    total_tests++;
    
    test_vector_partitioned(a, b, c, &errors);
    total_tests++;
    
    test_gang_vector_partitioned(a, b, c, &errors);
    total_tests++;
    
    test_worker_vector_partitioned(a, b, c, &errors);
    total_tests++;
    
    test_fully_partitioned(a, b, c, &errors);
    total_tests++;
    
    /* Test partitioned function calls */
    printf("\nTesting partitioned function calls...\n");
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < 1; i++) {
                acc_gang_function(a, b, c, N);
            }
        }
    }
    total_tests++;
    
    /* OpenMP tests if available */
#ifdef _OPENMP
    printf("\n=== OpenMP Target Partition Tests ===\n\n");
    test_omp_gang_partitioned(a, b, c, &errors);
    total_tests++;
    
    test_omp_fully_partitioned(a, b, c, &errors);
    total_tests++;
#endif
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests executed: %d\n", total_tests);
    printf("Errors detected: %d\n", errors);
    
    if (errors == 0) {
        printf("SUCCESS: All partition types exercised\n");
    } else {
        printf("FAILURE: %d test(s) failed\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return errors > 0 ? 1 : 0;
}
