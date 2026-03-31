/* test_partition_types.c - Coverage test for omp-oacc-neuter-broadcast.cc partition type strings */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_DEFAULT 1024

void verify_array(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    if (sum != expected_sum) {
        fprintf(stderr, "Verification failed: sum = %d, expected = %d\n", 
                sum, expected_sum);
        exit(1);
    }
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
    
    if (!a || !b || !c || !d || !e || !f || !g || !h) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = b[i] = c[i] = d[i] = e[i] = f[i] = g[i] = h[i] = 0;
    }
    
    /* Region 1: gang redundant (case 0) */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        #pragma acc loop independent
        for (int i = 0; i < n; i++) {
            a[i] = i + gang_id;
        }
    }
    verify_array(a, n, (n-1)*n/2 + 0);  // gang_id is 0 without OpenACC
    
    /* Region 2: gang partitioned (case 1) */
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    verify_array(b, n, (n-1)*n);
    
    /* Region 3: worker partitioned (case 2) */
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i + 1;
        }
    }
    verify_array(c, n, (n+1)*n/2);
    
    /* Region 4: gang+worker partitioned (case 3) */
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 3;
        }
    }
    verify_array(d, n, 3*(n-1)*n/2);
    
    /* Region 5: vector partitioned (case 4) */
    #pragma acc parallel loop vector vector_length(32) copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * 4;
    }
    verify_array(e, n, 2*(n-1)*n);
    
    /* Region 6: gang+vector partitioned (case 5) */
    #pragma acc parallel loop gang vector vector_length(32) copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i * 5;
    }
    verify_array(f, n, 5*(n-1)*n/2);
    
    /* Region 7: worker+vector partitioned (case 6) */
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            g[i] = i * 6;
        }
    }
    verify_array(g, n, 3*(n-1)*n);
    
    /* Region 8: fully partitioned (case 7) */
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            h[i] = i * 7;
        }
    }
    verify_array(h, n, 7*(n-1)*n/2);
    
    /* Final reduction across all arrays */
    int total_sum = 0;
    #pragma acc serial copyin(a[0:n], b[0:n], c[0:n], d[0:n], \
                              e[0:n], f[0:n], g[0:n], h[0:n]) \
                     copyout(total_sum)
    {
        total_sum = 0;
        #pragma acc loop reduction(+:total_sum)
        for (int i = 0; i < n; i++) {
            total_sum += a[i] + b[i] + c[i] + d[i] + 
                        e[i] + f[i] + g[i] + h[i];
        }
    }
    
    printf("All partition types tested successfully!\n");
    printf("Total sum across all arrays: %d\n", total_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
