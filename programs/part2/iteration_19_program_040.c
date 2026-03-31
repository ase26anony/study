/* test_partition_types.c - Cover all OpenACC partition type cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024
#define VERIFY_SUM 523776  /* sum(0..1023) */

void gang_redundant(int *a, int n) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang_id(acc_gang);
        #endif
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i + gang_id * 1000;
        }
    }
}

void gang_partitioned(int *a, int n) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 2;
    }
}

void worker_partitioned(int *a, int n) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = acc_worker_id(acc_worker);
        #endif
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] = i + worker_id * 100;
        }
    }
}

void gang_worker_partitioned(int *a, int n) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang_id(acc_gang);
        worker_id = acc_worker_id(acc_worker);
        #endif
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = i + gang_id * 1000 + worker_id * 100;
        }
    }
}

void vector_partitioned(int *a, int n) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
    }
}

void gang_vector_partitioned(int *a, int n) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 4;
    }
}

void worker_vector_partitioned(int *a, int n) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            a[i] = i * 5;
        }
    }
}

void fully_partitioned(int *a, int n) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(32) copy(a[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            a[i] = i * 6;
        }
    }
}

int verify_array(int *a, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    printf("Array sum: %d (expected: %d)\n", sum, expected_sum);
    return sum == expected_sum;
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            fprintf(stderr, "Error: n must be positive\n");
            return 1;
        }
    }
    
    printf("Testing OpenACC partition types with n = %d\n", n);
    
    int *a = (int *)malloc(n * sizeof(int));
    if (!a) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Test each partition type */
    printf("\n1. Testing gang redundant (case 0)...\n");
    gang_redundant(a, n);
    
    printf("2. Testing gang partitioned (case 1)...\n");
    gang_partitioned(a, n);
    if (!verify_array(a, n, (n-1)*n)) {
        printf("Warning: Verification failed for gang partitioned\n");
    }
    
    printf("3. Testing worker partitioned (case 2)...\n");
    worker_partitioned(a, n);
    
    printf("4. Testing gang+worker partitioned (case 3)...\n");
    gang_worker_partitioned(a, n);
    
    printf("5. Testing vector partitioned (case 4)...\n");
    vector_partitioned(a, n);
    if (!verify_array(a, n, 3*(n-1)*n/2)) {
        printf("Warning: Verification failed for vector partitioned\n");
    }
    
    printf("6. Testing gang+vector partitioned (case 5)...\n");
    gang_vector_partitioned(a, n);
    if (!verify_array(a, n, 2*(n-1)*n)) {
        printf("Warning: Verification failed for gang+vector partitioned\n");
    }
    
    printf("7. Testing worker+vector partitioned (case 6)...\n");
    worker_vector_partitioned(a, n);
    if (!verify_array(a, n, 5*(n-1)*n/2)) {
        printf("Warning: Verification failed for worker+vector partitioned\n");
    }
    
    printf("8. Testing fully partitioned (case 7)...\n");
    fully_partitioned(a, n);
    if (!verify_array(a, n, 3*(n-1)*n)) {
        printf("Warning: Verification failed for fully partitioned\n");
    }
    
    /* Final reduction to ensure all computations are used */
    int final_sum = 0;
    #pragma acc parallel loop reduction(+:final_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        final_sum += a[i];
    }
    
    printf("\nFinal array sum after all operations: %d\n", final_sum);
    
    free(a);
    printf("\nAll partition type tests completed.\n");
    
    return 0;
}
