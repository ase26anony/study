/* test_partition_coverage.c - Cover all partition types in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N_DEFAULT 1024

/* Helper to initialize arrays */
void init_array(int *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = i + seed;
    }
}

/* Helper to verify arrays */
int verify_array(int *arr, int n, int expected_base) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Allocate arrays for different test cases */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *d = (int *)malloc(n * sizeof(int));
    int *e = (int *)malloc(n * sizeof(int));
    int *f = (int *)malloc(n * sizeof(int));
    int *g = (int *)malloc(n * sizeof(int));
    int *h = (int *)malloc(n * sizeof(int));
    
    assert(a && b && c && d && e && f && g && h);
    
    /* Initialize arrays */
    init_array(a, n, 1);
    init_array(b, n, 2);
    init_array(c, n, 3);
    init_array(d, n, 4);
    init_array(e, n, 5);
    init_array(f, n, 6);
    init_array(g, n, 7);
    init_array(h, n, 8);
    
    int total_sum = 0;
    
    /* Test 1: gang redundant (case 0) */
    printf("Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
    total_sum += verify_array(a, n, 0);
    
    /* Test 2: gang partitioned (case 1) */
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i * 3;
        }
    }
    total_sum += verify_array(b, n, 0);
    
    /* Test 3: worker partitioned (case 2) */
    printf("Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 4;
        }
    }
    total_sum += verify_array(c, n, 0);
    
    /* Test 4: gang+worker partitioned (case 3) */
    printf("Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 5;
        }
    }
    total_sum += verify_array(d, n, 0);
    
    /* Test 5: vector partitioned (case 4) */
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector vector_length(32) copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i * 6;
        }
    }
    total_sum += verify_array(e, n, 0);
    
    /* Test 6: gang+vector partitioned (case 5) */
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector vector_length(32) copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i * 7;
        }
    }
    total_sum += verify_array(f, n, 0);
    
    /* Test 7: worker+vector partitioned (case 6) */
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel num_workers(4) copy(g[0:n])
    {
        #pragma acc loop worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            g[i] = i * 8;
        }
    }
    total_sum += verify_array(g, n, 0);
    
    /* Test 8: fully partitioned (case 7) */
    printf("Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(h[0:n])
    {
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            h[i] = i * 9;
        }
    }
    total_sum += verify_array(h, n, 0);
    
    /* Final reduction to ensure all computations are used */
    int final_result = 0;
    #pragma acc serial copyin(total_sum) copyout(final_result)
    {
        final_result = total_sum;
    }
    
    printf("Final result: %d\n", final_result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
