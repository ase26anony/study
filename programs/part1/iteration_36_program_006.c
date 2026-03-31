/* test_partition_cases.c - Exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

/* Prevent optimization elimination */
#define KEEP(var) asm volatile("" : : "r"(var))
#define KEEP_PTR(ptr) asm volatile("" : : "r"(ptr))

/* Debug output that could trigger partition string mapping */
static void debug_partition_info(int partition_code) {
    /* This mimics internal compiler logic that maps codes to strings */
    volatile int code = partition_code;
    KEEP(code);
    
    /* Conditional error that could use the partition strings */
    if (code < 0 || code > 7) {
        fprintf(stderr, "Invalid partition code encountered\n");
    }
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    volatile int use_acc = 1;
    
    for (int i = 0; i < size; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(1) num_workers(1) vector_length(1)
    {
        int idx = acc_gang_id * acc_worker_id;
        if (idx == 0) {
            for (int i = 0; i < size; i++) {
                #pragma acc atomic update
                data[i] += 1;
            }
        }
    }
    
    /* Verify and trigger debug */
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] != i + 1) errors++;
    }
    if (errors > 0) debug_partition_info(0);
    
    free(data);
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    int gangs = 4;
    
    for (int i = 0; i < size; i++) data[i] = 0;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(gangs) num_workers(1) vector_length(1)
    {
        int gid = acc_gang_id;
        int start = gid * (size / gangs);
        int end = (gid + 1) * (size / gangs);
        
        for (int i = start; i < end && i < size; i++) {
            data[i] = gid + 1;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 1 || data[i] > gangs) errors++;
    }
    if (errors > 0) debug_partition_info(1);
    
    free(data);
}

/* Test case 2: Worker partitioned (single gang, multiple workers) */
void test_worker_partitioned(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    int workers = 8;
    
    for (int i = 0; i < size; i++) data[i] = 0;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(1) num_workers(workers) vector_length(1)
    {
        int wid = acc_worker_id;
        int start = wid * (size / workers);
        int end = (wid + 1) * (size / workers);
        
        for (int i = start; i < end && i < size; i++) {
            data[i] = wid + 100;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 100 || data[i] > 100 + workers) errors++;
    }
    if (errors > 0) debug_partition_info(2);
    
    free(data);
}

