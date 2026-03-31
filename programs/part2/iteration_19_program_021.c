/* test_partition_types.c - Cover all OpenACC partition type cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024

void test_gang_redundant(int n, int *a) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
}

void test_gang_partitioned(int n, int *a, int *b) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n]) copyin(b[0:n])
    for (int i = 0; i < n; i++) {
        a[i] += b[i];
    }
}

void test_worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 3;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyin(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + b[i] * 2;
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] - 5;
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(64) copy(a[0:n]) copyin(b[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] * b[i];
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            a[i] = a[i] / 2;
        }
    }
}

void test_fully_partitioned(int n, int *a, int *b, int *c) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) copy(a[0:n]) copyin(b[0:n], c[0:n])
    {
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            a[i] = (a[i] + b[i]) * c[i];
        }
    }
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
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i % 10;
        c[i] = (i % 5) + 1;
    }
    
    /* Test all partition types sequentially */
    test_gang_redundant(n, a);
    printf("  Gang redundant completed\n");
    
    test_gang_partitioned(n, a, b);
    printf("  Gang partitioned completed\n");
    
    test_worker_partitioned(n, a);
    printf("  Worker partitioned completed\n");
    
    test_gang_worker_partitioned(n, a, b);
    printf("  Gang+worker partitioned completed\n");
    
    test_vector_partitioned(n, a);
    printf("  Vector partitioned completed\n");
    
    test_gang_vector_partitioned(n, a, b);
    printf("  Gang+vector partitioned completed\n");
    
    test_worker_vector_partitioned(n, a);
    printf("  Worker+vector partitioned completed\n");
    
    test_fully_partitioned(n, a, b, c);
    printf("  Fully partitioned completed\n");
    
    /* Verification step to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    printf("Final sum: %lld\n", sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
