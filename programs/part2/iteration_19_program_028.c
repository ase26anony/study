/* test_partition_types.c - Cover all partition type string mappings in GCC offloading */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#define USE_OPENACC 1
#else
#define USE_OPENACC 0
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Array size must be positive\n");
        return 1;
    }
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    int *f = (int*)malloc(n * sizeof(int));
    int *g = (int*)malloc(n * sizeof(int));
    int *h = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !e || !f || !g || !h) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = b[i] = c[i] = d[i] = e[i] = f[i] = g[i] = h[i] = 0;
    }
    
    printf("Testing %d elements with OpenACC\n", n);
    
    /* Region 1: gang redundant (case 0) */
    printf("Region 1: gang redundant\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    
    /* Region 2: gang partitioned (case 1) */
    printf("Region 2: gang partitioned\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2;
        }
    }
    
    /* Region 3: worker partitioned (case 2) */
    printf("Region 3: worker partitioned\n");
    #pragma acc parallel num_workers(4) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = b[i] + 1;
        }
    }
    
    /* Region 4: gang+worker partitioned (case 3) */
    printf("Region 4: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = c[i] * 3;
        }
    }
    
    /* Region 5: vector partitioned (case 4) */
    printf("Region 5: vector partitioned\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = d[i] / 2;
        }
    }
    
    /* Region 6: gang+vector partitioned (case 5) */
    printf("Region 6: gang+vector partitioned\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = e[i] + i;
        }
    }
    
    /* Region 7: worker+vector partitioned (case 6) */
    printf("Region 7: worker+vector partitioned\n");
    #pragma acc parallel num_workers(4) copy(g[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            g[i] = f[i] - i;
        }
    }
    
    /* Region 8: fully partitioned (case 7) */
    printf("Region 8: fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = g[i] * 2 + 1;
        }
    }
    
    /* Verification and final computation to prevent dead code elimination */
    int total_sum = 0;
    #pragma acc parallel loop reduction(+:total_sum) copyin(h[0:n])
    {
        for (int i = 0; i < n; i++) {
            total_sum += h[i];
        }
    }
    
    /* Also compute on host for verification */
    int host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += h[i];
    }
    
    printf("Device sum: %d, Host sum: %d\n", total_sum, host_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
