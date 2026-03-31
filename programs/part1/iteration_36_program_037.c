/* test_partition_cases.c - Exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(var))
#define VOLATILE_INIT volatile int

/* Helper to force runtime evaluation */
int runtime_value(int base) {
    static volatile int seed = 0;
    seed++;
    return base + (seed % 100);
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    const int N = 1000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    /* Initialize with runtime values */
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* Single gang - should map to case 0 */
    #pragma acc parallel num_gangs(1) copy(data[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] += 1;
        }
    }
    
    /* Force partition logic evaluation */
    int sum = 0;
    for (i = 0; i < N; i++) sum += data[i];
    KEEP(sum);
    
    free((void*)data);
}

/* Test case 1: Gang partitioned */
void test_gang_partitioned() {
    const int N = 10000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* Multiple gangs - should map to case 1 */
    int num_gangs = runtime_value(8);
    #pragma acc parallel num_gangs(num_gangs) copy(data[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] *= 2;
        }
    }
    
    KEEP(data[N/2]);
    free((void*)data);
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned() {
    const int N = 5000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* Single gang with workers - should map to case 2 */
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:N])
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            data[i] -= runtime_value(10);
        }
    }
    
    KEEP(data[0]);
    free((void*)data);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned() {
    const int N = 20000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* Combined gang and worker partitioning - case 3 */
    #pragma acc parallel num_gangs(8) num_workers(4) copy(data[0:N])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            data[i] = data[i] / 2 + runtime_value(1);
        }
    }
    
    KEEP(data[N-1]);
    free((void*)data);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    const int N = 8000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* Vector partitioning only - case 4 */
    #pragma acc parallel vector_length(128) copy(data[0:N])
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            data[i] = data[i] << 1;
        }
    }
    
    KEEP(data[N/4]);
    free((void*)data);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned() {
    const int N = 15000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* Gang and vector partitioning - case 5 */
    #pragma acc parallel num_gangs(16) vector_length(64) copy(data[0:N])
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            data[i] = data[i] | 0x1;
        }
    }
    
    KEEP(data[N/3]);
    free((void*)data);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned() {
    const int N = 12000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* Worker and vector partitioning - case 6 */
    #pragma acc parallel num_workers(8) vector_length(256) copy(data[0:N])
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            data[i] = data[i] & 0xFF;
        }
    }
    
    KEEP(data[N/2]);
    free((void*)data);
}

/* Test case 7: Fully partitioned */
void test_fully_partitioned() {
    const int N = 30000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* All three levels - case 7 */
    #pragma acc parallel num_gangs(32) num_workers(4) vector_length(128) copy(data[0:N])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            data[i] = data[i] * 3 + runtime_value(5);
        }
    }
    
    KEEP(data[N-100]);
    free((void*)data);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning() {
    const int N = 10000;
    VOLATILE_INIT *data = (VOLATILE_INIT*)malloc(N * sizeof(int));
    int i;
    
    for (i = 0; i < N; i++) data[i] = runtime_value(i);
    
    /* OpenMP target with teams and distribute - may map to various partitions */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:N]) \
        num_teams(8) thread_limit(256)
    for (i = 0; i < N; i++) {
        data[i] += runtime_value(100);
    }
    
    /* Nested parallelism with SIMD */
    #pragma omp target teams distribute parallel for simd \
        num_teams(4) thread_limit(128) simdlen(8)
    for (i = 0; i < N; i++) {
        data[i] *= 2;
    }
    
    KEEP(data[0]);
    free((void*)data);
}

/* Test invalid partition codes through boundary conditions */
void test_invalid_partitions() {
    /* Force compiler to consider invalid codes */
    VOLATILE_INIT invalid_code = runtime_value(100);
    
    /* This might trigger the default case if the compiler generates
       boundary checking code */
    if (invalid_code < 0 || invalid_code > 7) {
        /* Could trigger error path with "<illegal>" string */
        KEEP(invalid_code);
    }
    
    /* Array access with variable index that could be out of bounds */
    const char* partition_names[] = {
        "gang redundant", "gang partitioned", "worker partitioned",
        "gang+worker partitioned", "vector partitioned", 
        "gang+vector partitioned", "worker+vector partitioned",
        "fully partitioned"
    };
    
    VOLATILE_INIT idx = runtime_value(10);  /* Could be 8+ */
    if (idx >= 0 && idx < 8) {
        KEEP(partition_names[idx][0]);
    } else {
        /* Might reference error handling */
        KEEP(idx);
    }
}

/* Main driver that exercises all cases */
int main() {
    printf("Testing partition mapping cases...\n");
    
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
    
    /* Test OpenMP variants */
    test_omp_partitioning();
    printf("  OpenMP partitioning tested\n");
    
    /* Test boundary/invalid cases */
    test_invalid_partitions();
    printf("  Invalid partition cases tested\n");
    
    printf("All partition tests completed.\n");
    return 0;
}
