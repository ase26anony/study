/* test_omp_oacc_partition_types.c */
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
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 0) n = 1000;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    int *f = (int*)malloc(n * sizeof(int));
    int *g = (int*)malloc(n * sizeof(int));
    int *h = (int*)malloc(n * sizeof(int));
    
    assert(a && b && c && d && e && f && g && h);
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = b[i] = c[i] = d[i] = e[i] = f[i] = g[i] = h[i] = 0;
    }
    
    printf("Testing with n = %d\n", n);
    
    /* Region 1: gang redundant (case 0) */
    /* Simple parallel region with num_gangs only */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    
    /* Region 2: gang partitioned (case 1) */
    /* Parallel loop with gang clause */
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] * 2;
    }
    
    /* Region 3: worker partitioned (case 2) */
    /* Parallel region with num_workers */
    #pragma acc parallel num_workers(8) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = b[i] + 1;
        }
    }
    
    /* Region 4: gang+worker partitioned (case 3) */
    /* Parallel region with both num_gangs and num_workers */
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = c[i] * 3;
        }
    }
    
    /* Region 5: vector partitioned (case 4) */
    /* Parallel loop with vector clause */
    #pragma acc parallel loop vector copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = d[i] / 2;
    }
    
    /* Region 6: gang+vector partitioned (case 5) */
    /* Parallel loop with both gang and vector clauses */
    #pragma acc parallel loop gang vector copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = e[i] + i;
    }
    
    /* Region 7: worker+vector partitioned (case 6) */
    /* Parallel region with num_workers and inner vector loop */
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                g[i] += f[i] + j;
            }
        }
    }
    
    /* Region 8: fully partitioned (case 7) */
    /* Parallel region with all three levels */
    #pragma acc parallel num_gangs(2) num_workers(2) copyout(h[0:n])
    {
        #pragma acc loop gang
        for (int gi = 0; gi < 2; gi++) {
            #pragma acc loop worker
            for (int wi = 0; wi < 2; wi++) {
                int start = (gi * 2 + wi) * (n / 4);
                int end = start + (n / 4);
                #pragma acc loop vector
                for (int i = start; i < end && i < n; i++) {
                    h[i] = g[i] * 2;
                }
            }
        }
    }
    
    /* Verification and final computation to prevent dead code elimination */
    int total_sum = 0;
    #pragma acc parallel loop reduction(+:total_sum) copyin(h[0:n])
    for (int i = 0; i < n; i++) {
        total_sum += h[i];
    }
    
    /* Also compute on host for verification */
    int host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += h[i];
    }
    
    printf("Device total sum: %d\n", total_sum);
    printf("Host total sum: %d\n", host_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
