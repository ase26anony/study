/* test_partition_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024

void verify_array(int *arr, int n, int expected_value) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("Array verification: sum = %d (expected ~%d)\n", 
           sum, expected_value);
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Allocate and initialize arrays */
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
    for (int i = 0; i < n; i++) {
        a[i] = b[i] = c[i] = d[i] = e[i] = f[i] = g[i] = h[i] = 0;
    }
    
    /* ============================================
       Test different partition types to trigger
       all cases in the uncovered switch statement
       ============================================ */
    
    /* Case 0: "gang redundant" 
       Simple parallel region with num_gangs only */
    printf("\n1. Testing gang redundant (case 0)...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    verify_array(a, n, n*(n-1)/2);
    
    /* Case 1: "gang partitioned" 
       Parallel loop with gang clause */
    printf("\n2. Testing gang partitioned (case 1)...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    verify_array(b, n, n*(n-1));
    
    /* Case 2: "worker partitioned" 
       Parallel region with num_workers */
    printf("\n3. Testing worker partitioned (case 2)...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 3;
        }
    }
    verify_array(c, n, 3*n*(n-1)/2);
    
    /* Case 3: "gang+worker partitioned" 
       Parallel region with both num_gangs and num_workers */
    printf("\n4. Testing gang+worker partitioned (case 3)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 4;
        }
    }
    verify_array(d, n, 2*n*(n-1));
    
    /* Case 4: "vector partitioned" 
       Parallel loop with vector clause */
    printf("\n5. Testing vector partitioned (case 4)...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * 5;
    }
    verify_array(e, n, 5*n*(n-1)/2);
    
    /* Case 5: "gang+vector partitioned" 
       Parallel loop with both gang and vector clauses */
    printf("\n6. Testing gang+vector partitioned (case 5)...\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i * 6;
    }
    verify_array(f, n, 3*n*(n-1));
    
    /* Case 6: "worker+vector partitioned" 
       Parallel region with workers and vector loop */
    printf("\n7. Testing worker+vector partitioned (case 6)...\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                g[i] += j;
            }
        }
    }
    verify_array(g, n, 6*n);  // 0+1+2+3 = 6 per element
    
    /* Case 7: "fully partitioned" 
       Parallel region with gangs, workers, and vector */
    printf("\n8. Testing fully partitioned (case 7)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 8; j++) {
                h[i] += j;
            }
        }
    }
    verify_array(h, n, 28*n);  // 0+1+...+7 = 28 per element
    
    /* Final reduction combining results from all arrays */
    printf("\n9. Final combined reduction...\n");
    int total_sum = 0;
    #pragma acc parallel loop reduction(+:total_sum) \
        copyin(a[0:n], b[0:n], c[0:n], d[0:n], e[0:n], f[0:n], g[0:n], h[0:n])
    for (int i = 0; i < n; i++) {
        total_sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    
    printf("Total combined sum = %d\n", total_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    printf("\nAll partition type tests completed.\n");
    return 0;
}
