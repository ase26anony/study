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
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Test functions for each partition type */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 0: Single gang, redundant across gangs */
    #pragma acc parallel loop gang(num:1) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += 1;
    }
    
    /* Force partition logic with runtime check */
    if (force_runtime) {
        int sum = 0;
        for (int i = 0; i < n; i++) sum += data[i];
        KEEP_ALIVE(sum);
    }
    free(data);
}

void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 1: Multiple gangs, partitioned by gang */
    #pragma acc parallel loop gang(num:4) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
    }
    
    /* Dependency that forces gang partitioning */
    volatile int last = 0;
    #pragma acc parallel loop gang(num:4) copy(data[0:n])
    for (int i = 1; i < n; i++) {
        data[i] += data[i-1];
    }
    KEEP_ALIVE(last);
    
    free(data);
}

void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 2: Single gang, multiple workers */
    #pragma acc parallel loop gang(num:1) worker(num:4) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += i % 10;
    }
    
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 3: Multiple gangs and workers */
    #pragma acc parallel loop gang(num:2) worker(num:4) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 3 + 1;
    }
    
    /* Nested parallelism to force combined partitioning */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 4; w++) {
                int idx = g * (n/2) + w * (n/8);
                if (idx < n) data[idx] += g + w;
            }
        }
    }
    
    free(data);
}

void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 4: Vector partitioning only */
    #pragma acc parallel loop vector_length(32) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += (i & 0x1F);
    }
    
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 5: Gang + vector partitioning */
    #pragma acc parallel loop gang(num:2) vector_length(16) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = data[i] << 1;
    }
    
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 6: Worker + vector partitioning */
    #pragma acc parallel loop gang(num:1) worker(num:2) vector_length(8) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += (i % 8) * 2;
    }
    
    free(data);
}

void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Case 7: Fully partitioned (gang + worker + vector) */
    #pragma acc parallel loop gang(num:2) worker(num:2) vector_length(4) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        int g = (i / (n/2)) % 2;
        int w = (i / (n/4)) % 2;
        int v = i % 4;
        data[i] = g * 100 + w * 10 + v;
    }
    
    /* OpenMP equivalent to trigger same logic */
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(8) simdlen(4) map(tofrom:data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += 1;
    }
    
    free(data);
}

/* Test invalid partition codes (default case) */
void test_invalid_partitions(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    
    /* Force generation of partition mapping with invalid codes */
    int invalid_mode = 8;  /* Beyond valid range */
    
    /* Use macro to generate different partitionings based on runtime value */
    #define EXEC_PARTITION(mode) \
        if (mode == 0) { \
            #pragma acc parallel loop gang(num:1) copy(data[0:n]) \
            for (int i = 0; i < n; i++) data[i] = i; \
        } else if (mode == 8) { \
            /* Invalid partitioning - should trigger default case */ \
            #pragma acc parallel loop gang(num:-1) copy(data[0:n]) \
            for (int i = 0; i < n; i++) data[i] = i; \
        }
    
    EXEC_PARTITION(invalid_mode);
    
    /* Boundary violation through variable */
    int dynamic_gangs = n > 1000 ? 8 : -1;
    #pragma acc parallel loop gang(num:dynamic_gangs) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    free(data);
}

/* OpenMP versions to cover alternative code paths */
void test_omp_partitions(int n) {
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Various OpenMP partitionings */
    #pragma omp target teams distribute parallel for \
        num_teams(1) thread_limit(1) map(tofrom:a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] += 1;
    }
    
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(1) map(tofrom:a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] *= 2;
    }
    
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(4) simdlen(8) map(tofrom:b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + b[i];
    }
    
    free(a);
    free(b);
}

/* Main test driver */
int main(int argc, char **argv) {
    int n = 1024;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 128) n = 128;
    
    printf("Testing partition mapping cases with n = %d\n", n);
    
    /* Execute all test cases sequentially */
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
    
    test_invalid_partitions(n);
    printf("Default case tested\n");
    
    test_omp_partitions(n);
    printf("OpenMP partitions tested\n");
    
    printf("All partition tests completed\n");
    
    /* Force runtime evaluation of partition strings */
    volatile int check = 0;
    #pragma acc parallel copy(check)
    {
        check = 1;
    }
    
    if (check) {
        printf("OpenACC runtime active\n");
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
