/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 1024

/* Helper function to ensure computations aren't optimized away */
void verify_result(int *data, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    if (sum != expected_sum) {
        fprintf(stderr, "Verification failed: got %d, expected %d\n", sum, expected_sum);
        exit(1);
    }
}

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    /* Case 0: gang redundant */
    #pragma acc parallel loop gang(redundant) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    verify_result(data, N);

    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang(num:32) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 2;
    }
    verify_result(data, N * 2);

    /* Case 2: worker partitioned */
    #pragma acc parallel loop worker(num:4) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 3;
    }
    verify_result(data, N * 3);

    /* Case 3: gang+worker partitioned */
    #pragma acc parallel loop gang(num:16) worker(num:8) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 4;
    }
    verify_result(data, N * 4);

    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector_length(128) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 5;
    }
    verify_result(data, N * 5);

    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang(num:8) vector_length(64) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 6;
    }
    verify_result(data, N * 6);

    /* Case 6: worker+vector partitioned */
    #pragma acc parallel loop worker(num:4) vector_length(32) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 7;
    }
    verify_result(data, N * 7);

    /* Case 7: fully partitioned */
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 8;
    }
    verify_result(data, N * 8);

    /* Additional test using kernels construct for different code paths */
    #pragma acc kernels copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 9;
        }
    }
    verify_result(data, N * 9);

    /* Test with combined clauses in kernels */
    #pragma acc kernels copyout(data[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            data[i] = 10;
        }
    }
    verify_result(data, N * 10);

    /* Test with all three levels in kernels */
    #pragma acc kernels copyout(data[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            data[i] = 11;
        }
    }
    verify_result(data, N * 11);

    printf("All OpenACC partition tests completed successfully\n");
    
    free(data);
    return 0;
}
