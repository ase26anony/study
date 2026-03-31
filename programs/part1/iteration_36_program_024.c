/* Test program to exercise omp-oacc-neuter-broadcast.cc partition mapping logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

/* Prevent optimization of critical variables */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function to generate debug output that might use partition strings */
void debug_partition_info(int code, const char* desc) {
    volatile int keep_code = code;
    volatile const char* keep_desc = desc;
    KEEP(keep_code);
    KEEP(keep_desc);
    
    /* This could trigger the partition string mapping */
    if (code < 0 || code > 7) {
        fprintf(stderr, "Invalid partition code: %d (%s)\n", code, desc);
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int init_val = 42;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] = init_val + i;
        }
    }
    
    /* Force compiler to consider partition mapping */
    debug_partition_info(0, "gang redundant");
    
    free(data);
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int gangs = 4;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(gangs) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] = i * 2;
        }
    }
    
    debug_partition_info(1, "gang partitioned");
    free(data);
}

/* Test case 2: Worker partitioned (single gang, multiple workers) */
void test_worker_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int workers = 8;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(1) num_workers(workers) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] = i * 3;
        }
    }
    
    debug_partition_info(2, "worker partitioned");
    free(data);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int gangs = 2;
    volatile int workers = 4;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(gangs) num_workers(workers) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = i * 4;
        }
    }
    
    debug_partition_info(3, "gang+worker partitioned");
    free(data);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int vector_len = 32;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(1) num_workers(1) vector_length(vector_len)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] = i * 5;
        }
    }
    
    debug_partition_info(4, "vector partitioned");
    free(data);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int gangs = 4;
    volatile int vector_len = 16;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(gangs) num_workers(1) vector_length(vector_len)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = i * 6;
        }
    }
    
    debug_partition_info(5, "gang+vector partitioned");
    free(data);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int workers = 8;
    volatile int vector_len = 8;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(1) num_workers(workers) vector_length(vector_len)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] = i * 7;
        }
    }
    
    debug_partition_info(6, "worker+vector partitioned");
    free(data);
}

/* Test case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    volatile int gangs = 2;
    volatile int workers = 4;
    volatile int vector_len = 8;
    
    #pragma acc parallel copyout(data[0:n]) num_gangs(gangs) num_workers(workers) vector_length(vector_len)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = i * 8;
        }
    }
    
    debug_partition_info(7, "fully partitioned");
    free(data);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    /* OpenMP target offload with various partitioning */
    #pragma omp target teams distribute parallel for simd \
        num_teams(4) thread_limit(32) simdlen(8) \
        map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 9;
    }
    
    /* Nested OpenMP to trigger different partition combinations */
    #pragma omp target teams distribute simd \
        num_teams(2) simdlen(16) \
        map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += 1;
    }
    
    free(data);
}

/* Function to test invalid partition codes (default case) */
void test_invalid_partitions() {
    /* Force compiler to consider out-of-bounds partition codes */
    volatile int invalid_codes[] = {-1, 8, 255, 1024};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i], "<illegal>");
    }
    
    /* Template-like macro to generate various partition scenarios */
    #define TEST_PARTITION(code, gangs, workers, vectors) \
        do { \
            volatile int g = gangs; \
            volatile int w = workers; \
            volatile int v = vectors; \
            int size = 1024; \
            int* arr = (int*)malloc(size * sizeof(int)); \
            if (code >= 0 && code <= 7) { \
                #pragma acc parallel copyout(arr[0:size]) \
                    num_gangs(g) num_workers(w) vector_length(v) \
                { \
                    #pragma acc loop gang worker vector \
                    for (int j = 0; j < size; j++) { \
                        arr[j] = j * code; \
                    } \
                } \
            } \
            debug_partition_info(code, "dynamic"); \
            free(arr); \
        } while(0)
    
    /* Generate various partition combinations */
    TEST_PARTITION(0, 1, 1, 1);
    TEST_PARTITION(1, 4, 1, 1);
    TEST_PARTITION(2, 1, 8, 1);
    TEST_PARTITION(3, 2, 4, 1);
    TEST_PARTITION(4, 1, 1, 32);
    TEST_PARTITION(5, 4, 1, 16);
    TEST_PARTITION(6, 1, 8, 8);
    TEST_PARTITION(7, 2, 4, 8);
    
    #undef TEST_PARTITION
}

/* Main function that exercises all test cases */
int main(int argc, char** argv) {
    int n = 1024;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1024;
    }
    
    printf("Testing partition mapping logic with n = %d\n", n);
    
    /* Test all valid partition codes (0-7) */
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
    
    /* Test invalid partition codes (default case) */
    test_invalid_partitions();
    
    printf("All partition tests completed.\n");
    
    return 0;
}
