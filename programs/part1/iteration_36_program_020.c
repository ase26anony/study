/* test_partition_coverage.c
 * Compile with: gcc -O2 -fopenacc -fopenmp -o test_partition test_partition_coverage.c
 * For GCC coverage: add -fdump-tree-omplower -g
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#define N 1024
#define MAX_GANGS 8
#define MAX_WORKERS 4
#define VECTOR_LEN 32

/* Prevent optimization */
volatile int force_runtime = 1;

/* Function to use results and prevent dead code elimination */
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    int data[N];
    int i;
    
    /* Initialize with runtime values */
    for (i = 0; i < N; i++) {
        data[i] = i * (force_runtime + 1);
    }
    
    #pragma acc parallel num_gangs(1) copy(data[0:N])
    {
        int gang_id = acc_gang_id();
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] += gang_id;  /* All gangs see same value */
        }
    }
    
    /* Use result to prevent optimization */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += data[i];
    }
    use_result(sum);
    
    printf("Test 0 (gang redundant) completed\n");
}

/* Test case 1: Gang partitioned */
void test_gang_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel num_gangs(4) copy(data[0:N])
    {
        int gang_id = acc_gang_id();
        #pragma acc loop gang
        for (i = gang_id * (N/4); i < (gang_id + 1) * (N/4); i++) {
            if (i < N) {
                data[i] += gang_id * 1000;
            }
        }
    }
    
    /* Verification */
    int errors = 0;
    for (i = 0; i < N; i++) {
        int expected_gang = i / (N/4);
        if (expected_gang >= 4) expected_gang = 3;
        if (data[i] != i + expected_gang * 1000) {
            errors++;
        }
    }
    
    if (errors > 0) {
        printf("Test 1: Partition error detected (gang partitioned)\n");
    }
    printf("Test 1 (gang partitioned) completed\n");
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:N])
    {
        int worker_id = acc_worker_id();
        #pragma acc loop worker
        for (i = worker_id * (N/4); i < (worker_id + 1) * (N/4); i++) {
            if (i < N) {
                data[i] += worker_id * 100;
            }
        }
    }
    
    use_result(data[N-1]);
    printf("Test 2 (worker partitioned) completed\n");
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel num_gangs(2) num_workers(2) copy(data[0:N])
    {
        int gang_id = acc_gang_id();
        int worker_id = acc_worker_id();
        int chunk = N / (2 * 2);
        int start = (gang_id * 2 + worker_id) * chunk;
        int end = start + chunk;
        
        #pragma acc loop gang worker
        for (i = start; i < end && i < N; i++) {
            data[i] += gang_id * 1000 + worker_id * 100;
        }
    }
    
    printf("Test 3 (gang+worker partitioned) completed\n");
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel vector_length(VECTOR_LEN) copy(data[0:N])
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            data[i] += acc_vector_id();
        }
    }
    
    use_result(data[0]);
    printf("Test 4 (vector partitioned) completed\n");
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel num_gangs(4) vector_length(VECTOR_LEN) copy(data[0:N])
    {
        int gang_id = acc_gang_id();
        int vector_id = acc_vector_id();
        
        #pragma acc loop gang vector
        for (i = gang_id * (N/4) + vector_id; 
             i < (gang_id + 1) * (N/4); 
             i += VECTOR_LEN) {
            if (i < N) {
                data[i] += gang_id * 1000 + vector_id;
            }
        }
    }
    
    printf("Test 5 (gang+vector partitioned) completed\n");
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(VECTOR_LEN) copy(data[0:N])
    {
        int worker_id = acc_worker_id();
        int vector_id = acc_vector_id();
        
        #pragma acc loop worker vector
        for (i = worker_id * (N/2) + vector_id; 
             i < (worker_id + 1) * (N/2); 
             i += VECTOR_LEN) {
            if (i < N) {
                data[i] += worker_id * 100 + vector_id;
            }
        }
    }
    
    printf("Test 6 (worker+vector partitioned) completed\n");
}

/* Test case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    int data[N];
    int i;
    
    for (i = 0; i < N; i++) {
        data[i] = i;
    }
    
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) copy(data[0:N])
    {
        int gang_id = acc_gang_id();
        int worker_id = acc_worker_id();
        int vector_id = acc_vector_id();
        int total_workers = 2 * 2;  /* gangs * workers per gang */
        int worker_global = gang_id * 2 + worker_id;
        int chunk = N / total_workers;
        int start = worker_global * chunk + vector_id;
        
        #pragma acc loop gang worker vector
        for (i = start; i < (worker_global + 1) * chunk; i += 16) {
            if (i < N) {
                data[i] += gang_id * 1000 + worker_id * 100 + vector_id;
            }
        }
    }
    
    /* Complex verification that might trigger diagnostic output */
    int errors = 0;
    for (i = 0; i < N; i++) {
        if (data[i] < i) {
            errors++;
            if (errors == 1) {
                /* This could trigger partition string usage in diagnostics */
                printf("Error in fully partitioned computation at index %d\n", i);
            }
        }
    }
    
    printf("Test 7 (fully partitioned) completed with %d errors\n", errors);
}

