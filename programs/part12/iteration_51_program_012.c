/* Test program to cover partition type string conversion in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare the internal function if it's exposed */
extern "C" const char* oacc_partition_type_to_str(int type);

#define N 1024

void test_gang_redundant() {
    int data[N];
    
    /* Case 0: gang redundant */
    #pragma acc parallel gang(redundant) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
}

void test_gang_partitioned() {
    int data[N];
    
    /* Case 1: gang partitioned */
    #pragma acc parallel gang(num:32) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
}

void test_worker_partitioned() {
    int data[N];
    
    /* Case 2: worker partitioned */
    #pragma acc parallel worker(num:4) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 3;
    }
}

void test_gang_worker_partitioned() {
    int data[N];
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel gang(num:16) worker(num:8) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 4;
    }
}

void test_vector_partitioned() {
    int data[N];
    
    /* Case 4: vector partitioned */
    #pragma acc parallel vector_length(128) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 5;
    }
}

void test_gang_vector_partitioned() {
    int data[N];
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel gang(num:8) vector_length(64) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 6;
    }
}

void test_worker_vector_partitioned() {
    int data[N];
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel worker(num:4) vector_length(32) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 7;
    }
}

void test_fully_partitioned() {
    int data[N];
    
    /* Case 7: fully partitioned */
    #pragma acc parallel gang(num:4) worker(num:2) vector_length(16) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 8;
    }
}

void test_invalid_partition() {
    /* Try to trigger default case by calling the function directly if exposed */
    /* This might not work if the function isn't exported, but we try */
    #ifdef __GNUC__
    /* Use GCC's attribute to find the symbol */
    __attribute__((weak)) extern const char* __oacc_partition_type_to_str(int);
    
    if (&__oacc_partition_type_to_str) {
        /* Test invalid values */
        const char* result;
        result = __oacc_partition_type_to_str(-1);
        result = __oacc_partition_type_to_str(8);
        result = __oacc_partition_type_to_str(100);
    }
    #endif
}

int main() {
    printf("Testing OpenACC partition type coverage...\n");
    
    /* Test all valid partition types through OpenACC regions */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Try to trigger invalid partition type */
    test_invalid_partition();
    
    printf("All tests completed.\n");
    
    /* Verify results with a simple check */
    int verify[N];
    #pragma acc parallel loop gang worker vector copy(verify)
    for (int i = 0; i < N; i++) {
        verify[i] = i;
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += verify[i];
    }
    
    printf("Verification sum: %d\n", sum);
    
    return 0;
}
