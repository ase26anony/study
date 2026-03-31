/* test_partition_cases.c - Test program for OpenACC/OpenMP partition coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

/* Prevent optimization */
static volatile int force_code_generation = 0;

/* Helper to use results and prevent dead code elimination */
#define USE(var) asm volatile("" : : "r"(var))

/* Test functions for each partition type */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 0: Single gang, gang redundant */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        int idx = acc_gang_id * acc_worker_id * acc_vector_id;
        if (idx < n) data[idx] += 1;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 1: Multiple gangs, gang partitioned */
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(1)
    {
        int gid = acc_gang_id;
        if (gid < n) data[gid] *= 2;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 2: Single gang, multiple workers */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(1)
    {
        int wid = acc_worker_id;
        if (wid < n) data[wid] += wid;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 3: Multiple gangs and workers */
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(2) vector_length(1)
    {
        int idx = acc_gang_id * 2 + acc_worker_id;
        if (idx < n) data[idx] = idx * 3;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 4: Vector partitioning only */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(32)
    {
        int vid = acc_vector_id;
        if (vid < n) data[vid] += vid * 2;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 5: Gang + vector partitioning */
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(1) vector_length(16)
    {
        int idx = acc_gang_id * 16 + acc_vector_id;
        if (idx < n) data[idx] = idx * 5;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 6: Worker + vector partitioning */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(2) vector_length(8)
    {
        int idx = acc_worker_id * 8 + acc_vector_id;
        if (idx < n) data[idx] += idx * 7;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 7: Fully partitioned (gang + worker + vector) */
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(2) vector_length(4)
    {
        int idx = (acc_gang_id * 2 + acc_worker_id) * 4 + acc_vector_id;
        if (idx < n) data[idx] = idx * 11;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* OpenMP target with teams (gangs) */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n]) num_teams(4) thread_limit(1)
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num();
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_omp_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* OpenMP with threads (workers) */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n]) num_teams(1) thread_limit(4)
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_thread_num();
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

void test_omp_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* OpenMP with SIMD (vector) */
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:n]) \
            num_teams(2) thread_limit(2) simdlen(4)
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() * 100 + omp_get_thread_num() * 10;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

/* Function to potentially trigger default case through variable partitioning */
void test_variable_partition(int n, int gangs, int workers, int vector_len) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Runtime-determined partitioning - may generate different codes */
    #pragma acc parallel copy(data[0:n]) num_gangs(gangs) num_workers(workers) vector_length(vector_len)
    {
        int idx = acc_gang_id * workers * vector_len + acc_worker_id * vector_len + acc_vector_id;
        if (idx < n) data[idx] += idx;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    USE(sum);
    free(data);
}

/* Template-like macro to generate different partitionings */
#define TEST_PARTITION_CASE(case_id, gangs, workers, vector_len) \
    do { \
        int n = 64; \
        int *arr = (int*)malloc(n * sizeof(int)); \
        for (int i = 0; i < n; i++) arr[i] = i; \
        \
        #pragma acc parallel copy(arr[0:n]) \
                num_gangs(gangs) num_workers(workers) vector_length(vector_len) \
        { \
            int idx = acc_gang_id * workers * vector_len + \
                     acc_worker_id * vector_len + acc_vector_id; \
            if (idx < n) arr[idx] += case_id; \
        } \
        \
        int check = 0; \
        for (int i = 0; i < n; i++) check += arr[i]; \
        USE(check); \
        free(arr); \
    } while(0)

int main() {
    int test_size = 128;
    
    printf("Testing all partition cases...\n");
    
    /* Test OpenACC cases */
    test_gang_redundant(test_size);
    test_gang_partitioned(test_size);
    test_worker_partitioned(test_size);
    test_gang_worker_partitioned(test_size);
    test_vector_partitioned(test_size);
    test_gang_vector_partitioned(test_size);
    test_worker_vector_partitioned(test_size);
    test_fully_partitioned(test_size);
    
    /* Test OpenMP cases */
    test_omp_gang_partitioned(test_size);
    test_omp_worker_partitioned(test_size);
    test_omp_fully_partitioned(test_size);
    
    /* Test with macro expansion for all cases */
    TEST_PARTITION_CASE(0, 1, 1, 1);    /* gang redundant */
    TEST_PARTITION_CASE(1, 4, 1, 1);    /* gang partitioned */
    TEST_PARTITION_CASE(2, 1, 4, 1);    /* worker partitioned */
    TEST_PARTITION_CASE(3, 2, 2, 1);    /* gang+worker partitioned */
    TEST_PARTITION_CASE(4, 1, 1, 32);   /* vector partitioned */
    TEST_PARTITION_CASE(5, 2, 1, 16);   /* gang+vector partitioned */
    TEST_PARTITION_CASE(6, 1, 2, 8);    /* worker+vector partitioned */
    TEST_PARTITION_CASE(7, 2, 2, 4);    /* fully partitioned */
    
    /* Test variable partitioning - may generate different codes */
    for (int i = 0; i < 10; i++) {
        test_variable_partition(test_size, 
                               (i % 3) + 1,    /* gangs 1-3 */
                               (i % 4) + 1,    /* workers 1-4 */
                               (i % 8) * 4 + 1); /* vector 1-29 */
    }
    
    /* Boundary case: potentially invalid partition codes */
    if (force_code_generation) {
        /* This might trigger default case through compiler internals */
        test_variable_partition(test_size, 0, 0, 0);  /* Invalid values */
    }
    
    printf("All partition tests completed.\n");
    return 0;
}