/* Test default case: Invalid partition codes */
void test_invalid_partitions() {
    /* Force generation of invalid partition codes through macro expansion */
    #define TRY_PARTITION(code) \
        do { \
            int p = (code); \
            if (p > 7 || p < 0) { \
                printf("Invalid partition code: %d\n", p); \
            } \
        } while(0)
    
    /* Generate boundary violations */
    TRY_PARTITION(-1);
    TRY_PARTITION(8);
    TRY_PARTITION(255);
    
    /* Runtime-determined invalid code */
    int invalid_code = force_runtime ? 100 : 0;
    TRY_PARTITION(invalid_code);
    
    printf("Invalid partition tests completed\n");
}

/* OpenMP equivalents to trigger different partitioning */
void test_omp_partitioning() {
    int data[N];
    int i;
    
    /* OpenMP target with teams (gangs) and distribute */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:N]) num_teams(4) thread_limit(128)
    for (i = 0; i < N; i++) {
        data[i] = i * omp_get_team_num();
    }
    
    /* Nested parallelism for combined partitioning */
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:N]) \
            num_teams(2) thread_limit(64) simdlen(8)
    for (i = 0; i < N; i++) {
        data[i] += omp_get_team_num() * 1000 + omp_get_thread_num() % 8;
    }
    
    use_result(data[N/2]);
    printf("OpenMP partitioning tests completed\n");
}

/* Template-like macro to generate different partition configurations */
#define GENERATE_PARTITION_TEST(NAME, GANGS, WORKERS, VECTORS) \
void NAME() { \
    int data[64]; \
    int i; \
    for (i = 0; i < 64; i++) data[i] = i; \
    \
    if (force_runtime) { \
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS) copy(data[0:64]) \
        { \
            #pragma acc loop gang worker vector \
            for (i = 0; i < 64; i++) { \
                data[i] += acc_gang_id() * 100 + acc_worker_id() * 10 + acc_vector_id(); \
            } \
        } \
    } \
    use_result(data[0]); \
}

/* Generate specific configurations */
GENERATE_PARTITION_TEST(test_mixed_1, 2, 1, 1)   /* Gang only */
GENERATE_PARTITION_TEST(test_mixed_2, 1, 2, 1)   /* Worker only */
GENERATE_PARTITION_TEST(test_mixed_3, 1, 1, 8)   /* Vector only */
GENERATE_PARTITION_TEST(test_mixed_4, 2, 2, 1)   /* Gang+Worker */
GENERATE_PARTITION_TEST(test_mixed_5, 2, 1, 8)   /* Gang+Vector */
GENERATE_PARTITION_TEST(test_mixed_6, 1, 2, 8)   /* Worker+Vector */
GENERATE_PARTITION_TEST(test_mixed_7, 2, 2, 8)   /* Fully partitioned */

int main() {
    printf("Starting partition coverage tests...\n");
    
    /* Test all OpenACC partition cases */
    test_gang_redundant();          /* Case 0 */
    test_gang_partitioned();        /* Case 1 */
    test_worker_partitioned();      /* Case 2 */
    test_gang_worker_partitioned(); /* Case 3 */
    test_vector_partitioned();      /* Case 4 */
    test_gang_vector_partitioned(); /* Case 5 */
    test_worker_vector_partitioned(); /* Case 6 */
    test_fully_partitioned();       /* Case 7 */
    
    /* Test invalid/default case */
    test_invalid_partitions();
    
    /* Test OpenMP variants */
    test_omp_partitioning();
    
    /* Test macro-generated configurations */
    test_mixed_1();
    test_mixed_2();
    test_mixed_3();
    test_mixed_4();
    test_mixed_5();
    test_mixed_6();
    test_mixed_7();
    
    printf("\nAll partition tests completed.\n");
    printf("To ensure coverage instrumentation, compile with:\n");
    printf("  gcc -O2 -fopenacc -fopenmp -fdump-tree-omplower -g test_partition_coverage.c\n");
    
    return 0;
}
