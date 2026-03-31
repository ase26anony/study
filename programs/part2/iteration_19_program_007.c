/* test_partition_types.c - Coverage for omp-oacc-neuter-broadcast.cc partition type strings */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#define USE_OPENACC 1
#else
#define USE_OPENACC 0
#endif

#ifdef _OPENMP
#define USE_OPENMP 1
#else
#define USE_OPENMP 0
#endif

void verify_array(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("Array sum: %d (expected: %d)\n", sum, expected_sum);
    assert(sum == expected_sum);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 0) n = 1000;
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    int *f = (int*)malloc(n * sizeof(int));
    int *g = (int*)malloc(n * sizeof(int));
    int *h = (int*)malloc(n * sizeof(int));
    
    /* Initialize to zero */
    memset(a, 0, n * sizeof(int));
    memset(b, 0, n * sizeof(int));
    memset(c, 0, n * sizeof(int));
    memset(d, 0, n * sizeof(int));
    memset(e, 0, n * sizeof(int));
    memset(f, 0, n * sizeof(int));
    memset(g, 0, n * sizeof(int));
    memset(h, 0, n * sizeof(int));
    
    /* Test 1: gang redundant (case 0) */
    printf("\n1. Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i % 100;
        }
    }
    verify_array(a, n, (n-1)*n/2 % 100 * (n/100) + ((n%100)-1)*(n%100)/2);
    
    /* Test 2: gang partitioned (case 1) */
    printf("\n2. Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    verify_array(b, n, (n-1)*n);
    
    /* Test 3: worker partitioned (case 2) */
    printf("\n3. Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i + 1;
        }
    }
    verify_array(c, n, n*(n+1)/2);
    
    /* Test 4: gang+worker partitioned (case 3) */
    printf("\n4. Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 3;
        }
    }
    verify_array(d, n, 3*(n-1)*n/2);
    
    /* Test 5: vector partitioned (case 4) */
    printf("\n5. Testing vector partitioned...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * i;
    }
    verify_array(e, n, (n-1)*n*(2*n-1)/6);
    
    /* Test 6: gang+vector partitioned (case 5) */
    printf("\n6. Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i % 10;
    }
    int expected_sum6 = (n/10)*45 + ((n%10)-1)*(n%10)/2;
    verify_array(f, n, expected_sum6);
    
    /* Test 7: worker+vector partitioned (case 6) */
    printf("\n7. Testing worker+vector partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            g[i] = (i % 20) + 1;
        }
    }
    int full_cycles7 = n / 20;
    int remainder7 = n % 20;
    int expected_sum7 = full_cycles7 * 210 + remainder7*(remainder7+1)/2;
    verify_array(g, n, expected_sum7);
    
    /* Test 8: fully partitioned (case 7) */
    printf("\n8. Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = (i % 5) * 2;
        }
    }
    int full_cycles8 = n / 5;
    int remainder8 = n % 5;
    int cycle_sum = 0 + 2 + 4 + 6 + 8;  /* 0*2 + 1*2 + 2*2 + 3*2 + 4*2 */
    int expected_sum8 = full_cycles8 * cycle_sum + remainder8*(remainder8-1);
    verify_array(h, n, expected_sum8);
    
    /* Final combined operation to ensure all data is used */
    printf("\nFinal combined result...\n");
    int total_sum = 0;
    #pragma acc parallel loop reduction(+:total_sum) copyin(a[0:n],b[0:n],c[0:n],d[0:n],e[0:n],f[0:n],g[0:n],h[0:n])
    for (int i = 0; i < n; i++) {
        total_sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    printf("Total combined sum: %d\n", total_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