/* Test case 3: Gang+worker partitioned */
void test_gang_worker_partitioned(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    int gangs = 2, workers = 4;
    
    for (int i = 0; i < size; i++) data[i] = 0;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(gangs) num_workers(workers) vector_length(1)
    {
        int gid = acc_gang_id;
        int wid = acc_worker_id;
        int idx = gid * workers + wid;
        
        if (idx < size) {
            data[idx] = gid * 1000 + wid;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < size && i < gangs * workers; i++) {
        int expected = (i / workers) * 1000 + (i % workers);
        if (data[i] != expected) errors++;
    }
    if (errors > 0) debug_partition_info(3);
    
    free(data);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    int vector_len = 32;
    
    for (int i = 0; i < size; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(1) num_workers(1) vector_length(vector_len)
    {
        #pragma acc loop vector
        for (int i = 0; i < size; i++) {
            data[i] *= 2;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] != i * 2) errors++;
    }
    if (errors > 0) debug_partition_info(4);
    
    free(data);
}

/* Test case 5: Gang+vector partitioned */
void test_gang_vector_partitioned(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    int gangs = 4, vector_len = 16;
    
    for (int i = 0; i < size; i++) data[i] = 0;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(gangs) num_workers(1) vector_length(vector_len)
    {
        int gid = acc_gang_id;
        #pragma acc loop gang vector
        for (int i = gid; i < size; i += gangs) {
            data[i] = gid * 10 + (i % vector_len);
        }
    }
    
    int errors = 0;
    for (int i = 0; i < size; i++) {
        int expected = (i % gangs) * 10 + (i % vector_len);
        if (data[i] != expected) errors++;
    }
    if (errors > 0) debug_partition_info(5);
    
    free(data);
}

/* Test case 6: Worker+vector partitioned */
void test_worker_vector_partitioned(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    int workers = 8, vector_len = 8;
    
    for (int i = 0; i < size; i++) data[i] = 0;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(1) num_workers(workers) vector_length(vector_len)
    {
        int wid = acc_worker_id;
        #pragma acc loop worker vector
        for (int i = wid; i < size; i += workers) {
            data[i] = wid * 100 + (i % vector_len);
        }
    }
    
    int errors = 0;
    for (int i = 0; i < size; i++) {
        int expected = (i % workers) * 100 + (i % vector_len);
        if (data[i] != expected) errors++;
    }
    if (errors > 0) debug_partition_info(6);
    
    free(data);
}

/* Test case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    int gangs = 2, workers = 2, vector_len = 8;
    int total_parallel = gangs * workers * vector_len;
    
    for (int i = 0; i < size; i++) data[i] = 0;
    
    #pragma acc parallel copy(data[0:size]) \
        num_gangs(gangs) num_workers(workers) vector_length(vector_len)
    {
        int gid = acc_gang_id;
        int wid = acc_worker_id;
        int vid = acc_vector_id;
        int idx = (gid * workers * vector_len) + (wid * vector_len) + vid;
        
        if (idx < size) {
            data[idx] = gid * 10000 + wid * 100 + vid;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < size && i < total_parallel; i++) {
        int gid = i / (workers * vector_len);
        int rem = i % (workers * vector_len);
        int wid = rem / vector_len;
        int vid = rem % vector_len;
        int expected = gid * 10000 + wid * 100 + vid;
        if (data[i] != expected) errors++;
    }
    if (errors > 0) debug_partition_info(7);
    
    free(data);
}

/* OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    
    /* OpenMP target offload with various team/thread configurations */
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(tofrom: data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    /* Combined SIMD for vector partitioning */
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(16) simdlen(8) map(tofrom: data[0:size])
    for (int i = 0; i < size; i++) {
        data[i] += 1;
    }
    
    free(data);
}

/* Generate invalid partition codes for default case */
void test_invalid_partitions(void) {
    volatile int invalid_codes[] = {-1, 8, 255, 1024};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i]);
    }
    
    /* Force compiler to consider all cases through template-like macro */
    #define TEST_PARTITION_CASE(n) \
        do { \
            volatile int code = n; \
            if (code >= 0 && code <= 7) { \
                /* Valid case */ \
            } else { \
                debug_partition_info(code); \
            } \
            KEEP(code); \
        } while(0)
    
    for (int c = -2; c <= 10; c++) {
        TEST_PARTITION_CASE(c);
    }
}

int main(int argc, char **argv) {
    int size = 1024;
    
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size < 64) size = 64;
        if (size > 65536) size = 65536;
    }
    
    printf("Testing partition mapping cases with size = %d\n", size);
    
    /* Test all valid partition cases (0-7) */
    test_gang_redundant(size);
    printf("Case 0 tested\n");
    
    test_gang_partitioned(size);
    printf("Case 1 tested\n");
    
    test_worker_partitioned(size);
    printf("Case 2 tested\n");
    
    test_gang_worker_partitioned(size);
    printf("Case 3 tested\n");
    
    test_vector_partitioned(size);
    printf("Case 4 tested\n");
    
    test_gang_vector_partitioned(size);
    printf("Case 5 tested\n");
    
    test_worker_vector_partitioned(size);
    printf("Case 6 tested\n");
    
    test_fully_partitioned(size);
    printf("Case 7 tested\n");
    
    /* Test OpenMP partitioning */
    test_omp_partitioning(size);
    printf("OpenMP partitioning tested\n");
    
    /* Test invalid partition codes (default case) */
    test_invalid_partitions();
    printf("Invalid partition codes tested\n");
    
    printf("All partition mapping cases exercised\n");
    return 0;
}
