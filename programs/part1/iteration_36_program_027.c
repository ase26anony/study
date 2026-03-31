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

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    volatile int partition_hint = 0;
    int data[ARRAY_SIZE];
    
    #pragma acc parallel loop gang num_gangs(1) copy(data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 2;
    }
    
    /* Force partition logic */
    if (partition_hint == 0) {
        /* This could trigger "gang redundant" string */
        printf("Testing gang redundant partitioning\n");
    }
    use(&data);
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    int data[ARRAY_SIZE];
    int num_gangs = 4;
    
    #pragma acc parallel loop gang num_gangs(num_gangs) copy(data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = acc_gang(1) * 1000 + i;
    }
    
    /* Runtime check that forces partition analysis */
    volatile int check = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] < 0) check = 1;
    }
    use(&check);
}

/* Test case 2: Worker partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    int data[ARRAY_SIZE];
    
    #pragma acc parallel loop gang(1) worker num_workers(4) copy(data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = acc_worker(1) * 100 + i;
    }
    
    /* Conditional that could use partition description */
    if (acc_get_num_workers(acc_async_noval) > 1) {
        printf("Worker partitioning active\n");
    }
    use(&data);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    int data[ARRAY_SIZE * 2];
    int gangs = 2, workers = 2;
    
    #pragma acc parallel loop gang(gangs) worker(workers) copy(data)
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        data[i] = acc_gang(1) * 1000 + acc_worker(1) * 100 + i;
    }
    
    /* Complex dependency to force partitioning */
    volatile int sum = 0;
    #pragma acc parallel loop reduction(+:sum) gang(gangs) worker(workers)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += data[i] % 17;
    }
    use(&sum);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    float data[ARRAY_SIZE];
    
    #pragma acc parallel loop vector vector_length(VECTOR_LENGTH) copy(data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 3.14f;
    }
    
    /* Vector-specific operation */
    #pragma acc parallel loop vector_length(VECTOR_LENGTH)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = data[i] * 2.0f;
    }
    use(&data);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    double data[ARRAY_SIZE];
    int gangs = 4;
    
    #pragma acc parallel loop gang(gangs) vector vector_length(16) copy(data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = acc_gang(1) * 1.5 + i * 0.01;
    }
    
    /* Nested parallelism triggering combined partitioning */
    volatile double total = 0.0;
    #pragma acc parallel loop gang(gangs) vector reduction(+:total)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += data[i];
    }
    use(&total);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    int data[ARRAY_SIZE];
    
    #pragma acc parallel loop worker vector vector_length(8) copy(data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = acc_worker(1) * 50 + (i % VECTOR_LENGTH);
    }
    
    /* Multi-dimensional access pattern */
    for (int iter = 0; iter < 3; iter++) {
        #pragma acc parallel loop worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] += iter;
        }
    }
    use(&data);
}

/* Test case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    int data[ARRAY_SIZE * 4];
    int gangs = 2, workers = 2, vec_len = 8;
    
    #pragma acc parallel loop gang(gangs) worker(workers) vector vector_length(vec_len) copy(data)
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        int g = acc_gang(1);
        int w = acc_worker(1);
        int v = i % vec_len;
        data[i] = g * 10000 + w * 1000 + v * 10 + (i % 100);
    }
    
    /* Complex reduction with all levels */
    volatile long long mega_sum = 0;
    #pragma acc parallel loop gang(gangs) worker(workers) vector reduction(+:mega_sum)
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        mega_sum += data[i];
    }
    
    /* Conditional error reporting that could use partition strings */
    if (mega_sum == 0) {
        printf("Error in fully partitioned computation\n");
    }
    use(&mega_sum);
}

/* OpenMP equivalents to trigger same logic */
void test_omp_partitioning() {
    int data[ARRAY_SIZE];
    
    /* OpenMP target with teams (gangs) and distribute */
    #pragma omp target teams distribute parallel for num_teams(4) thread_limit(32) map(tofrom:data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num() * 10 + i;
    }
    
    /* Combined SIMD (vector) with teams */
    #pragma omp target teams distribute simd num_teams(2) simdlen(8) map(tofrom:data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] += i;
    }
    use(&data);
}

/* Generate invalid partition codes for default case */
void test_invalid_partitions() {
    /* Force invalid partition through macro expansion */
    #define INVALID_PARTITION 99
    
    volatile int invalid_code = INVALID_PARTITION;
    
    /* This might trigger the default "<illegal>" case if the compiler
       tries to map an invalid partition code */
    if (invalid_code > 7) {
        /* Could trigger error reporting with partition string */
        printf("Invalid partition code encountered\n");
    }
    
    /* Boundary violation through runtime variable */
    int dynamic_partition = 8; /* Just outside valid range */
    volatile int *volatile ptr = &dynamic_partition;
    use(ptr);
}

/* Template-like macro to generate different partitionings */
#define GENERATE_PARTITION_TEST(level) \
    do { \
        int arr[256]; \
        _Pragma("acc parallel loop " level " copy(arr)") \
        for (int i = 0; i < 256; i++) { \
            arr[i] = i * level; \
        } \
        use(&arr); \
    } while(0)

void test_macro_expansion() {
    /* Generate different partitionings via macro */
    GENERATE_PARTITION_TEST("gang(1)");
    GENERATE_PARTITION_TEST("gang(2)");
    GENERATE_PARTITION_TEST("worker");
    GENERATE_PARTITION_TEST("vector");
}

int main() {
    printf("Testing all partition mapping cases...\n");
    
    /* Execute all test cases systematically */
    test_gang_redundant();        /* Case 0 */
    test_gang_partitioned();      /* Case 1 */
    test_worker_partitioned();    /* Case 2 */
    test_gang_worker_partitioned(); /* Case 3 */
    test_vector_partitioned();    /* Case 4 */
    test_gang_vector_partitioned(); /* Case 5 */
    test_worker_vector_partitioned(); /* Case 6 */
    test_fully_partitioned();     /* Case 7 */
    
    /* Additional tests for coverage */
    test_omp_partitioning();
    test_invalid_partitions();
    test_macro_expansion();
    
    printf("All partition tests completed.\n");
    
    /* Final check that forces all code paths */
    volatile int final_check = 0;
    #pragma acc parallel loop reduction(max:final_check)
    for (int i = 0; i < 100; i++) {
        if (i > final_check) final_check = i;
    }
    
    return final_check == 99 ? 0 : 1;
}
