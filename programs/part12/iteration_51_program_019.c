/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 1024

/* Helper function to verify computations */
void verify_array(int *arr, int expected_value, const char *test_name) {
    for (int i = 0; i < N; i++) {
        if (arr[i] != expected_value) {
            printf("Test %s failed at index %d: got %d, expected %d\n",
                   test_name, i, arr[i], expected_value);
            exit(1);
        }
    }
}

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    printf("Testing all OpenACC partition types...\n");

    /* Case 0: gang redundant */
    printf("Testing case 0: gang redundant\n");
    #pragma acc parallel loop gang(redundant) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    verify_array(data, 1, "gang redundant");

    /* Case 1: gang partitioned */
    printf("Testing case 1: gang partitioned\n");
    #pragma acc parallel loop gang(num:32) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 2;
    }
    verify_array(data, 2, "gang partitioned");

    /* Case 2: worker partitioned */
    printf("Testing case 2: worker partitioned\n");
    #pragma acc parallel loop worker(num:4) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 3;
    }
    verify_array(data, 3, "worker partitioned");

    /* Case 3: gang+worker partitioned */
    printf("Testing case 3: gang+worker partitioned\n");
    #pragma acc parallel loop gang(num:16) worker(num:8) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 4;
    }
    verify_array(data, 4, "gang+worker partitioned");

    /* Case 4: vector partitioned */
    printf("Testing case 4: vector partitioned\n");
    #pragma acc parallel loop vector_length(128) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 5;
    }
    verify_array(data, 5, "vector partitioned");

    /* Case 5: gang+vector partitioned */
    printf("Testing case 5: gang+vector partitioned\n");
    #pragma acc parallel loop gang(num:8) vector_length(64) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 6;
    }
    verify_array(data, 6, "gang+vector partitioned");

    /* Case 6: worker+vector partitioned */
    printf("Testing case 6: worker+vector partitioned\n");
    #pragma acc parallel loop worker(num:4) vector_length(32) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 7;
    }
    verify_array(data, 7, "worker+vector partitioned");

    /* Case 7: fully partitioned */
    printf("Testing case 7: fully partitioned\n");
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 8;
    }
    verify_array(data, 8, "fully partitioned");

    /* Additional test with kernels region to ensure different code paths */
    printf("Testing with kernels region\n");
    #pragma acc kernels copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 9;
        }
    }
    verify_array(data, 9, "kernels region");

    /* Test invalid partition type by directly calling the internal function if possible */
    /* Note: This is compiler-specific and may not work on all implementations */
    printf("All partition type tests completed successfully\n");

    free(data);
    return 0;
}
