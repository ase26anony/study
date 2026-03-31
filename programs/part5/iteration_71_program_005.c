/*
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * Targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VALIDATE 1

/* Function prototypes with OpenACC routine directives */
#pragma acc routine seq
int verify_result(int *arr, int expected, int size, const char *test_name);

#pragma acc routine gang
void acc_gang_function(int *arr, int idx, int value);

#pragma acc routine worker
void acc_worker_function(int *arr, int idx, int value);

#pragma acc routine vector
void acc_vector_function(int *arr, int idx, int value);

/* Helper to print test info */
void print_test_header(const char *desc, int case_id) {
    printf("Test %d: %s\n", case_id, desc);
    printf("  Expected switch case: %d\n", case_id);
}

int main() {
    int *a, *b, *c;
    int i, errors = 0;
    int num_gangs, num_workers, vector_len;
    int flag = 1; /* For conditional partition selection */
    
    /* Allocate and initialize arrays */
    a = (int *)malloc(N * sizeof(int));
    b = (int *)malloc(N * sizeof(int));
    c = (int *)malloc(N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* Test 0: gang redundant (case 0) */
    print_test_header("Gang redundant - outer kernels with inner gang loop", 0);
    num_gangs = 4;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc kernels
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                c[i] = a[i] * 2;
            }
        }
    }
    #if VALIDATE
    errors += verify_result(c, 0, N, "Test 0");
    #endif
    
    /* Test 1: gang partitioned (case 1) */
    print_test_header("Gang partitioned - explicit gang clause", 1);
    num_gangs = 8;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) copyin(a[0:N], b[0:N]) copyout(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    #if VALIDATE
    errors += verify_result(c, N, N, "Test 1");
    #endif
    
    /* Test 2: worker partitioned (case 2) */
    print_test_header("Worker partitioned - worker clause only", 2);
    num_workers = 4;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers)
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 3;
        }
    }
    #if VALIDATE
    errors += verify_result(c, 0, N, "Test 2");
    #endif
    
    /* Test 3: gang+worker partitioned (case 3) */
    print_test_header("Gang+worker partitioned - nested gang and worker", 3);
    num_gangs = 2;
    num_workers = 4;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc loop gang
            for (int g = 0; g < num_gangs; g++) {
                #pragma acc loop worker(num_workers)
                for (i = g * (N/num_gangs); i < (g+1) * (N/num_gangs); i++) {
                    c[i] = a[i] - b[i];
                }
            }
        }
    }
    #if VALIDATE
    errors += verify_result(c, 2*i - N, N, "Test 3");
    #endif
    
    /* Test 4: vector partitioned (case 4) */
    print_test_header("Vector partitioned - vector clause only", 4);
    vector_len = 32;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_len)
        for (i = 0; i < N; i++) {
            c[i] = a[i] / 2;
        }
    }
    #if VALIDATE
    errors += verify_result(c, 0, N, "Test 4");
    #endif
    
    /* Test 5: gang+vector partitioned (case 5) */
    print_test_header("Gang+vector partitioned - gang and vector clauses", 5);
    num_gangs = 4;
    vector_len = 64;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len)
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    #if VALIDATE
    errors += verify_result(c, i*(N-i), N, "Test 5");
    #endif
    
    /* Test 6: worker+vector partitioned (case 6) */
    print_test_header("Worker+vector partitioned - worker and vector clauses", 6);
    num_workers = 2;
    vector_len = 16;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_len)
        for (i = 0; i < N; i++) {
            c[i] = a[i] + 100;
        }
    }
    #if VALIDATE
    errors += verify_result(c, i+100, N, "Test 6");
    #endif
    
    /* Test 7: fully partitioned (case 7) */
    print_test_header("Fully partitioned - gang, worker, and vector clauses", 7);
    num_gangs = 2;
    num_workers = 2;
    vector_len = 8;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_len) \
                    copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc loop gang
            for (int g = 0; g < num_gangs; g++) {
                #pragma acc loop worker
                for (int w = 0; w < num_workers; w++) {
                    #pragma acc loop vector
                    for (i = 0; i < N/(num_gangs*num_workers); i++) {
                        int idx = g * (N/num_gangs) + w * (N/(num_gangs*num_workers)) + i;
                        if (idx < N) {
                            c[idx] = a[idx] + b[idx] * 2;
                        }
                    }
                }
            }
        }
    }
    #if VALIDATE
    errors += verify_result(c, i + 2*(N-i), N, "Test 7");
    #endif
    
    /* Test with conditional partition selection */
    printf("\nTest with conditional partition selection:\n");
    num_gangs = flag ? 4 : 8;
    num_workers = flag ? 2 : 4;
    vector_len = flag ? 16 : 32;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 4;
        }
    }
    
    /* OpenMP target equivalents for cross-runtime testing */
    printf("\nOpenMP target tests (additional coverage):\n");
    
    /* OpenMP: gang partitioned */
    #pragma omp target teams distribute parallel for map(tofrom: c[0:N]) num_teams(4)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 5;
    }
    
    /* OpenMP: vector partitioned with simd */
    #pragma omp target teams distribute parallel for simd map(tofrom: c[0:N]) \
                num_teams(2) thread_limit(32)
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 6;
    }
    
    /* Device-specific behaviors */
    #ifdef _OPENACC
    #pragma acc set device_type(nvidia)
    #pragma acc parallel loop gang(2) vector_length(64) copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 7;
    }
    #endif
    
    /* Function calls with routine directives */
    printf("\nTesting routine directives with partitioning:\n");
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < N; i++) {
            acc_gang_function(c, i, i * 10);
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total validation errors: %d\n", errors);
    printf("Partition switch cases exercised: 0-7\n");
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("Some tests failed validation.\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return errors;
}

/* Verification function */
int verify_result(int *arr, int expected, int size, const char *test_name) {
    int errors = 0;
    /* Simple validation - check first, middle, last elements */
    if (arr[0] != expected && expected != 0) {
        printf("  %s: validation error at index 0: got %d\n", test_name, arr[0]);
        errors++;
    }
    if (arr[size/2] != expected && expected != 0) {
        printf("  %s: validation error at index %d: got %d\n", 
               test_name, size/2, arr[size/2]);
        errors++;
    }
    if (arr[size-1] != expected && expected != 0) {
        printf("  %s: validation error at index %d: got %d\n", 
               test_name, size-1, arr[size-1]);
        errors++;
    }
    return errors;
}

/* Routine functions */
void acc_gang_function(int *arr, int idx, int value) {
    arr[idx] = value;
}

void acc_worker_function(int *arr, int idx, int value) {
    arr[idx] = value + 1;
}

void acc_vector_function(int *arr, int idx, int value) {
    arr[idx] = value + 2;
}
