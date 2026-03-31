/* Test program to cover omp-oacc-neuter-broadcast.cc partition mapping logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#ifdef _OPENMP
#include <omp.h>
#endif

/* Prevent optimization */
volatile int force_runtime = 1;

/* Function to use partition-dependent results */
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

/* Test each partition type systematically */

/* Case 0: gang redundant */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        int idx = 0;
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1;  // All gangs do same work
        }
    }
    
    use_result(data[0]);
    free(data);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] *= 2;  // Different gangs handle different iterations
        }
    }
    
    use_result(data[n-1]);
    free(data);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] += i % 4;  // Workers partitioned
        }
    }
    
    use_result(data[n/2]);
    free(data);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 3 + (acc_gang() * 10) + acc_worker();
        }
    }
    
    use_result(data[0] + data[n-1]);
    free(data);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(128)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] += acc_vector();
        }
    }
    
    use_result(data[1]);
    free(data);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = (data[i] << 2) | (acc_gang() & 3);
        }
    }
    
    use_result(data[n/4]);
    free(data);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(8) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] += acc_worker() * 100 + acc_vector();
        }
    }
    
    use_result(data[7]);
    free(data);
}

/* Case 7: fully partitioned */
void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(8) num_workers(16) vector_length(32)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = (acc_gang() << 16) | (acc_worker() << 8) | acc_vector();
        }
    }
    
    use_result(data[0] + data[n-1]);
    free(data);
}

/* OpenMP equivalents for additional coverage */
#ifdef _OPENMP
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Various OpenMP partitioning combinations */
    #pragma omp target teams distribute parallel for simd \
        num_teams(4) thread_limit(32) simdlen(8) \
        map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    /* Nested partitioning */
    #pragma omp target teams distribute parallel for \
        num_teams(2) thread_limit(64) \
        map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            data[i] += j;
        }
    }
    
    use_result(data[0]);
    free(data);
}
#endif

/* Force invalid partition codes through boundary conditions */
void test_invalid_partition(int n) {
    /* This may generate invalid partition codes through compiler internals */
    int *data = (int*)malloc(n * sizeof(int));
    
    /* Dynamic partitioning that might confuse the compiler */
    int gangs = force_runtime ? 0 : 4;  /* Could generate gang=0 */
    int workers = force_runtime ? -1 : 2; /* Could generate invalid worker count */
    
    #pragma acc parallel copy(data[0:n]) \
        num_gangs(gangs) num_workers(workers) vector_length(1) \
        if(force_runtime)  /* Conditional compilation */
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            data[i] = i * 2;
        }
    }
    
    /* Template-like macro expansion for different partition schemes */
    #define PARTITION_TEST(level) \
        #pragma acc parallel copy(data[0:n]) \
            num_gangs(level>0?2:1) \
            num_workers(level>1?4:1) \
            vector_length(level>2?8:1) \
        { \
            #pragma acc loop \
            for (int i = 0; i < n; i++) { \
                data[i] += level; \
            } \
        }
    
    /* Generate multiple partition configurations */
    for (int level = 0; level < 8; level++) {
        PARTITION_TEST(level)
    }
    
    free(data);
}

/* Main test driver */
int main(int argc, char **argv) {
    int n = 1024;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 128) n = 128;
    
    printf("Testing partition mapping coverage...\n");
    
    /* Test all valid partition cases */
    test_gang_redundant(n);
    printf("  Case 0 tested\n");
    
    test_gang_partitioned(n);
    printf("  Case 1 tested\n");
    
    test_worker_partitioned(n);
    printf("  Case 2 tested\n");
    
    test_gang_worker_partitioned(n);
    printf("  Case 3 tested\n");
    
    test_vector_partitioned(n);
    printf("  Case 4 tested\n");
    
    test_gang_vector_partitioned(n);
    printf("  Case 5 tested\n");
    
    test_worker_vector_partitioned(n);
    printf("  Case 6 tested\n");
    
    test_fully_partitioned(n);
    printf("  Case 7 tested\n");
    
#ifdef _OPENMP
    test_omp_partitioning(n);
    printf("  OpenMP partitioning tested\n");
#endif
    
    /* Test boundary/invalid cases */
    test_invalid_partition(n);
    printf("  Invalid partition cases tested\n");
    
    printf("All partition tests completed.\n");
    
    /* Force compiler to generate diagnostic strings */
    volatile int error_code = 0;
    if (error_code) {
        /* This would use the partition string mapping in error reporting */
        printf("Error in partition: %d\n", error_code);
    }
    
    return 0;
}
