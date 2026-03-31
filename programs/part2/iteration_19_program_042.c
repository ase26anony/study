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
        a[i] += i * 2;
    }
}

void test_worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] -= i;
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
        a[i] *= 2;
    }
}

void test_gang_vector_partitioned(int n, int *a) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(64) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] -= 1;
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            a[i] += i % 10;
        }
    }
}

void test_fully_partitioned(int n, int *a) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            a[i] = (a[i] + i) % 1000;
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
    if (!a) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize to zero */
    memset(a, 0, n * sizeof(int));
    
    /* Execute all test cases to trigger different partition types */
    test_gang_redundant(n, a);
    
    /* Reset for next test */
    for (int i = 0; i < n; i++) a[i] = 0;
    test_gang_partitioned(n, a);
    
    for (int i = 0; i < n; i++) a[i] = 0;
    test_worker_partitioned(n, a);
    
    for (int i = 0; i < n; i++) a[i] = 0;
    test_gang_worker_partitioned(n, a);
    
    for (int i = 0; i < n; i++) a[i] = 0;
    test_vector_partitioned(n, a);
    
    for (int i = 0; i < n; i++) a[i] = 0;
    test_gang_vector_partitioned(n, a);
    
    for (int i = 0; i < n; i++) a[i] = 0;
    test_worker_vector_partitioned(n, a);
    
    for (int i = 0; i < n; i++) a[i] = 0;
    test_fully_partitioned(n, a);
    
    /* Final verification to prevent dead code elimination */
    long long sum = 0;
    #pragma acc serial copyin(a[0:n]) copy(sum)
    {
        for (int i = 0; i < n; i++) {
            sum += a[i];
        }
    }
    
    /* Also compute on host for verification */
    long long host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += a[i];
    }
    
    printf("Device sum: %lld, Host sum: %lld\n", sum, host_sum);
    
    /* Additional test with OpenMP target for broader coverage */
    #ifdef _OPENMP
    {
        int *b = (int *)malloc(n * sizeof(int));
        #pragma omp target teams distribute parallel for map(tofrom: b[0:n])
        for (int i = 0; i < n; i++) {
            b[i] = i * 3;
        }
        
        long long omp_sum = 0;
        for (int i = 0; i < n; i++) {
            omp_sum += b[i];
        }
        printf("OpenMP target sum: %lld\n", omp_sum);
        free(b);
    }
    #endif
    
    free(a);
    
    printf("All tests completed\n");
    return 0;
}
