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
volatile int anti_opt = 0;

/* Function that could trigger partition string mapping */
void debug_partition_info(int code) {
    /* This mimics the internal compiler logic */
    const char* desc;
    switch(code) {
        case 0: desc = "gang redundant"; break;
        case 1: desc = "gang partitioned"; break;
        case 2: desc = "worker partitioned"; break;
        case 3: desc = "gang+worker partitioned"; break;
        case 4: desc = "vector partitioned"; break;
        case 5: desc = "gang+vector partitioned"; break;
        case 6: desc = "worker+vector partitioned"; break;
        case 7: desc = "fully partitioned"; break;
        default: desc = "<illegal>"; break;
    }
    
    /* Force compiler to consider this logic */
    if (anti_opt) printf("Partition: %s\n", desc);
}

/* Case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop gang num_gangs(1) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 2;
    }
    
    /* Verify and potentially trigger debug output */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) gang num_gangs(1)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += data[i];
    }
    
    if (sum != (ARRAY_SIZE-1)*ARRAY_SIZE) {
        debug_partition_info(0);  /* Could trigger case 0 */
    }
}

/* Case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    int gang_size = 4;
    
    #pragma acc parallel loop gang num_gangs(gang_size) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        int gang_id = acc_gang();
        data[i] = gang_id * 1000 + i;
    }
    
    /* Complex dependency forcing gang partitioning */
    int temp[ARRAY_SIZE];
    #pragma acc parallel loop gang num_gangs(gang_size) copy(temp)
    for (i = 1; i < ARRAY_SIZE; i++) {
        temp[i] = data[i] + data[i-1];  /* Cross-gang dependency */
    }
    
    if (temp[ARRAY_SIZE/2] > 0) {
        debug_partition_info(1);  /* Could trigger case 1 */
    }
}

/* Case 2: Worker partitioned */
void test_worker_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    /* Single gang, multiple workers */
    #pragma acc parallel loop gang(1) worker num_workers(4) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = acc_worker() * 100.0f + i;
    }
    
    /* Worker-specific reduction */
    float worker_sums[4] = {0};
    #pragma acc parallel loop gang(1) worker num_workers(4) \
        copy(worker_sums) reduction(+:worker_sums[:4])
    for (i = 0; i < ARRAY_SIZE; i++) {
        int wid = acc_worker();
        worker_sums[wid] += data[i];
    }
    
    if (worker_sums[0] > 0) {
        debug_partition_info(2);  /* Could trigger case 2 */
    }
}

/* Case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    double data[ARRAY_SIZE * 2];
    int i;
    
    /* Both gang and worker partitioning */
    #pragma acc parallel loop gang(2) worker num_workers(2) copy(data)
    for (i = 0; i < ARRAY_SIZE * 2; i++) {
        int gid = acc_gang();
        int wid = acc_worker();
        data[i] = gid * 1000.0 + wid * 100.0 + i;
    }
    
    /* Nested parallelism requiring both levels */
    double result[4] = {0};
    #pragma acc parallel num_gangs(2) num_workers(2) copy(result)
    {
        int gid = acc_gang();
        int wid = acc_worker();
        int idx = gid * 2 + wid;
        
        #pragma acc loop vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            result[idx] += data[gid * ARRAY_SIZE + i] * (wid + 1);
        }
    }
    
    if (result[0] > 0) {
        debug_partition_info(3);  /* Could trigger case 3 */
    }
}

/* Case 4: Vector partitioned */
void test_vector_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    /* Vector-level parallelism only */
    #pragma acc parallel loop vector vector_length(VECTOR_LENGTH) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * i;
    }
    
    /* Vector operations with dependencies */
    int temp[ARRAY_SIZE];
    #pragma acc parallel loop vector vector_length(VECTOR_LENGTH) \
        copy(temp) independent
    for (i = 0; i < ARRAY_SIZE; i++) {
        temp[i] = data[i] * 2 - data[(i + 1) % ARRAY_SIZE];
    }
    
    if (temp[0] != -1) {
        debug_partition_info(4);  /* Could trigger case 4 */
    }
}

