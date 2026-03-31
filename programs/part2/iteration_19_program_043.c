/* test_omp_oacc_partition_types.c
 * 
 * Test program to cover partition type mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_omp_oacc_partition_types.c
 * Run with: ./test_partition 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N_DEFAULT 1000

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
    #pragma acc parallel loop gang copy(a[0:n]) copyout(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + 1;
        }
    }
}

void test_worker_partitioned(int n, int *a, int *b) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] - 1;
        }
    }
}

void test_vector_partitioned(int n, int *a, int *b) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = a[i] / 2;
        }
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * a[i];
        }
    }
}

void test_worker_vector_partitioned(int n, int *a, int *b) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector vector_length(16)
            for (int j = 0; j < 4; j++) {
                b[i] += j;
            }
        }
    }
}

void test_fully_partitioned(int n, int *a, int *b, int *c) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copy(b[0:n]) copyout(c[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            fprintf(stderr, "Error: n must be > 0\n");
            return 1;
        }
    }
    
    printf("Testing partition types with n = %d\n", n);
    
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
        b[i] = 0;
        c[i] = 0;
    }
    
    /* Test each partition type sequentially */
    printf("1. Testing gang redundant...\n");
    test_gang_redundant(n, a);
    
    printf("2. Testing gang partitioned...\n");
    test_gang_partitioned(n, a, b);
    
    printf("3. Testing worker partitioned...\n");
    test_worker_partitioned(n, a, b);
    
    printf("4. Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, a, b);
    
    printf("5. Testing vector partitioned...\n");
    test_vector_partitioned(n, a, b);
    
    printf("6. Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, a, b);
    
    printf("7. Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, a, b);
    
    printf("8. Testing fully partitioned...\n");
    test_fully_partitioned(n, a, b, c);
    
    /* Verification step to prevent dead code elimination */
    long long sum = 0;
    #pragma acc serial copyin(a[0:n], b[0:n], c[0:n]) copy(sum)
    {
        for (int i = 0; i < n; i++) {
            sum += a[i] + b[i] + c[i];
        }
    }
    
    printf("Final sum: %lld\n", sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
