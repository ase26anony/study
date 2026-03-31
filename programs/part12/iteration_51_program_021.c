/* Test program to cover all partition type string conversions in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024

/* Helper function to ensure computations aren't optimized away */
static void verify_result(int *arr, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    if (sum != expected_sum) {
        fprintf(stderr, "Verification failed: got %d, expected %d\n", sum, expected_sum);
    }
}

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    if (!data) return 1;

    /* Case 0: gang redundant */
    printf("Testing gang redundant (case 0)...\n");
    #pragma acc parallel loop gang(redundant) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    verify_result(data, N);

    /* Case 1: gang partitioned */
    printf("Testing gang partitioned (case 1)...\n");
    #pragma acc parallel loop gang(num:32) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 2;
    }
    verify_result(data, N * 2);

    /* Case 2: worker partitioned */
    printf("Testing worker partitioned (case 2)...\n");
    #pragma acc parallel loop worker(num:4) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 3;
    }
    verify_result(data, N * 3);

    /* Case 3: gang+worker partitioned */
    printf("Testing gang+worker partitioned (case 3)...\n");
    #pragma acc parallel loop gang(num:16) worker(num:8) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 4;
    }
    verify_result(data, N * 4);

    /* Case 4: vector partitioned */
    printf("Testing vector partitioned (case 4)...\n");
    #pragma acc parallel loop vector_length(128) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 5;
    }
    verify_result(data, N * 5);

    /* Case 5: gang+vector partitioned */
    printf("Testing gang+vector partitioned (case 5)...\n");
    #pragma acc parallel loop gang(num:8) vector_length(64) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 6;
    }
    verify_result(data, N * 6);

    /* Case 6: worker+vector partitioned */
    printf("Testing worker+vector partitioned (case 6)...\n");
    #pragma acc parallel loop worker(num:4) vector_length(32) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 7;
    }
    verify_result(data, N * 7);

    /* Case 7: fully partitioned */
    printf("Testing fully partitioned (case 7)...\n");
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 8;
    }
    verify_result(data, N * 8);

    /* Alternative approach for kernels regions */
    printf("Testing with kernels regions...\n");
    
    /* gang redundant with kernels */
    #pragma acc kernels loop gang(redundant) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 9;
    }
    
    /* gang partitioned with kernels */
    #pragma acc kernels loop gang(num:16) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 10;
    }
    
    /* Combined clauses in kernels */
    #pragma acc kernels loop gang(num:8) worker(num:4) vector_length(32) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = 11;
    }

    printf("All OpenACC directives executed.\n");

    /* For the default case (<illegal>), we need to trigger an invalid partition type.
       This is tricky from user code, but we can try to create an invalid combination.
       One approach is to use nested parallel regions with conflicting clauses. */
    printf("Attempting to trigger invalid partition type...\n");
    
    /* This might trigger internal error states */
    #pragma acc parallel copy(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 12;
        }
        
        /* Nested loop with different partitioning - might confuse runtime */
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = 13;
        }
    }

    free(data);
    return 0;
}
