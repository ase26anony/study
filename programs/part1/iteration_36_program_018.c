/* Test program to cover partition mapping logic in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#ifdef _OPENMP
#include <omp.h>
#endif

/* Prevent optimization */
volatile int force_runtime = 1;

/* Function to use results and prevent dead code elimination */
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

/* Function that could trigger partition string mapping in diagnostics */
void debug_partition_info(int partition_code) {
    /* This mimics internal compiler logic that maps codes to strings */
    if (partition_code < 0 || partition_code > 7) {
        /* Could trigger default case "<illegal>" */
        fprintf(stderr, "Invalid partition code encountered\n");
    }
}

/* Test cases for each partition type */

/* Case 0: gang redundant - single gang */
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1;
        }
    }
    
    /* Verify and use result */
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    free(data);
}

/* Case 1: gang partitioned - multiple gangs */
void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    int gangs = force_runtime ? 4 : 1;
    #pragma acc parallel copy(data[0:n]) num_gangs(gangs) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
    }
    
    use_result(data[n/2]);
    free(data);
}

/* Case 2: worker partitioned - single gang, multiple workers */
void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] += i % 10;
        }
    }
    
    use_result(data[0]);
    free(data);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    int gangs = force_runtime ? 2 : 1;
    int workers = force_runtime ? 4 : 1;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(gangs) num_workers(workers) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 3 + 1;
        }
    }
    
    use_result(data[n-1]);
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
            data[i] += 5;
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
            data[i] -= 2;
        }
    }
    
    use_result(data[n/4]);
    free(data);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] = data[i] / 2;
        }
    }
    
    use_result(data[n/2]);
    free(data);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = data[i] % 100;
        }
    }
    
    use_result(data[0] + data[n-1]);
    free(data);
}

/* OpenMP equivalents for additional coverage */
#ifdef _OPENMP
void test_omp_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n]) num_teams(4) thread_limit(1)
    for (int i = 0; i < n; i++) {
        data[i] += 7;
    }
    
    use_result(data[0]);
    free(data);
}

void test_omp_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:n]) \
            num_teams(2) thread_limit(64) simdlen(8)
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2 + 1;
    }
    
    use_result(data[n/3]);
    free(data);
}
#endif

/* Test invalid partition codes (default case) */
void test_invalid_partition_codes() {
    /* Force generation of invalid codes through boundary conditions */
    int invalid_codes[] = {-1, 8, 100, -100};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i]);
    }
    
    /* Variable partition spec that could be out of bounds */
    int dynamic_partition = force_runtime ? 10 : 0;
    #pragma acc parallel num_gangs(dynamic_partition) num_workers(dynamic_partition) vector_length(dynamic_partition)
    {
        /* Empty but forces partition consideration */
    }
}

/* Combined test with runtime selection */
void test_mixed_partitioning(int n, int mode) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    /* Runtime-dependent partitioning */
    switch (mode % 8) {
        case 0:
            #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
            { #pragma acc loop gang for (int i = 0; i < n; i++) data[i] += 1; }
            break;
        case 1:
            #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(1)
            { #pragma acc loop gang for (int i = 0; i < n; i++) data[i] += 2; }
            break;
        case 2:
            #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(1)
            { #pragma acc loop worker for (int i = 0; i < n; i++) data[i] += 3; }
            break;
        case 3:
            #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(4) vector_length(1)
            { #pragma acc loop gang worker for (int i = 0; i < n; i++) data[i] += 4; }
            break;
        case 4:
            #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(128)
            { #pragma acc loop vector for (int i = 0; i < n; i++) data[i] += 5; }
            break;
        case 5:
            #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(64)
            { #pragma acc loop gang vector for (int i = 0; i < n; i++) data[i] += 6; }
            break;
        case 6:
            #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(32)
            { #pragma acc loop worker vector for (int i = 0; i < n; i++) data[i] += 7; }
            break;
        case 7:
            #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(4) vector_length(16)
            { #pragma acc loop gang worker vector for (int i = 0; i < n; i++) data[i] += 8; }
            break;
    }
    
    use_result(data[0]);
    free(data);
}

int main() {
    int n = 1024;
    
    printf("Testing all partition cases...\n");
    
    /* Test each partition type systematically */
    test_gang_redundant(n);
    test_gang_partitioned(n);
    test_worker_partitioned(n);
    test_gang_worker_partitioned(n);
    test_vector_partitioned(n);
    test_gang_vector_partitioned(n);
    test_worker_vector_partitioned(n);
    test_fully_partitioned(n);
    
    /* Test OpenMP variants if available */
    #ifdef _OPENMP
    test_omp_gang_partitioned(n);
    test_omp_fully_partitioned(n);
    #endif
    
    /* Test invalid codes for default case */
    test_invalid_partition_codes();
    
    /* Test mixed partitioning with runtime selection */
    for (int mode = 0; mode < 10; mode++) {
        test_mixed_partitioning(n, mode);
    }
    
    printf("All tests completed (check compiler diagnostics for partition mapping)\n");
    
    return 0;
}
