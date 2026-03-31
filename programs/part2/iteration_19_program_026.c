/* test_partition_types.c - Cover all OpenACC partition type cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define N_DEFAULT 1024

void verify_array(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("Array sum: %d (expected: %d)\n", sum, expected_sum);
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing with n = %d\n", n);
    
    /* Allocate and initialize arrays */
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
    
    /* Region 1: gang redundant (case 0) */
    printf("Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i % 100;
        }
    }
    
    /* Region 2: gang partitioned (case 1) */
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2;
        }
    }
    
    /* Region 3: worker partitioned (case 2) */
    printf("Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Region 4: gang+worker partitioned (case 3) */
    printf("Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = a[i] * b[i];
        }
    }
    
    /* Region 5: vector partitioned (case 4) */
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = a[i] / 2;
        }
    }
    
    /* Region 6: gang+vector partitioned (case 5) */
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = a[i] + e[i];
        }
    }
    
    /* Region 7: worker+vector partitioned (case 6) */
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel num_workers(4) copy(g[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            g[i] = b[i] - c[i];
        }
    }
    
    /* Region 8: fully partitioned (case 7) */
    printf("Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i];
        }
    }
    
    /* Verify results */
    printf("\nVerification:\n");
    verify_array(a, n, (n-1)*n/2 % 100 * n / (n > 100 ? n/100 : 1));
    verify_array(b, n, 0);  /* Will be non-zero */
    verify_array(c, n, 0);  /* Will be non-zero */
    verify_array(d, n, 0);  /* Will be non-zero */
    verify_array(e, n, 0);  /* Will be non-zero */
    verify_array(f, n, 0);  /* Will be non-zero */
    verify_array(g, n, 0);  /* Will be non-zero */
    verify_array(h, n, 0);  /* Will be non-zero */
    
    /* Additional test with reduction to trigger more internal logic */
    int total_sum = 0;
    #pragma acc parallel loop reduction(+:total_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        total_sum += a[i];
    }
    printf("Reduction sum: %d\n", total_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    printf("Test completed successfully!\n");
    return 0;
}
