/* test-omp-oacc-partition-types.c */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test-omp-oacc-partition-types.c */
/* Or with: gcc -O2 -fopenmp -foffload=amd -o test_partition test-omp-oacc-partition-types.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
        a[i] = b[i] + 1;
    }
}

void test_worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] += i;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyin(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * 2;
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] * 3;
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(64) copy(a[0:n]) copyin(b[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = b[i] - a[i];
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            a[i] += 5;
        }
    }
}

void test_fully_partitioned(int n, int *a, int *b) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyin(b[0:n])
    {
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + b[i] * 2;
        }
    }
}

int main(int argc, char **argv) {
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
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < n; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = 0;
    }
    
    /* Test each partition type sequentially */
    printf("Testing gang redundant...\n");
    test_gang_redundant(n, c);
    
    printf("Testing gang partitioned...\n");
    test_gang_partitioned(n, a, b);
    
    printf("Testing worker partitioned...\n");
    test_worker_partitioned(n, a);
    
    printf("Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, c, a);
    
    printf("Testing vector partitioned...\n");
    test_vector_partitioned(n, b);
    
    printf("Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, a, b);
    
    printf("Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, c);
    
    printf("Testing fully partitioned...\n");
    test_fully_partitioned(n, a, b);
    
    /* Final verification to prevent dead code elimination */
    long long sum = 0;
    #pragma acc serial copyin(a[0:n], b[0:n], c[0:n]) copy(sum)
    {
        for (int i = 0; i < n; i++) {
            sum += a[i] + b[i] + c[i];
        }
    }
    
    printf("Final sum: %lld\n", sum);
    
    /* Additional host-side verification */
    long long host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += a[i];
    }
    printf("Host sum of array a: %lld\n", host_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
