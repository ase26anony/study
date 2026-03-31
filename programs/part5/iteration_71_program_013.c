/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 * 
 * Compilation:
 *   OpenACC: gcc -O2 -fopenacc -foffload=nvptx-none -std=gnu11 test.c -o test_acc
 *   OpenMP:  gcc -O2 -fopenmp -foffload=amd_gcn -std=gnu11 test.c -o test_omp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

/* Function declarations with partition hints */
#pragma acc routine gang
void acc_gang_function(float *a, float *b, int n);

#pragma acc routine worker
void acc_worker_function(float *a, float *b, int n);

#pragma acc routine vector
void acc_vector_function(float *a, float *b, int n);

/* Device-specific partition behaviors */
#ifdef _OPENACC
#pragma acc set device_type(nvidia)
#endif

#ifdef _OPENMP
#pragma omp requires unified_shared_memory
#endif

/* Helper functions */
void initialize_arrays(float *a, float *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)i;
        b[i] = (float)(n - i);
    }
}

int verify_results(float *a, float *b, float *c, int n, float expected_factor) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float expected = a[i] + b[i] * expected_factor;
        if (c[i] - expected > VERIFY_TOLERANCE || expected - c[i] > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: got %f, expected %f\n", i, c[i], expected);
            }
        }
    }
    return errors;
}

/* Partitioned function implementations */
void acc_gang_function(float *a, float *b, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        a[i] += b[i] * 2.0f;
    }
}

void acc_worker_function(float *a, float *b, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        a[i] += b[i] * 3.0f;
    }
}

void acc_vector_function(float *a, float *b, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        a[i] += b[i] * 4.0f;
    }
}

int main() {
    float *a, *b, *c;
    int total_errors = 0;
    int test_count = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1; /* For conditional partition selection */
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Testing OpenACC/OpenMP partition clause variations\n");
    printf("==================================================\n\n");
    
    /* Test 1: Gang redundant (case 0) */
    printf("Test %d: Gang redundant\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) copy(c[0:N])
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    total_errors += verify_results(a, b, c, N, 1.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 1.0f));
    
    /* Test 2: Gang partitioned (case 1) */
    printf("\nTest %d: Gang partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 2.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 2.0f));
    
    /* Test 3: Worker partitioned (case 2) */
    printf("\nTest %d: Worker partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 3.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 3.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 3.0f));
    
    /* Test 4: Gang+worker partitioned (case 3) */
    printf("\nTest %d: Gang+worker partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 4.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 4.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 4.0f));
    
    /* Test 5: Vector partitioned (case 4) */
    printf("\nTest %d: Vector partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop vector(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 5.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 5.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 5.0f));
    
    /* Test 6: Gang+vector partitioned (case 5) */
    printf("\nTest %d: Gang+vector partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 6.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 6.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 6.0f));
    
    /* Test 7: Worker+vector partitioned (case 6) */
    printf("\nTest %d: Worker+vector partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 7.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 7.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 7.0f));
    
    /* Test 8: Fully partitioned (case 7) */
    printf("\nTest %d: Fully partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 8.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 8.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 8.0f));
    
    /* Test 9: Nested parallelism with different partition specifications */
    printf("\nTest %d: Nested parallelism (outer gang, inner worker)\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc kernels gang(num_gangs) copy(c[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                float temp = a[i];
                #pragma acc loop worker
                for (int j = 0; j < 4; j++) {
                    temp += b[i] * 0.25f;
                }
                c[i] = temp;
            }
        }
    }
    
    /* Test 10: Conditional partition selection */
    printf("\nTest %d: Conditional partition selection\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector(flag ? 16 : 32) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * (flag ? 9.0f : 10.0f);
        }
    }
    
    total_errors += verify_results(a, b, c, N, 9.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 9.0f));
    
    /* Test 11: OpenMP target equivalent for gang partitioned */
    printf("\nTest %d: OpenMP target - gang partitioned\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 11.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, 11.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, 11.0f));
    
    /* Test 12: Function calls with partition hints */
    printf("\nTest %d: Function calls with partition hints\n", ++test_count);
    initialize_arrays(a, b, N);
    memset(c, 0, N * sizeof(float));
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i += 256) {
            int chunk = (i + 256 < N) ? 256 : N - i;
            acc_gang_function(&a[i], &b[i], chunk);
            acc_worker_function(&a[i], &b[i], chunk);
            acc_vector_function(&a[i], &b[i], chunk);
            c[i] = a[i];
        }
    }
    
    /* Summary */
    printf("\n==================================================\n");
    printf("Total tests executed: %d\n", test_count);
    printf("Total errors across all tests: %d\n", total_errors);
    
    if (total_errors == 0) {
        printf("SUCCESS: All tests passed!\n");
    } else {
        printf("WARNING: Some tests had errors\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return total_errors > 0 ? 1 : 0;
}
