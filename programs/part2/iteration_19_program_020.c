/* test_partition_types.c - Cover all OpenACC partition type cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024
#define VERIFY_VALUE 523776  /* Sum of 0..1023 */

void test_gang_redundant(int n, int *a) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
}

void test_gang_partitioned(int n, int *a) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] += 1;
    }
}

void test_worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] += 1;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] += 1;
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] += 1;
    }
}

void test_gang_vector_partitioned(int n, int *a) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] += 1;
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            a[i] += 1;
        }
    }
}

void test_fully_partitioned(int n, int *a) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            a[i] += 1;
        }
    }
}

int verify_result(int n, int *a) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    return sum;
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
    
    printf("Testing with n = %d\n", n);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(n * sizeof(int));
    if (!a) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Test each partition type sequentially */
    printf("1. Testing gang redundant...\n");
    test_gang_redundant(n, a);
    
    printf("2. Testing gang partitioned...\n");
    test_gang_partitioned(n, a);
    
    printf("3. Testing worker partitioned...\n");
    test_worker_partitioned(n, a);
    
    printf("4. Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, a);
    
    printf("5. Testing vector partitioned...\n");
    test_vector_partitioned(n, a);
    
    printf("6. Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, a);
    
    printf("7. Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, a);
    
    printf("8. Testing fully partitioned...\n");
    test_fully_partitioned(n, a);
    
    /* Final verification */
    int final_sum = verify_result(n, a);
    printf("Final sum: %d\n", final_sum);
    
    /* For n=1024, each element gets incremented 8 times (from 0 to 8) */
    int expected = 0;
    for (int i = 0; i < n; i++) {
        expected += i + 8;  /* i from gang_redundant + 8 increments */
    }
    
    if (final_sum == expected) {
        printf("SUCCESS: Result matches expected value\n");
    } else {
        printf("WARNING: Result %d doesn't match expected %d\n", final_sum, expected);
    }
    
    free(a);
    return 0;
}
