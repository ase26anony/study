/* Test program to cover partition type string conversion in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024

void test_gang_redundant() {
    int data[N];
    
    /* Case 0: gang redundant */
    #pragma acc parallel loop gang(redundant) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) error = 1;
    }
    if (!error) printf("Case 0 (gang redundant) passed\n");
}

void test_gang_partitioned() {
    int data[N];
    
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang(num:32) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 3) error = 1;
    }
    if (!error) printf("Case 1 (gang partitioned) passed\n");
}

void test_worker_partitioned() {
    int data[N];
    
    /* Case 2: worker partitioned */
    #pragma acc parallel loop worker(num:4) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 4;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 4) error = 1;
    }
    if (!error) printf("Case 2 (worker partitioned) passed\n");
}

void test_gang_worker_partitioned() {
    int data[N];
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel loop gang(num:16) worker(num:8) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 5;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 5) error = 1;
    }
    if (!error) printf("Case 3 (gang+worker partitioned) passed\n");
}

void test_vector_partitioned() {
    int data[N];
    
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector_length(128) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 6;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 6) error = 1;
    }
    if (!error) printf("Case 4 (vector partitioned) passed\n");
}

void test_gang_vector_partitioned() {
    int data[N];
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang(num:8) vector_length(64) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 7;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 7) error = 1;
    }
    if (!error) printf("Case 5 (gang+vector partitioned) passed\n");
}

void test_worker_vector_partitioned() {
    int data[N];
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel loop worker(num:4) vector_length(32) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 8;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 8) error = 1;
    }
    if (!error) printf("Case 6 (worker+vector partitioned) passed\n");
}

void test_fully_partitioned() {
    int data[N];
    
    /* Case 7: fully partitioned */
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copy(data)
    for (int i = 0; i < N; i++) {
        data[i] = i * 9;
    }
    
    /* Verify */
    int error = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 9) error = 1;
    }
    if (!error) printf("Case 7 (fully partitioned) passed\n");
}

/* Alternative approach using kernels region */
void test_kernels_variants() {
    int data[N];
    
    /* Test different partition types using kernels */
    #pragma acc kernels copy(data)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i;
        }
    }
    
    #pragma acc kernels copy(data)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    #pragma acc kernels copy(data)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = i * 3;
        }
    }
    
    printf("Kernels variants tested\n");
}

int main() {
    printf("Testing OpenACC partition type coverage...\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_kernels_variants();
    
    printf("All tests completed.\n");
    
    return 0;
}
