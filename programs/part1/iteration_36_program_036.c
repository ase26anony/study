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
volatile int prevent_opt = 0;

/* Function that could use partition description strings */
void debug_partition_info(int code) {
    /* This mimics internal compiler logic that maps codes to strings */
    if (code < 0 || code > 7) {
        /* Could trigger default case "<illegal>" */
        printf("Invalid partition code: %d\n", code);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop gang(num:1) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 2;
    }
    
    /* Verify and potentially trigger debug output */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) gang(num:1)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += data[i];
    }
    
    if (sum != ARRAY_SIZE * (ARRAY_SIZE - 1)) {
        debug_partition_info(0);  /* Could trigger case 0 string */
    }
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel loop gang(num:MAX_GANGS) copy(data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i + omp_get_thread_num();
    }
    
    /* Nested gang partitioning */
    #pragma acc parallel num_gangs(MAX_GANGS)
    {
        #pragma acc loop gang
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] *= 2;
        }
    }
    
    prevent_opt = data[ARRAY_SIZE/2];
}

/* Test case 2: Worker partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel num_gangs(1) num_workers(MAX_WORKERS) copy(data)
    {
        #pragma acc loop worker
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 3.14f;
        }
    }
    
    /* Worker-only parallel region */
    #pragma acc parallel loop worker num_workers(MAX_WORKERS)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] += 1.0f;
    }
    
    prevent_opt = (int)data[0];
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    double data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel num_gangs(MAX_GANGS) num_workers(MAX_WORKERS) copy(data)
    {
        #pragma acc loop gang worker
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = (double)i / (omp_get_thread_num() + 1);
        }
    }
    
    /* Combined gang-worker loops */
    #pragma acc parallel loop gang(MAX_GANGS) worker(MAX_WORKERS)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = data[i] * 2.0;
    }
    
    prevent_opt = (int)data[ARRAY_SIZE-1];
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel vector_length(VECTOR_LENGTH) copy(data)
    {
        #pragma acc loop vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i << 2;
        }
    }
    
    /* Vector-only computation */
    #pragma acc parallel loop vector vector_length(VECTOR_LENGTH)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] |= 0x1;
    }
    
    prevent_opt = data[0];
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel num_gangs(MAX_GANGS) vector_length(VECTOR_LENGTH) copy(data)
    {
        #pragma acc loop gang vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * acc_on_device(acc_device_not_host);
        }
    }
    
    /* Alternative syntax */
    #pragma acc parallel loop gang(MAX_GANGS) vector(VECTOR_LENGTH)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] += prevent_opt;
    }
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel num_workers(MAX_WORKERS) vector_length(VECTOR_LENGTH) copy(data)
    {
        #pragma acc loop worker vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i % 256;
        }
    }
    
    /* Worker-vector combined */
    #pragma acc parallel loop worker(MAX_WORKERS) vector(VECTOR_LENGTH)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = data[i] * 3;
    }
    
    prevent_opt = data[100];
}

/* Test case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    #pragma acc parallel num_gangs(MAX_GANGS) num_workers(MAX_WORKERS) \
                vector_length(VECTOR_LENGTH) copy(data)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i + acc_on_device(acc_device_nvidia);
        }
    }
    
    /* Fully explicit partitioning */
    #pragma acc parallel loop gang(MAX_GANGS) worker(MAX_WORKERS) vector(VECTOR_LENGTH)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = data[i] ^ 0xAA;
    }
    
    /* Complex nested partitioning */
    #pragma acc parallel num_gangs(MAX_GANGS)
    {
        #pragma acc loop gang
        for (int g = 0; g < MAX_GANGS; g++) {
            #pragma acc loop worker num_workers(MAX_WORKERS)
            for (int w = 0; w < MAX_WORKERS; w++) {
                #pragma acc loop vector vector_length(VECTOR_LENGTH)
                for (int v = 0; v < VECTOR_LENGTH; v++) {
                    int idx = g * MAX_WORKERS * VECTOR_LENGTH + w * VECTOR_LENGTH + v;
                    if (idx < ARRAY_SIZE) {
                        data[idx] += g + w + v;
                    }
                }
            }
        }
    }
    
    prevent_opt = data[500];
}

/* Test default case: Invalid partition codes */
void test_invalid_partitions() {
    /* Force invalid partition through macro expansion */
    #define INVALID_PARTITION 99
    
    /* This could trigger the default case in debug output */
    debug_partition_info(INVALID_PARTITION);
    debug_partition_info(-1);
    debug_partition_info(8);
    
    /* Dynamic invalid partition */
    int invalid_code = rand() % 100 + 8;
    debug_partition_info(invalid_code);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning() {
    int data[ARRAY_SIZE];
    int i;
    
    /* OpenMP target with teams (gangs) and parallel (workers) */
    #pragma omp target teams distribute parallel for \
                num_teams(MAX_GANGS) thread_limit(MAX_WORKERS*VECTOR_LENGTH) \
                map(tofrom: data)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * omp_get_team_num();
    }
    
    /* OpenMP with SIMD (vector) */
    #pragma omp target teams distribute parallel for simd \
                num_teams(MAX_GANGS) thread_limit(MAX_WORKERS) \
                simdlen(VECTOR_LENGTH)
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] += omp_get_thread_num();
    }
    
    prevent_opt = data[300];
}

/* Main test driver */
int main() {
    printf("Testing partition mapping cases...\n");
    
    /* Initialize random seed for invalid partition tests */
    srand(42);
    
    /* Execute all test cases */
    test_gang_redundant();
    printf("Case 0 tested\n");
    
    test_gang_partitioned();
    printf("Case 1 tested\n");
    
    test_worker_partitioned();
    printf("Case 2 tested\n");
    
    test_gang_worker_partitioned();
    printf("Case 3 tested\n");
    
    test_vector_partitioned();
    printf("Case 4 tested\n");
    
    test_gang_vector_partitioned();
    printf("Case 5 tested\n");
    
    test_worker_vector_partitioned();
    printf("Case 6 tested\n");
    
    test_fully_partitioned();
    printf("Case 7 tested\n");
    
    test_omp_partitioning();
    printf("OpenMP partitioning tested\n");
    
    test_invalid_partitions();
    printf("Invalid partitions tested (default case)\n");
    
    printf("All partition cases covered\n");
    
    /* Use the volatile variable to prevent dead code elimination */
    asm volatile("" : : "r"(prevent_opt));
    
    return 0;
}