/* Case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    /* Gang and vector partitioning */
    #pragma acc parallel loop gang vector \
        num_gangs(4) vector_length(16) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        int gid = acc_gang();
        data[i] = gid * 100.0f + i * 0.5f;
    }
    
    /* Reduction requiring both gang and vector */
    float gang_sums[4] = {0};
    #pragma acc parallel num_gangs(4) copy(gang_sums)
    {
        int gid = acc_gang();
        
        #pragma acc loop vector reduction(+:gang_sums[gid]) \
            vector_length(16)
        for (i = 0; i < ARRAY_SIZE/4; i++) {
            gang_sums[gid] += data[gid * ARRAY_SIZE/4 + i];
        }
    }
    
    if (gang_sums[0] > 0) {
        debug_partition_info(5);  /* Could trigger case 5 */
    }
}

/* Case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    double data[ARRAY_SIZE];
    int i;
    
    /* Worker and vector partitioning */
    #pragma acc parallel loop worker vector \
        num_workers(2) vector_length(8) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        int wid = acc_worker();
        data[i] = wid * 500.0 + i * 0.25;
    }
    
    /* Worker-private with vector operations */
    double worker_results[2] = {0};
    #pragma acc parallel num_workers(2) copy(worker_results)
    {
        int wid = acc_worker();
        
        #pragma acc loop vector reduction(+:worker_results[wid]) \
            vector_length(8)
        for (i = 0; i < ARRAY_SIZE/2; i++) {
            worker_results[wid] += data[wid * ARRAY_SIZE/2 + i];
        }
    }
    
    if (worker_results[0] > 0) {
        debug_partition_info(6);  /* Could trigger case 6 */
    }
}

/* Case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    int data[ARRAY_SIZE * 4];
    int i;
    
    /* All three levels of partitioning */
    #pragma acc parallel loop gang worker vector \
        num_gangs(2) num_workers(2) vector_length(4) copy(data)
    for (i = 0; i < ARRAY_SIZE * 4; i++) {
        int gid = acc_gang();
        int wid = acc_worker();
        int vid = acc_vector();
        data[i] = gid * 10000 + wid * 1000 + vid * 100 + i;
    }
    
    /* Complex nested computation */
    int result[8] = {0};
    #pragma acc parallel num_gangs(2) num_workers(2) copy(result)
    {
        int gid = acc_gang();
        int wid = acc_worker();
        int idx = gid * 2 + wid;
        
        #pragma acc loop vector reduction(+:result[idx]) \
            vector_length(4)
        for (i = 0; i < ARRAY_SIZE; i++) {
            int base = (gid * 2 + wid) * ARRAY_SIZE;
            result[idx] += data[base + i] % 100;
        }
    }
    
    if (result[0] > 0) {
        debug_partition_info(7);  /* Could trigger case 7 */
    }
}

/* Test invalid partition codes (default case) */
void test_invalid_partitions() {
    /* Force compiler to consider invalid codes through variable input */
    int invalid_codes[] = {-1, 8, 100, 255};
    
    for (int i = 0; i < 4; i++) {
        int code = invalid_codes[i];
        
        /* Use in condition that compiler can't optimize away */
        if (anti_opt) {
            debug_partition_info(code);  /* Could trigger default case */
        }
        
        /* OpenMP version with potentially invalid parameters */
        #pragma omp target teams distribute parallel for \
            num_teams(code > 100 ? 1 : 2) \
            thread_limit(code < 0 ? 1 : code)
        for (int j = 0; j < 10; j++) {
            /* Empty but forces code generation */
            asm volatile("" : : "r"(j));
        }
    }
}

/* OpenMP equivalents to cover different code paths */
void test_omp_partitions() {
    int data[ARRAY_SIZE];
    
    /* OpenMP target with teams (gangs) */
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(tofrom: data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num() * 10 + i;
    }
    
    /* OpenMP with SIMD (vector) */
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) simdlen(8) map(tofrom: data)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] += i % 8;
    }
    
    /* Combined OpenMP nesting */
    #pragma omp target teams distribute parallel for collapse(2) \
        num_teams(2) thread_limit(64) map(tofrom: data)
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            int idx = i * 32 + j;
            data[idx] = omp_get_team_num() * 10000 + 
                       omp_get_thread_num() * 100 + 
                       idx;
        }
    }
}

int main() {
    printf("Testing partition mapping coverage...\n");
    
    /* Initialize anti-optimization variable */
    anti_opt = getenv("ANTI_OPT") != NULL;
    
    /* Test all valid partition cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Test OpenMP variants */
    test_omp_partitions();
    
    /* Test invalid cases */
    test_invalid_partitions();
    
    printf("All partition tests completed.\n");
    printf("Compile with -fopenacc -fopenmp for full coverage.\n");
    
    return 0;
}
