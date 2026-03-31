/* Test program to exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

/* Prevent optimization of partition-dependent results */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Helper to generate runtime-dependent partition codes */
volatile int force_partition_code = 0;

/* Test functions for each partition type */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 0: Single gang, redundant across gangs */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        int idx = acc_threadid();
        if (idx < n) data[idx] += 1;
    }
    
    /* Force partition mapping logic */
    if (force_partition_code == 0) {
        printf("Partition type: gang redundant\n");
    }
    
    free(data);
}

void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 1: Multiple gangs, partitioned by gang */
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(1)
    {
        int idx = acc_threadid();
        if (idx < n) data[idx] += acc_gangid();
    }
    
    KEEP(data[n/2]);
    free(data);
}

void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 2: Single gang, multiple workers */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(1)
    {
        int idx = acc_threadid();
        if (idx < n) data[idx] += acc_workerid();
    }
    
    KEEP(data[n/2]);
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 3: Multiple gangs and workers */
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(2) vector_length(1)
    {
        int idx = acc_threadid();
        if (idx < n) data[idx] += acc_gangid() * 10 + acc_workerid();
    }
    
    KEEP(data[n/2]);
    free(data);
}

void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 4: Vector partitioning only */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(32)
    {
        int idx = acc_threadid();
        if (idx < n) data[idx] += acc_vectorid();
    }
    
    KEEP(data[n/2]);
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 5: Gang and vector partitioning */
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(1) vector_length(16)
    {
        int idx = acc_threadid();
        if (idx < n) data[idx] += acc_gangid() * 100 + acc_vectorid();
    }
    
    KEEP(data[n/2]);
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 6: Worker and vector partitioning */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(2) vector_length(8)
    {
        int idx = acc_threadid();
        if (idx < n) data[idx] += acc_workerid() * 10 + acc_vectorid();
    }
    
    KEEP(data[n/2]);
    free(data);
}

void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 7: Fully partitioned (gang, worker, vector) */
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(2) vector_length(4)
    {
        int idx = acc_threadid();
        if (idx < n) {
            data[idx] += acc_gangid() * 100 + acc_workerid() * 10 + acc_vectorid();
        }
    }
    
    KEEP(data[n/2]);
    free(data);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    /* Nested parallelism for combined partitioning */
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(64) simdlen(8) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += 1;
    }
    
    KEEP(data[n/2]);
    free(data);
}

/* Function that could generate invalid partition codes */
void test_invalid_partition(int code) {
    /* This might trigger the default case if code > 7 */
    volatile int partition = code;
    
    /* Use runtime condition to prevent optimization */
    if (partition > 7) {
        printf("Invalid partition code: %d\n", partition);
        /* This could trigger the "<illegal>" return */
    }
}

/* Template-like macro to generate different partition schemes */
#define GENERATE_PARTITION_TEST(level, n) \
    do { \
        int *arr = (int*)malloc((n) * sizeof(int)); \
        for (int i = 0; i < (n); i++) arr[i] = i; \
        \
        if ((level) == 0) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(1) num_workers(1) vector_length(1) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 1; } \
        } else if ((level) == 1) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(4) num_workers(1) vector_length(1) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 2; } \
        } else if ((level) == 2) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(1) num_workers(4) vector_length(1) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 3; } \
        } else if ((level) == 3) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(2) num_workers(2) vector_length(1) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 4; } \
        } else if ((level) == 4) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(1) num_workers(1) vector_length(32) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 5; } \
        } else if ((level) == 5) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(2) num_workers(1) vector_length(16) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 6; } \
        } else if ((level) == 6) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(1) num_workers(2) vector_length(8) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 7; } \
        } else if ((level) == 7) { \
            #pragma acc parallel copy(arr[0:(n)]) num_gangs(2) num_workers(2) vector_length(4) \
            { if (acc_threadid() < (n)) arr[acc_threadid()] += 8; } \
        } \
        KEEP(arr[(n)/2]); \
        free(arr); \
    } while(0)

int main() {
    int n = 1000;
    
    printf("Testing all partition mapping cases...\n");
    
    /* Test each partition type systematically */
    test_gang_redundant(n);
    test_gang_partitioned(n);
    test_worker_partitioned(n);
    test_gang_worker_partitioned(n);
    test_vector_partitioned(n);
    test_gang_vector_partitioned(n);
    test_worker_vector_partitioned(n);
    test_fully_partitioned(n);
    
    /* Test OpenMP partitioning */
    test_omp_partitioning(n);
    
    /* Test macro-based generation for all cases */
    for (int i = 0; i < 8; i++) {
        GENERATE_PARTITION_TEST(i, n);
    }
    
    /* Test potential invalid partition codes */
    test_invalid_partition(8);  /* Could trigger default case */
    test_invalid_partition(-1); /* Could trigger default case */
    
    /* Runtime-dependent partition selection */
    for (int i = 0; i < 10; i++) {
        int partition_type = i % 9;  /* 0-8, where 8 is invalid */
        
        /* This switch-like structure mirrors the uncovered code */
        switch (partition_type) {
            case 0: force_partition_code = 0; break;
            case 1: force_partition_code = 1; break;
            case 2: force_partition_code = 2; break;
            case 3: force_partition_code = 3; break;
            case 4: force_partition_code = 4; break;
            case 5: force_partition_code = 5; break;
            case 6: force_partition_code = 6; break;
            case 7: force_partition_code = 7; break;
            default: force_partition_code = 8; break;  /* Invalid */
        }
        
        /* Use the partition code in a way that can't be optimized away */
        KEEP(force_partition_code);
    }
    
    printf("All partition tests completed.\n");
    
    return 0;
}
