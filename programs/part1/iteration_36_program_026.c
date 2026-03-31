/* test_partition_cases.c - Cover all partition mapping switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prevent optimization */
volatile int force_runtime = 1;
#define NO_OPTIMIZE(var) asm volatile("" : : "r"(var))

/* Test functions for different partition types */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 0: Single gang, redundant across gangs */
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1;
        }
    }
    
    /* Force partition mapping */
    if (force_runtime > 1000) printf("gang redundant\n");
    free(data);
}

void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 1: Multiple gangs, partitioned by gang */
    #pragma acc parallel num_gangs(4) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += acc_gang_id();
        }
    }
    
    NO_OPTIMIZE(data[0]);
    free(data);
}

void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 2: Single gang with workers, partitioned by worker */
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            if (acc_worker_id() == 0) data[i] *= 2;
        }
    }
    
    if (force_runtime) printf("worker partitioned\n");
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 3: Both gang and worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) copy(data[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] += acc_gang_id() * 10 + acc_worker_id();
        }
    }
    
    NO_OPTIMIZE(data[n/2]);
    free(data);
}

void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 4: Vector partitioned only */
    #pragma acc parallel vector_length(32) copy(data[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] += acc_vector_id();
        }
    }
    
    if (force_runtime < 0) printf("vector partitioned\n");
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 5: Gang and vector partitioned */
    #pragma acc parallel num_gangs(2) vector_length(16) copy(data[0:n])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] += acc_gang_id() * 100 + acc_vector_id();
        }
    }
    
    NO_OPTIMIZE(data[1]);
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 6: Worker and vector partitioned */
    #pragma acc parallel num_workers(2) vector_length(8) copy(data[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] += acc_worker_id() * 10 + acc_vector_id();
        }
    }
    
    if (force_runtime != 0) printf("worker+vector partitioned\n");
    free(data);
}

void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 7: Fully partitioned (gang, worker, vector) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(4) copy(data[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            int idx = acc_gang_id() * 100 + acc_worker_id() * 10 + acc_vector_id();
            data[i] = (data[i] + idx) % 1000;
        }
    }
    
    NO_OPTIMIZE(data[n-1]);
    free(data);
}

/* OpenMP equivalents to trigger similar logic */
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* OpenMP target with teams and distribute - triggers various partition mappings */
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(64) simdlen(8) \
        map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() * 100 + omp_get_thread_num() % 10;
    }
    
    /* Nested parallel regions for combined partitioning */
    #pragma omp target teams distribute parallel for simd \
        collapse(2) num_teams(4) thread_limit(32) \
        map(tofrom: data[0:n])
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            data[idx] = (data[idx] + i + j) % 100;
        }
    }
    
    free(data);
}

/* Function that could generate invalid partition codes */
void test_invalid_partition(int code) {
    /* This might trigger the default case if code > 7 */
    volatile int partition = code;
    
    /* Force compiler to consider all possible values */
    switch (partition) {
        case 0: case 1: case 2: case 3:
        case 4: case 5: case 6: case 7:
            printf("Valid partition code\n");
            break;
        default:
            /* This could trigger <illegal> return */
            if (force_runtime) printf("Invalid partition code\n");
    }
}

/* Template/macro approach for compile-time variations */
#define GENERATE_PARTITION_TEST(level) \
    void test_partition_##level(int n) { \
        int *data = (int*)malloc(n * sizeof(int)); \
        for (int i = 0; i < n; i++) data[i] = i; \
        \
        if (level == 0) { \
            #pragma acc parallel num_gangs(1) copy(data[0:n]) \
            { \
                #pragma acc loop gang \
                for (int i = 0; i < n; i++) data[i]++; \
            } \
        } else if (level == 1) { \
            #pragma acc parallel num_gangs(4) copy(data[0:n]) \
            { \
                #pragma acc loop gang \
                for (int i = 0; i < n; i++) data[i] += 2; \
            } \
        } \
        free(data); \
    }

/* Generate specific test functions */
GENERATE_PARTITION_TEST(0)
GENERATE_PARTITION_TEST(1)

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    printf("Testing partition mapping cases...\n");
    
    /* Test all switch cases systematically */
    test_gang_redundant(n);          /* Case 0 */
    test_gang_partitioned(n);        /* Case 1 */
    test_worker_partitioned(n);      /* Case 2 */
    test_gang_worker_partitioned(n); /* Case 3 */
    test_vector_partitioned(n);      /* Case 4 */
    test_gang_vector_partitioned(n); /* Case 5 */
    test_worker_vector_partitioned(n); /* Case 6 */
    test_fully_partitioned(n);       /* Case 7 */
    
    /* Additional OpenMP tests */
    test_omp_partitioning(n);
    
    /* Test with compile-time generated functions */
    test_partition_0(n/2);
    test_partition_1(n/2);
    
    /* Test boundary/invalid cases */
    for (int i = 0; i < 10; i++) {
        test_invalid_partition(i);
    }
    
    /* Runtime-dependent partitioning */
    int mode = force_runtime ? 1 : 2;
    if (mode == 1) {
        #pragma acc parallel num_gangs(2) num_workers(2)
        {
            /* Mixed partitioning */
            #pragma acc loop gang
            for (int i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (int j = 0; j < 10; j++) {
                    /* Do nothing, just structure */
                    asm volatile("" : : : "memory");
                }
            }
        }
    }
    
    printf("All partition tests completed.\n");
    return 0;
}

#ifdef __cplusplus
}
#endif
