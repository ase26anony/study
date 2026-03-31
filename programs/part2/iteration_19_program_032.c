/* test_omp_oacc_partition_coverage.c
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_omp_oacc_partition_coverage.c
 * Run with: ./test_partition 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ARRAY_SIZE 1024

void test_gang_redundant(int n, int *a) {
    // Case 0: gang redundant
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = gang_id * 1000 + i;
        }
    }
}

void test_gang_partitioned(int n, int *a, int *b) {
    // Case 1: gang partitioned
    #pragma acc parallel loop gang copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] * 2;
    }
}

void test_worker_partitioned(int n, int *a) {
    // Case 2: worker partitioned
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = __pgi_workeridx();
        #endif
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] += worker_id;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    // Case 3: gang+worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copy(b[0:n])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        worker_id = __pgi_workeridx();
        #endif
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + gang_id * 100 + worker_id * 10;
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    // Case 4: vector partitioned
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = a[i] * a[i];
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    // Case 5: gang+vector partitioned
    #pragma acc parallel loop gang vector vector_length(64) copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] / 2;
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    // Case 6: worker+vector partitioned
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            a[i] = a[i] % 17;
        }
    }
}

void test_fully_partitioned(int n, int *a, int *b, int *c) {
    // Case 7: fully partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copy(b[0:n]) copyout(c[0:n])
    {
        int gang_id = 0, worker_id = 0, vector_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        worker_id = __pgi_workeridx();
        vector_id = __pgi_vectoridx();
        #endif
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i] + gang_id * 1000 + worker_id * 100 + vector_id * 10;
        }
    }
}

int main(int argc, char *argv[]) {
    int n = ARRAY_SIZE;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = ARRAY_SIZE;
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    // Allocate arrays
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *d = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
        c[i] = 0;
        d[i] = 0;
    }
    
    // Test each partition type
    printf("1. Testing gang redundant...\n");
    test_gang_redundant(n, d);
    
    printf("2. Testing gang partitioned...\n");
    test_gang_partitioned(n, a, c);
    
    printf("3. Testing worker partitioned...\n");
    test_worker_partitioned(n, a);
    
    printf("4. Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, a, b);
    
    printf("5. Testing vector partitioned...\n");
    test_vector_partitioned(n, a);
    
    printf("6. Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, a, c);
    
    printf("7. Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, b);
    
    printf("8. Testing fully partitioned...\n");
    test_fully_partitioned(n, a, b, d);
    
    // Verification step to prevent dead code elimination
    int sum = 0;
    #pragma acc serial copyin(a[0:n], b[0:n], c[0:n], d[0:n]) copy(sum)
    {
        #pragma acc loop reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += a[i] + b[i] + c[i] + d[i];
        }
    }
    
    printf("Final sum for verification: %d\n", sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
