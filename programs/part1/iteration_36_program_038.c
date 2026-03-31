/* test_partition_cases.c - Cover all partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

/* Prevent optimization */
static volatile int force_runtime = 1;

/* Helper to use results and prevent dead code elimination */
#define USE(var) asm volatile("" : : "r"(var))

/* Test functions for each partition type */

/* Case 0: gang redundant - single gang */
void test_gang_redundant(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        int idx = 0;
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr[i] += 1;
        }
    }
    
    /* Force runtime check that could use partition info */
    if (force_runtime) {
        int sum = 0;
        for (int i = 0; i < n; i++) sum += arr[i];
        USE(sum);
    }
    
    free(arr);
}

/* Case 1: gang partitioned - multiple gangs */
void test_gang_partitioned(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(4) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr[i] *= 2;
        }
    }
    
    free(arr);
}

/* Case 2: worker partitioned - single gang, multiple workers */
void test_worker_partitioned(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            arr[i] += i % 10;
        }
    }
    
    free(arr);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(2) num_workers(2) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 3 + 1;
        }
    }
    
    free(arr);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(1) num_workers(1) vector_length(128)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] << 1;
        }
    }
    
    free(arr);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(4) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] / 2;
        }
    }
    
    free(arr);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(1) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] | 0xFF;
        }
    }
    
    free(arr);
}

/* Case 7: fully partitioned */
void test_fully_partitioned(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:n]) num_gangs(8) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            arr[i] = (arr[i] + 7) * 11;
        }
    }
    
    free(arr);
}

/* OpenMP equivalents to trigger similar logic */
void test_omp_partitioning(int n) {
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i;
    
    /* Various OpenMP target configurations */
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:n]) \
        num_teams(4) thread_limit(64)
    for (int i = 0; i < n; i++) {
        arr[i] += omp_get_team_num();
    }
    
    #pragma omp target teams distribute parallel for simd map(tofrom: arr[0:n]) \
        num_teams(2) thread_limit(32)
    for (int i = 0; i < n; i++) {
        arr[i] *= omp_get_thread_num();
    }
    
    free(arr);
}

/* Function that could generate invalid partition codes */
void test_invalid_partition(int code) {
    /* This might trigger the default case through various paths */
    int n = 100;
    int *arr = (int*)malloc(n * sizeof(int));
    
    /* Use code to select partitioning - compiler might generate all paths */
    switch (code & 0x7) {
        case 0:
            #pragma acc parallel copy(arr[0:n]) num_gangs(1)
            { /* ... */ }
            break;
        case 1:
            #pragma acc parallel copy(arr[0:n]) num_gangs(2)
            { /* ... */ }
            break;
        /* ... other cases ... */
        default:
            /* Could generate invalid partition configuration */
            if (code > 7) {
                /* Force generation of default case logic */
                volatile int *volatile_ptr = (volatile int*)&code;
                USE(volatile_ptr);
            }
    }
    
    free(arr);
}

/* Main test driver */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    
    printf("Testing partition mapping cases...\n");
    
    /* Test all valid partition cases */
    test_gang_redundant(n);
    printf("Case 0 tested\n");
    
    test_gang_partitioned(n);
    printf("Case 1 tested\n");
    
    test_worker_partitioned(n);
    printf("Case 2 tested\n");
    
    test_gang_worker_partitioned(n);
    printf("Case 3 tested\n");
    
    test_vector_partitioned(n);
    printf("Case 4 tested\n");
    
    test_gang_vector_partitioned(n);
    printf("Case 5 tested\n");
    
    test_worker_vector_partitioned(n);
    printf("Case 6 tested\n");
    
    test_fully_partitioned(n);
    printf("Case 7 tested\n");
    
    /* Test OpenMP variants */
    test_omp_partitioning(n);
    printf("OpenMP variants tested\n");
    
    /* Test boundary/invalid cases */
    for (int i = 0; i < 10; i++) {
        test_invalid_partition(i);
    }
    printf("Boundary cases tested\n");
    
    printf("All tests completed\n");
    return 0;
}
