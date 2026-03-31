/* test_partition_types.c - Cover all partition type cases in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024

/* Helper function to initialize array */
void init_array(int *arr, int n, int start) {
    for (int i = 0; i < n; i++) {
        arr[i] = start + i;
    }
}

/* Helper function to verify array */
int verify_array(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("Array sum: %d (expected: %d)\n", sum, expected_sum);
    return sum == expected_sum;
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing with n = %d\n", n);
    
    /* Allocate and initialize arrays for different test cases */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *d = (int *)malloc(n * sizeof(int));
    int *e = (int *)malloc(n * sizeof(int));
    int *f = (int *)malloc(n * sizeof(int));
    int *g = (int *)malloc(n * sizeof(int));
    int *h = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !e || !f || !g || !h) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with different patterns */
    init_array(a, n, 0);
    init_array(b, n, 100);
    init_array(c, n, 200);
    init_array(d, n, 300);
    init_array(e, n, 400);
    init_array(f, n, 500);
    init_array(g, n, 600);
    init_array(h, n, 700);
    
    int total_checks = 0;
    
    /* Test Case 1: gang redundant (case 0) */
    printf("\n=== Test 1: gang redundant ===\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
    total_checks += verify_array(a, n, n*(n-1));
    
    /* Test Case 2: gang partitioned (case 1) */
    printf("\n=== Test 2: gang partitioned ===\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i * 3;
        }
    }
    total_checks += verify_array(b, n, (n*(n-1)*3)/2);
    
    /* Test Case 3: worker partitioned (case 2) */
    printf("\n=== Test 3: worker partitioned ===\n");
    #pragma acc parallel num_workers(4) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 4;
        }
    }
    total_checks += verify_array(c, n, n*(n-1)*2);
    
    /* Test Case 4: gang+worker partitioned (case 3) */
    printf("\n=== Test 4: gang+worker partitioned ===\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 5;
        }
    }
    total_checks += verify_array(d, n, (n*(n-1)*5)/2);
    
    /* Test Case 5: vector partitioned (case 4) */
    printf("\n=== Test 5: vector partitioned ===\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i * 6;
        }
    }
    total_checks += verify_array(e, n, n*(n-1)*3);
    
    /* Test Case 6: gang+vector partitioned (case 5) */
    printf("\n=== Test 6: gang+vector partitioned ===\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i * 7;
        }
    }
    total_checks += verify_array(f, n, (n*(n-1)*7)/2);
    
    /* Test Case 7: worker+vector partitioned (case 6) */
    printf("\n=== Test 7: worker+vector partitioned ===\n");
    #pragma acc parallel num_workers(4) copy(g[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            g[i] = i * 8;
        }
    }
    total_checks += verify_array(g, n, n*(n-1)*4);
    
    /* Test Case 8: fully partitioned (case 7) */
    printf("\n=== Test 8: fully partitioned ===\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = i * 9;
        }
    }
    total_checks += verify_array(h, n, (n*(n-1)*9)/2);
    
    /* Additional test with reduction to ensure variable broadcasting */
    printf("\n=== Additional test: reduction with mixed partitioning ===\n");
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    #pragma acc parallel num_gangs(4) copy(sum1)
    {
        #pragma acc loop gang reduction(+:sum1)
        for (int i = 0; i < n; i++) {
            sum1 += 1;
        }
    }
    
    #pragma acc parallel num_gangs(2) num_workers(2) copy(sum2)
    {
        #pragma acc loop gang worker reduction(+:sum2)
        for (int i = 0; i < n; i++) {
            sum2 += 2;
        }
    }
    
    #pragma acc parallel num_gangs(2) num_workers(2) copy(sum3)
    {
        #pragma acc loop gang worker vector reduction(+:sum3)
        for (int i = 0; i < n; i++) {
            sum3 += 3;
        }
    }
    
    printf("Reduction sums: %d + %d + %d = %d\n", sum1, sum2, sum3, sum1 + sum2 + sum3);
    total_checks += (sum1 == n) && (sum2 == 2*n) && (sum3 == 3*n);
    
    /* Final verification */
    printf("\n=== Final Results ===\n");
    printf("Total checks passed: %d/9\n", total_checks);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return (total_checks == 9) ? 0 : 1;
}
