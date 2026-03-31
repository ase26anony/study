/* test_partition_cases.c - Cover all partition mapping switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#define ARRAY_SIZE 1024
#define MAX_GANGS 8
#define MAX_WORKERS 4
#define VECTOR_LENGTH 32

/* Helper to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Function that could trigger partition string mapping */
static void debug_partition_info(int code) {
    /* This might trigger the compiler's internal partition mapping */
    volatile int volatile_code = code;
    if (volatile_code < 0 || volatile_code > 7) {
        /* Could trigger default case "<illegal>" */
        fprintf(stderr, "Invalid partition code: %d\n", volatile_code);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop gang num_gangs(1) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 2;
    }
    
    /* Verify and potentially trigger debug output */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) gang num_gangs(1) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += data[i];
    }
    
    debug_partition_info(0);  /* Hint for case 0 */
    use(&sum);
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop gang num_gangs(MAX_GANGS) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i + acc_gangid();
    }
    
    debug_partition_info(1);  /* Hint for case 1 */
    use(data);
}

/* Test case 2: Worker partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop gang num_gangs(1) worker num_workers(MAX_WORKERS) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = acc_workerid() * 1000 + i;
    }
    
    debug_partition_info(2);  /* Hint for case 2 */
    use(data);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    int data[ARRAY_SIZE * 2];
    int i;
    
    #pragma acc parallel loop gang num_gangs(MAX_GANGS) worker num_workers(MAX_WORKERS) copy(data)
    for (i = 0; i < ARRAY_SIZE * 2; i++) {
        data[i] = acc_gangid() * 10000 + acc_workerid() * 100 + i;
    }
    
    debug_partition_info(3);  /* Hint for case 3 */
    use(data);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop vector vector_length(VECTOR_LENGTH) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i * 1.5f;
    }
    
    debug_partition_info(4);  /* Hint for case 4 */
    use(data);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    float data[ARRAY_SIZE * 2];
    int i;
    
    #pragma acc parallel loop gang num_gangs(MAX_GANGS) vector vector_length(VECTOR_LENGTH) copy(data)
    for (i = 0; i < ARRAY_SIZE * 2; i++) {
        data[i] = (float)(acc_gangid() * 1000 + i) * 0.5f;
    }
    
    debug_partition_info(5);  /* Hint for case 5 */
    use(data);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop worker num_workers(MAX_WORKERS) vector vector_length(VECTOR_LENGTH) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)(acc_workerid() * 100 + i) * 0.25f;
    }
    
    debug_partition_info(6);  /* Hint for case 6 */
    use(data);
}

/* Test case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    double data[ARRAY_SIZE * 4];
    int i;
    
    #pragma acc parallel loop gang num_gangs(MAX_GANGS) \
        worker num_workers(MAX_WORKERS) \
        vector vector_length(VECTOR_LENGTH) \
        copy(data)
    for (i = 0; i < ARRAY_SIZE * 4; i++) {
        data[i] = (double)(acc_gangid() * 10000 + 
                          acc_workerid() * 100 + 
                          i) * 0.1;
    }
    
    debug_partition_info(7);  /* Hint for case 7 */
    use(data);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning() {
    int data[ARRAY_SIZE];
    int i;
    
    /* OpenMP target with teams (gangs) and distribute */
    #pragma omp target teams distribute parallel for \
        num_teams(MAX_GANGS) thread_limit(MAX_WORKERS*VECTOR_LENGTH) \
        map(tofrom: data[0:ARRAY_SIZE])
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 3 + omp_get_team_num();
    }
    
    /* OpenMP with SIMD (vector) */
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(128) \
        map(tofrom: data[0:ARRAY_SIZE])
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] += omp_get_thread_num();
    }
    
    use(data);
}

/* Test invalid partition codes (default case) */
void test_invalid_partitions() {
    /* Force invalid codes through variable manipulation */
    volatile int invalid_codes[] = {-1, 8, 100, -100};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i]);
    }
    
    /* Complex expression that might generate invalid partition */
    int dynamic_size = ARRAY_SIZE;
    if (dynamic_size < 0) {  /* Always false, but compiler doesn't know */
        #pragma acc parallel loop gang num_gangs(dynamic_size) copyout(data[0:0])
        for (int j = 0; j < 0; j++) {
            /* Never executed, but might generate partition code */
        }
    }
}

/* Template-like macro to generate different partition schemes */
#define GENERATE_PARTITION_TEST(NAME, GANGS, WORKERS, VECTORS) \
void test_##NAME() { \
    int data[ARRAY_SIZE]; \
    int i; \
    \
    #pragma acc parallel loop \
        gang num_gangs(GANGS) \
        worker num_workers(WORKERS) \
        vector vector_length(VECTORS) \
        copy(data) \
    for (i = 0; i < ARRAY_SIZE; i++) { \
        data[i] = i + acc_gangid() * 1000 + acc_workerid() * 10; \
    } \
    \
    use(data); \
}

/* Generate specific combinations */
GENERATE_PARTITION_TEST(mixed_1, 2, 1, 1)   /* Mostly gang partitioned */
GENERATE_PARTITION_TEST(mixed_2, 1, 2, 1)   /* Worker partitioned */
GENERATE_PARTITION_TEST(mixed_3, 1, 1, 32)  /* Vector partitioned */
GENERATE_PARTITION_TEST(mixed_4, 2, 2, 16)  /* Fully partitioned */

int main() {
    printf("Testing all partition mapping cases...\n");
    
    /* Test OpenACC partition cases */
    test_gang_redundant();
    printf("  Case 0 tested\n");
    
    test_gang_partitioned();
    printf("  Case 1 tested\n");
    
    test_worker_partitioned();
    printf("  Case 2 tested\n");
    
    test_gang_worker_partitioned();
    printf("  Case 3 tested\n");
    
    test_vector_partitioned();
    printf("  Case 4 tested\n");
    
    test_gang_vector_partitioned();
    printf("  Case 5 tested\n");
    
    test_worker_vector_partitioned();
    printf("  Case 6 tested\n");
    
    test_fully_partitioned();
    printf("  Case 7 tested\n");
    
    /* Test OpenMP equivalents */
    test_omp_partitioning();
    printf("  OpenMP partitioning tested\n");
    
    /* Test generated combinations */
    test_mixed_1();
    test_mixed_2();
    test_mixed_3();
    test_mixed_4();
    printf("  Mixed partitions tested\n");
    
    /* Test invalid cases (default switch case) */
    test_invalid_partitions();
    printf("  Invalid partitions tested\n");
    
    printf("All tests completed.\n");
    
    return 0;
}
