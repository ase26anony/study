/* Test program to cover partition type mapping in omp-oacc-neuter-broadcast.cc
   Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
   Run with: ./test_partitions 1000
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N_DEFAULT 1000

void test_gang_redundant(int n, int *a) {
    // Case 0: gang redundant
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
}

void test_gang_partitioned(int n, int *a, int *b) {
    // Case 1: gang partitioned
    #pragma acc parallel loop gang copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + 1;
    }
}

void test_worker_partitioned(int n, int *a, int *b) {
    // Case 2: worker partitioned
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    // Case 3: gang+worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] - 1;
        }
    }
}

void test_vector_partitioned(int n, int *a, int *b) {
    // Case 4: vector partitioned
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] / 2;
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    // Case 5: gang+vector partitioned
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] * a[i];
    }
}

void test_worker_vector_partitioned(int n, int *a, int *b) {
    // Case 6: worker+vector partitioned
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            b[i] = a[i] % 100;
        }
    }
}

void test_fully_partitioned(int n, int *a, int *b) {
    // Case 7: fully partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + i;
        }
    }
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    // Allocate arrays
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    assert(a != NULL && b != NULL && c != NULL);
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
        c[i] = 0;
    }
    
    // Test each partition type
    printf("1. Testing gang redundant...\n");
    test_gang_redundant(n, a);
    
    printf("2. Testing gang partitioned...\n");
    test_gang_partitioned(n, a, b);
    
    printf("3. Testing worker partitioned...\n");
    test_worker_partitioned(n, b, c);
    
    printf("4. Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, c, a);
    
    printf("5. Testing vector partitioned...\n");
    test_vector_partitioned(n, a, b);
    
    printf("6. Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, b, c);
    
    printf("7. Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, c, a);
    
    printf("8. Testing fully partitioned...\n");
    test_fully_partitioned(n, a, b);
    
    // Final verification to prevent dead code elimination
    long long sum = 0;
    #pragma acc serial copyin(b[0:n]) copy(sum)
    {
        #pragma acc loop reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += b[i];
        }
    }
    
    printf("Final sum: %lld\n", sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    
    return 0;
}
