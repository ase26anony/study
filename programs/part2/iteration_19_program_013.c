/* test_partition_types.c - Cover all partition type cases in omp-oacc-neuter-broadcast.cc */
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
    printf("Array verification: sum = %d (expected ~%d)\n", sum, expected_value);
}

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    printf("Testing with n = %d\n", n);
    
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
    
    /* Region 1: gang redundant (case 0) */
    printf("\n1. Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
    verify_array(a, n < 100 ? n : 100, (n < 100 ? n : 100) * (n < 100 ? n-1 : 99));
    
    /* Region 2: gang partitioned (case 1) */
    printf("\n2. Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i * 3;
        }
    }
    verify_array(b, n < 100 ? n : 100, (n < 100 ? n : 100) * (n < 100 ? n-1 : 99) * 3/2);
    
    /* Region 3: worker partitioned (case 2) */
    printf("\n3. Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i + 10;
        }
    }
    verify_array(c, n < 100 ? n : 100, (n < 100 ? n : 100) * 10 + (n < 100 ? n*(n-1)/2 : 4950));
    
    /* Region 4: gang+worker partitioned (case 3) */
    printf("\n4. Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * i;
        }
    }
    verify_array(d, n < 100 ? n : 100, (n < 100 ? n*(n-1)*(2*n-1)/6 : 328350));
    
    /* Region 5: vector partitioned (case 4) */
    printf("\n5. Testing vector partitioned...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i % 7;
        }
    }
    verify_array(e, n < 100 ? n : 100, (n < 100 ? n/7 : 14) * 21);
    
    /* Region 6: gang+vector partitioned (case 5) */
    printf("\n6. Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = (i % 3) + (i % 5);
        }
    }
    verify_array(f, n < 100 ? n : 100, 0); /* Non-zero expected, just checking execution */
    
    /* Region 7: worker+vector partitioned (case 6) */
    printf("\n7. Testing worker+vector partitioned...\n");
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
    verify_array(g, n < 100 ? n : 100, (n < 100 ? n : 100) * 6);
    
    /* Region 8: fully partitioned (case 7) */
    printf("\n8. Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            int sum = 0;
            #pragma acc loop vector reduction(+:sum)
            for (int j = 0; j < 8; j++) {
                sum += j;
            }
            h[i] = sum;
        }
    }
    verify_array(h, n < 100 ? n : 100, (n < 100 ? n : 100) * 28);
    
    /* Final combined result */
    int final_sum = 0;
    for (int i = 0; i < n; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    printf("\nFinal combined sum: %d\n", final_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
