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

/* Test functions for each partition type */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 0: Single gang, redundant across gangs */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        int idx = acc_gang_id * 1000 + acc_worker_id * 100 + acc_vector_id;
        if (idx < n) data[idx] += 1;
    }
    
    /* Force partition mapping */
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 1: Multiple gangs, partitioned by gang */
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(1)
    {
        int gang_id = acc_gang_id;
        if (gang_id * 256 < n) {
            for (int i = gang_id * 256; i < (gang_id + 1) * 256 && i < n; i++) {
                data[i] *= 2;
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 2: Single gang, multiple workers */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(8) vector_length(1)
    {
        int worker_id = acc_worker_id;
        if (worker_id * 128 < n) {
            for (int i = worker_id * 128; i < (worker_id + 1) * 128 && i < n; i++) {
                data[i] += worker_id;
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 3: Both gang and worker partitioned */
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(4) vector_length(1)
    {
        int gang_id = acc_gang_id;
        int worker_id = acc_worker_id;
        int idx = gang_id * 1024 + worker_id * 64;
        if (idx < n) {
            for (int i = 0; i < 64 && (idx + i) < n; i++) {
                data[idx + i] += gang_id + worker_id;
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 4: Vector partitioned only */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(256)
    {
        int vec_id = acc_vector_id;
        if (vec_id < n) {
            data[vec_id] <<= 1;
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 5: Gang and vector partitioned */
    #pragma acc parallel copy(data[0:n]) num_gangs(8) num_workers(1) vector_length(32)
    {
        int gang_id = acc_gang_id;
        int vec_id = acc_vector_id;
        int idx = gang_id * 32 + vec_id;
        if (idx < n) {
            data[idx] += gang_id * vec_id;
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 6: Worker and vector partitioned */
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(16) vector_length(16)
    {
        int worker_id = acc_worker_id;
        int vec_id = acc_vector_id;
        int idx = worker_id * 16 + vec_id;
        if (idx < n) {
            data[idx] -= worker_id + vec_id;
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 7: Fully partitioned (gang, worker, vector) */
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(4) vector_length(8)
    {
        int gang_id = acc_gang_id;
        int worker_id = acc_worker_id;
        int vec_id = acc_vector_id;
        int idx = gang_id * 256 + worker_id * 16 + vec_id;
        if (idx < n) {
            data[idx] = gang_id * 100 + worker_id * 10 + vec_id;
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

/* OpenMP equivalents to trigger different partition mappings */
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* OpenMP target with teams - can trigger various partition mappings */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n]) num_teams(4) thread_limit(64)
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() * 100 + omp_get_thread_num();
    }
    
    /* Nested parallelism for combined partitioning */
    #pragma omp target teams distribute simd \
        map(tofrom: data[0:n]) num_teams(8)
    for (int i = 0; i < n; i++) {
        #pragma omp parallel for simd
        for (int j = 0; j < 4; j++) {
            if (i * 4 + j < n) {
                data[i * 4 + j] *= 2;
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < n; i++) check += data[i];
    NO_OPTIMIZE(check);
    free(data);
}

/* Function that could generate invalid partition codes */
void test_invalid_partition(int mode) {
    /* This might trigger the default case if mode is out of bounds */
    int n = 1024;
    int *data = (int*)malloc(n * sizeof(int));
    
    /* Dynamic partitioning based on runtime input */
    if (mode == 0) {
        #pragma acc parallel copy(data[0:n]) num_gangs(2)
        for (int i = 0; i < n; i++) data[i] = i;
    } else if (mode == 1) {
        #pragma acc parallel copy(data[0:n]) num_workers(4)
        for (int i = 0; i < n; i++) data[i] = i * 2;
    } else if (mode == 2) {
        #pragma acc parallel copy(data[0:n]) vector_length(128)
        for (int i = 0; i < n; i++) data[i] = i * 3;
    }
    /* No else - could leave partition unspecified */
    
    free(data);
}

/* Template-like macro for different partition schemes */
#define TEST_PARTITION_SCHEME(scheme_id, n_gangs, n_workers, vec_len) \
    do { \
        int size = 4096; \
        int *arr = (int*)malloc(size * sizeof(int)); \
        for (int i = 0; i < size; i++) arr[i] = i; \
        \
        #pragma acc parallel copy(arr[0:size]) \
            num_gangs(n_gangs) num_workers(n_workers) vector_length(vec_len) \
        { \
            int g = acc_gang_id; \
            int w = acc_worker_id; \
            int v = acc_vector_id; \
            int idx = g * n_workers * vec_len + w * vec_len + v; \
            if (idx < size) arr[idx] += scheme_id; \
        } \
        \
        int sum = 0; \
        for (int i = 0; i < size; i++) sum += arr[i]; \
        NO_OPTIMIZE(sum); \
        free(arr); \
    } while(0)

int main(int argc, char **argv) {
    int test_size = 8192;
    if (argc > 1) test_size = atoi(argv[1]);
    if (test_size < 1024) test_size = 1024;
    
    printf("Testing partition mapping coverage...\n");
    
    /* Test all valid partition cases (0-7) */
    test_gang_redundant(test_size);
    printf("  Case 0 tested\n");
    
    test_gang_partitioned(test_size);
    printf("  Case 1 tested\n");
    
    test_worker_partitioned(test_size);
    printf("  Case 2 tested\n");
    
    test_gang_worker_partitioned(test_size);
    printf("  Case 3 tested\n");
    
    test_vector_partitioned(test_size);
    printf("  Case 4 tested\n");
    
    test_gang_vector_partitioned(test_size);
    printf("  Case 5 tested\n");
    
    test_worker_vector_partitioned(test_size);
    printf("  Case 6 tested\n");
    
    test_fully_partitioned(test_size);
    printf("  Case 7 tested\n");
    
    /* Test OpenMP variants */
    test_omp_partitioning(test_size);
    printf("  OpenMP partitioning tested\n");
    
    /* Test with macro expansions covering different schemes */
    TEST_PARTITION_SCHEME(0, 1, 1, 1);      /* gang redundant */
    TEST_PARTITION_SCHEME(1, 4, 1, 1);      /* gang partitioned */
    TEST_PARTITION_SCHEME(2, 1, 8, 1);      /* worker partitioned */
    TEST_PARTITION_SCHEME(3, 2, 4, 1);      /* gang+worker partitioned */
    TEST_PARTITION_SCHEME(4, 1, 1, 256);    /* vector partitioned */
    TEST_PARTITION_SCHEME(5, 4, 1, 64);     /* gang+vector partitioned */
    TEST_PARTITION_SCHEME(6, 1, 4, 64);     /* worker+vector partitioned */
    TEST_PARTITION_SCHEME(7, 2, 2, 32);     /* fully partitioned */
    printf("  Macro-based schemes tested\n");
    
    /* Potentially trigger default case with unusual inputs */
    for (int i = 0; i < 3; i++) {
        test_invalid_partition(i);
    }
    printf("  Boundary cases tested\n");
    
    printf("All partition tests completed.\n");
    
    return 0;
}

#ifdef __cplusplus
}
#endif
