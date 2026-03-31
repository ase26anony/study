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
        a[i] = a[i] + 1;
    }
}

void test_worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 2;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] / 2;
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] + i;
    }
}

void test_gang_vector_partitioned(int n, int *a) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] - i;
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + 1;
        }
    }
}

void test_fully_partitioned(int n, int *a) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(32) copy(a[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 3;
        }
    }
}

/* Alternative using OpenMP target for additional coverage */
#ifdef _OPENMP
void test_omp_target(int n, int *a) {
    /* OpenMP target version that may trigger different partition types */
    #pragma omp target teams distribute parallel for map(tofrom: a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] + 100;
    }
    
    #pragma omp target teams distribute parallel for simd map(tofrom: a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] - 50;
    }
}
#endif

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            fprintf(stderr, "Error: n must be > 0\n");
            return 1;
        }
    }
    
    printf("Testing with n = %d\n", n);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
    }
    
    /* Test each partition type */
    printf("Testing gang redundant...\n");
    test_gang_redundant(n, b);
    memcpy(a, b, n * sizeof(int));
    
    printf("Testing gang partitioned...\n");
    test_gang_partitioned(n, a);
    
    printf("Testing worker partitioned...\n");
    test_worker_partitioned(n, a);
    
    printf("Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, a);
    
    printf("Testing vector partitioned...\n");
    test_vector_partitioned(n, a);
    
    printf("Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, a);
    
    printf("Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, a);
    
    printf("Testing fully partitioned...\n");
    test_fully_partitioned(n, a);
    
#ifdef _OPENMP
    printf("Testing OpenMP target...\n");
    test_omp_target(n, a);
#endif
    
    /* Verification step to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    printf("Final sum: %lld\n", sum);
    
    /* Additional verification with reduction in serial region */
    int verify_sum = 0;
    #pragma acc serial copyin(a[0:n]) copyout(verify_sum)
    {
        verify_sum = 0;
        #pragma acc loop reduction(+:verify_sum)
        for (int i = 0; i < n; i++) {
            verify_sum += a[i];
        }
    }
    printf("Device-calculated sum: %d\n", verify_sum);
    
    free(a);
    free(b);
    
    return 0;
}
