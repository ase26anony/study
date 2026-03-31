/* test_partition_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024

void test_gang_redundant(int n, int *a) {
    printf("Testing gang redundant (case 0)...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
}

void test_gang_partitioned(int n, int *a, int *b) {
    printf("Testing gang partitioned (case 1)...\n");
    #pragma acc parallel loop gang copy(a[0:n]) copyin(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            a[i] += b[i];
        }
    }
}

void test_worker_partitioned(int n, int *a) {
    printf("Testing worker partitioned (case 2)...\n");
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 3;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    printf("Testing gang+worker partitioned (case 3)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyin(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + b[i] * 2;
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    printf("Testing vector partitioned (case 4)...\n");
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    {
        for (int i = 0; i < n; i++) {
            a[i] = a[i] - i;
        }
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    printf("Testing gang+vector partitioned (case 5)...\n");
    #pragma acc parallel loop gang vector vector_length(64) copy(a[0:n]) copyin(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * b[i];
        }
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    printf("Testing worker+vector partitioned (case 6)...\n");
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i += 4) {
            #pragma acc loop vector vector_length(16)
            for (int j = 0; j < 4 && (i + j) < n; j++) {
                a[i + j] = a[i + j] + 1;
            }
        }
    }
}

void test_fully_partitioned(int n, int *a, int *b, int *c) {
    printf("Testing fully partitioned (case 7)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyin(b[0:n], c[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i += 8) {
            #pragma acc loop vector vector_length(32)
            for (int j = 0; j < 8 && (i + j) < n; j++) {
                a[i + j] = (b[i + j] + c[i + j]) * a[i + j];
            }
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
    
    printf("Running partition coverage test with n = %d\n", n);
    
    // Allocate and initialize arrays
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = 1;
        b[i] = i % 10;
        c[i] = (i % 5) + 1;
    }
    
    // Test all partition types
    test_gang_redundant(n, a);
    
    // Reset a for next test
    for (int i = 0; i < n; i++) a[i] = i;
    test_gang_partitioned(n, a, b);
    
    test_worker_partitioned(n, a);
    test_gang_worker_partitioned(n, a, b);
    test_vector_partitioned(n, a);
    test_gang_vector_partitioned(n, a, b);
    test_worker_vector_partitioned(n, a);
    test_fully_partitioned(n, a, b, c);
    
    // Final verification to prevent dead code elimination
    int sum = 0;
    #pragma acc serial copyin(a[0:n]) copy(sum)
    {
        for (int i = 0; i < n; i++) {
            sum += a[i];
        }
    }
    
    // Also compute on host for verification
    int host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += a[i];
    }
    
    printf("Final sum (device): %d\n", sum);
    printf("Final sum (host): %d\n", host_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    
    return 0;
}
