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
    #pragma acc parallel loop vector copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] *= 2;
    }
}

void test_gang_vector_partitioned(int n, int *a) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] -= 1;
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            a[i] += i % 10;
        }
    }
}

void test_fully_partitioned(int n, int *a) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            a[i] = (a[i] + i) % 1000;
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
    
    /* Test all partition types sequentially */
    memset(a, 0, n * sizeof(int));
    
    test_gang_redundant(n, a);
    printf("  Gang redundant complete\n");
    
    test_gang_partitioned(n, a);
    printf("  Gang partitioned complete\n");
    
    test_worker_partitioned(n, a);
    printf("  Worker partitioned complete\n");
    
    test_gang_worker_partitioned(n, a);
    printf("  Gang+worker partitioned complete\n");
    
    test_vector_partitioned(n, a);
    printf("  Vector partitioned complete\n");
    
    test_gang_vector_partitioned(n, a);
    printf("  Gang+vector partitioned complete\n");
    
    test_worker_vector_partitioned(n, a);
    printf("  Worker+vector partitioned complete\n");
    
    test_fully_partitioned(n, a);
    printf("  Fully partitioned complete\n");
    
    /* Final verification */
    int final_sum = verify_result(n, a);
    printf("Final sum: %d\n", final_sum);
    
    /* Additional test with reduction to trigger more internal paths */
    int reduction_sum = 0;
    #pragma acc parallel loop reduction(+:reduction_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        reduction_sum += a[i];
    }
    printf("Reduction sum: %d\n", reduction_sum);
    
    /* Test with async to trigger different code paths */
    int async_id = 1;
    #pragma acc parallel loop async(async_id) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] % 256;
    }
    #pragma acc wait(async_id)
    
    /* Final host-side computation to prevent optimization */
    int host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += a[i];
    }
    printf("Host sum: %d\n", host_sum);
    
    free(a);
    
    /* Also test OpenMP target offload if OpenACC not available */
    #ifdef _OPENMP
    printf("\nTesting OpenMP target offload...\n");
    int *b = (int *)malloc(n * sizeof(int));
    #pragma omp target teams distribute parallel for map(tofrom:b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * i;
    }
    
    int omp_sum = 0;
    for (int i = 0; i < n; i++) {
        omp_sum += b[i];
    }
    printf("OpenMP target sum: %d\n", omp_sum);
    free(b);
    #endif
    
    return 0;
}
