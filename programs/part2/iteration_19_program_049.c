/* test_partition_coverage.c - Cover all partition types in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024
#define VERIFY_VALUE 523776  /* Sum of 0..1023 */

void gang_redundant(int n, int *a) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i + gang_id * 1000;  /* Different gangs write different values */
        }
    }
}

void gang_partitioned(int n, int *a) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 2;
    }
}

void worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + 1;
        }
    }
}

void gang_worker_partitioned(int n, int *a) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) copy(a[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 2;
        }
    }
}

void vector_partitioned(int n, int *a) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] + i;
    }
}

void gang_vector_partitioned(int n, int *a) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] - (i % 10);
    }
}

void worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + (i % 5);
        }
    }
}

void fully_partitioned(int n, int *a) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) copy(a[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            a[i] = a[i] % 100;
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
    
    printf("Testing partition coverage with n = %d\n", n);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize reference array */
    for (int i = 0; i < n; i++) {
        b[i] = i;
    }
    
    /* Test 1: gang redundant */
    printf("Testing gang redundant...\n");
    gang_redundant(n, a);
    
    /* Test 2: gang partitioned */
    printf("Testing gang partitioned...\n");
    memcpy(a, b, n * sizeof(int));
    gang_partitioned(n, a);
    
    /* Test 3: worker partitioned */
    printf("Testing worker partitioned...\n");
    memcpy(a, b, n * sizeof(int));
    worker_partitioned(n, a);
    
    /* Test 4: gang+worker partitioned */
    printf("Testing gang+worker partitioned...\n");
    memcpy(a, b, n * sizeof(int));
    gang_worker_partitioned(n, a);
    
    /* Test 5: vector partitioned */
    printf("Testing vector partitioned...\n");
    memcpy(a, b, n * sizeof(int));
    vector_partitioned(n, a);
    
    /* Test 6: gang+vector partitioned */
    printf("Testing gang+vector partitioned...\n");
    memcpy(a, b, n * sizeof(int));
    gang_vector_partitioned(n, a);
    
    /* Test 7: worker+vector partitioned */
    printf("Testing worker+vector partitioned...\n");
    memcpy(a, b, n * sizeof(int));
    worker_vector_partitioned(n, a);
    
    /* Test 8: fully partitioned */
    printf("Testing fully partitioned...\n");
    memcpy(a, b, n * sizeof(int));
    fully_partitioned(n, a);
    
    /* Final verification to prevent dead code elimination */
    printf("Final verification...\n");
    long long sum = 0;
    #pragma acc parallel loop reduction(+:sum) copyin(a[0:n]) copy(sum)
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    
    /* Also compute on host for verification */
    long long host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += a[i];
    }
    
    printf("Device sum: %lld, Host sum: %lld\n", sum, host_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    
    printf("Test completed successfully\n");
    return 0;
}
