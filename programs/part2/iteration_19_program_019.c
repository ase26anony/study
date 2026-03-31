/* test_omp_oacc_partition_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024

void verify_result(const char* test_name, int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("%s: sum = %d (expected %d)\n", test_name, sum, expected_sum);
    assert(sum == expected_sum);
}

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Allocate and initialize arrays for different test cases */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    int *f = (int*)malloc(n * sizeof(int));
    int *g = (int*)malloc(n * sizeof(int));
    int *h = (int*)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = b[i] = c[i] = d[i] = e[i] = f[i] = g[i] = h[i] = 0;
    }
    
    /* Test 1: gang redundant (case 0) */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    verify_result("gang redundant", a, n, n*(n-1)/2);
    
    /* Test 2: gang partitioned (case 1) */
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    verify_result("gang partitioned", b, n, n*(n-1));
    
    /* Test 3: worker partitioned (case 2) */
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i + 1;
        }
    }
    verify_result("worker partitioned", c, n, n*(n+1)/2);
    
    /* Test 4: gang+worker partitioned (case 3) */
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 3;
        }
    }
    verify_result("gang+worker partitioned", d, n, 3*n*(n-1)/2);
    
    /* Test 5: vector partitioned (case 4) */
    #pragma acc parallel loop vector vector_length(32) copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * 4;
    }
    verify_result("vector partitioned", e, n, 2*n*(n-1));
    
    /* Test 6: gang+vector partitioned (case 5) */
    #pragma acc parallel loop gang vector vector_length(32) copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i * 5;
    }
    verify_result("gang+vector partitioned", f, n, 5*n*(n-1)/2);
    
    /* Test 7: worker+vector partitioned (case 6) */
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            g[i] = i * 6;
        }
    }
    verify_result("worker+vector partitioned", g, n, 3*n*(n-1));
    
    /* Test 8: fully partitioned (case 7) */
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            h[i] = i * 7;
        }
    }
    verify_result("fully partitioned", h, n, 7*n*(n-1)/2);
    
    /* Combined verification to prevent optimization */
    int total_sum = 0;
    for (int i = 0; i < n; i++) {
        total_sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    printf("Total combined sum: %d\n", total_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    printf("All partition tests completed successfully!\n");
    return 0;
}
