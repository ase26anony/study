/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ARRAY_SIZE 1024

/* Helper function to ensure computations aren't optimized away */
static void verify_result(int *arr, int size, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    if (sum != expected_sum) {
        fprintf(stderr, "Verification failed: got %d, expected %d\n", sum, expected_sum);
    }
}

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    int *result = (int*)malloc(N * sizeof(int));
    
    if (!data || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = i % 100;
    }
    
    printf("Testing OpenACC partition types...\n");
    
    /* Case 0: gang redundant */
    printf("Testing gang redundant...\n");
    #pragma acc parallel loop gang(redundant) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] * 2;
    }
    verify_result(result, N, 9900); /* Sum of (i%100)*2 for i=0..1023 */
    
    /* Case 1: gang partitioned */
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang(num:32) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] + 1;
    }
    verify_result(result, N, 10496); /* Sum of (i%100)+1 for i=0..1023 */
    
    /* Case 2: worker partitioned */
    printf("Testing worker partitioned...\n");
    #pragma acc parallel loop worker(num:4) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] * 3;
    }
    verify_result(result, N, 14850); /* Sum of (i%100)*3 for i=0..1023 */
    
    /* Case 3: gang+worker partitioned */
    printf("Testing gang+worker partitioned...\n");
    #pragma acc parallel loop gang(num:16) worker(num:8) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] - 1;
    }
    verify_result(result, N, 9392); /* Sum of (i%100)-1 for i=0..1023 */
    
    /* Case 4: vector partitioned */
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector_length(128) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] / 2;
    }
    verify_result(result, N, 2475); /* Sum of (i%100)/2 for i=0..1023 */
    
    /* Case 5: gang+vector partitioned */
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang(num:8) vector_length(64) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] * data[i];
    }
    verify_result(result, N, 328350); /* Sum of (i%100)^2 for i=0..1023 */
    
    /* Case 6: worker+vector partitioned */
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel loop worker(num:4) vector_length(32) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] % 10;
    }
    verify_result(result, N, 4590); /* Sum of (i%100)%10 for i=0..1023 */
    
    /* Case 7: fully partitioned */
    printf("Testing fully partitioned...\n");
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] * 4;
    }
    verify_result(result, N, 19800); /* Sum of (i%100)*4 for i=0..1023 */
    
    /* Additional test using kernels construct for different partitioning */
    printf("Testing kernels with various partitions...\n");
    
    /* Kernels with gang partitioning */
    #pragma acc kernels copyin(data[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = data[i] * 5;
        }
    }
    verify_result(result, N, 24750); /* Sum of (i%100)*5 for i=0..1023 */
    
    /* Kernels with gang and worker partitioning */
    #pragma acc kernels copyin(data[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            result[i] = data[i] + 10;
        }
    }
    verify_result(result, N, 15096); /* Sum of (i%100)+10 for i=0..1023 */
    
    /* Kernels with vector partitioning */
    #pragma acc kernels copyin(data[0:N]) copyout(result[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = data[i] - 10;
        }
    }
    verify_result(result, N, 3896); /* Sum of (i%100)-10 for i=0..1023 */
    
    /* Test with async clause which might trigger different internal paths */
    printf("Testing with async...\n");
    #pragma acc parallel loop gang(num:4) async(1) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] * 6;
    }
    #pragma acc wait(1)
    verify_result(result, N, 29700); /* Sum of (i%100)*6 for i=0..1023 */
    
    /* Test with if clause */
    printf("Testing with conditional...\n");
    int use_acc = 1;
    #pragma acc parallel loop gang(num:2) if(use_acc) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] * 7;
    }
    verify_result(result, N, 34650); /* Sum of (i%100)*7 for i=0..1023 */
    
    /* Test with tile clause which might affect partitioning */
    printf("Testing with tile...\n");
    #pragma acc parallel loop tile(32) copyin(data[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = data[i] + 20;
    }
    verify_result(result, N, 25096); /* Sum of (i%100)+20 for i=0..1023 */
    
    free(data);
    free(result);
    
    printf("All tests completed.\n");
    return 0;
}
