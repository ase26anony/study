/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 1024

/* Helper function to ensure computations aren't optimized away */
static void verify_result(int *arr, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    if (sum != expected_sum) {
        fprintf(stderr, "Verification failed: got %d, expected %d\n", sum, expected_sum);
        exit(1);
    }
}

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    if (!data) return 1;
    
    /* Case 0: gang redundant */
    printf("Testing gang redundant...\n");
    #pragma acc parallel loop gang(redundant) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    verify_result(data, N);
    
    /* Case 1: gang partitioned */
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang(num:32) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 2;
    }
    verify_result(data, 2 * N);
    
    /* Case 2: worker partitioned */
    printf("Testing worker partitioned...\n");
    #pragma acc parallel loop worker(num:4) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 3;
    }
    verify_result(data, 3 * N);
    
    /* Case 3: gang+worker partitioned */
    printf("Testing gang+worker partitioned...\n");
    #pragma acc parallel loop gang(num:16) worker(num:8) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 4;
    }
    verify_result(data, 4 * N);
    
    /* Case 4: vector partitioned */
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector_length(128) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 5;
    }
    verify_result(data, 5 * N);
    
    /* Case 5: gang+vector partitioned */
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang(num:8) vector_length(64) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 6;
    }
    verify_result(data, 6 * N);
    
    /* Case 6: worker+vector partitioned */
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel loop worker(num:4) vector_length(32) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 7;
    }
    verify_result(data, 7 * N);
    
    /* Case 7: fully partitioned */
    printf("Testing fully partitioned...\n");
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 8;
    }
    verify_result(data, 8 * N);
    
    /* Additional test with kernels construct */
    printf("Testing kernels construct...\n");
    #pragma acc kernels copy(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 9;
        }
    }
    verify_result(data, 9 * N);
    
    /* Test with combined constructs */
    printf("Testing combined constructs...\n");
    #pragma acc parallel loop gang(num:2) worker(num:2) vector_length(32) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 10;
    }
    verify_result(data, 10 * N);
    
    /* Test with auto clause */
    printf("Testing auto clause...\n");
    #pragma acc parallel loop auto copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 11;
    }
    verify_result(data, 11 * N);
    
    /* Test with seq clause (should be gang redundant) */
    printf("Testing seq clause...\n");
    #pragma acc parallel loop seq copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 12;
    }
    verify_result(data, 12 * N);
    
    free(data);
    printf("All tests completed successfully!\n");
    
    return 0;
}
