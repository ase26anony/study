/* test_partition_coverage.c */
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
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Region 1: gang redundant (case 0) */
    printf("Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
    
    /* Region 2: gang partitioned (case 1) */
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i * 3;
        }
    }
    
    /* Region 3: worker partitioned (case 2) */
    printf("Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 4;
        }
    }
    
    /* Region 4: gang+worker partitioned (case 3) */
    printf("Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 5;
        }
    }
    
    /* Region 5: vector partitioned (case 4) */
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i * 6;
        }
    }
    
    /* Region 6: gang+vector partitioned (case 5) */
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i * 7;
        }
    }
    
    /* Region 7: worker+vector partitioned (case 6) */
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            g[i] = i * 8;
        }
    }
    
    /* Region 8: fully partitioned (case 7) */
    printf("Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = i * 9;
        }
    }
    
    /* Verification and final computation to prevent dead code elimination */
    printf("Verifying results...\n");
    long long total_sum = 0;
    
    #pragma acc parallel loop reduction(+:total_sum) copyin(a[0:n], b[0:n], c[0:n], d[0:n], \
                                                           e[0:n], f[0:n], g[0:n], h[0:n])
    for (int i = 0; i < n; i++) {
        total_sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    
    /* Host-side verification */
    long long host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    
    printf("Device sum: %lld\n", total_sum);
    printf("Host sum: %lld\n", host_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
