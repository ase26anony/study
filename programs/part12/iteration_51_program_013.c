/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 1024

/* Helper function to verify computations */
void verify_array(int *arr, int expected, int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            fprintf(stderr, "Verification failed at index %d: got %d, expected %d\n", 
                    i, arr[i], expected);
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

    /* Case 0: gang redundant */
    printf("Testing case 0: gang redundant\n");
    #pragma acc parallel loop gang(redundant) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    verify_array(data, 1, N);

    /* Case 1: gang partitioned */
    printf("Testing case 1: gang partitioned\n");
    #pragma acc parallel loop gang(num:32) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 2;
    }
    verify_array(data, 2, N);

    /* Case 2: worker partitioned */
    printf("Testing case 2: worker partitioned\n");
    #pragma acc parallel loop worker(num:4) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 3;
    }
    verify_array(data, 3, N);

    /* Case 3: gang+worker partitioned */
    printf("Testing case 3: gang+worker partitioned\n");
    #pragma acc parallel loop gang(num:16) worker(num:8) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 4;
    }
    verify_array(data, 4, N);

    /* Case 4: vector partitioned */
    printf("Testing case 4: vector partitioned\n");
    #pragma acc parallel loop vector_length(128) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 5;
    }
    verify_array(data, 5, N);

    /* Case 5: gang+vector partitioned */
    printf("Testing case 5: gang+vector partitioned\n");
    #pragma acc parallel loop gang(num:8) vector_length(64) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 6;
    }
    verify_array(data, 6, N);

    /* Case 6: worker+vector partitioned */
    printf("Testing case 6: worker+vector partitioned\n");
    #pragma acc parallel loop worker(num:4) vector_length(32) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 7;
    }
    verify_array(data, 7, N);

    /* Case 7: fully partitioned */
    printf("Testing case 7: fully partitioned\n");
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 8;
    }
    verify_array(data, 8, N);

    /* Additional test with kernels directive for different code paths */
    printf("Testing with kernels directive\n");
    #pragma acc kernels copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 9;
        }
    }
    verify_array(data, 9, N);

    /* Test with combined clauses */
    printf("Testing combined clauses\n");
    #pragma acc parallel loop gang worker vector copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 10;
    }
    verify_array(data, 10, N);

    printf("All tests passed successfully!\n");
    
    free(data);
    return 0;
}
