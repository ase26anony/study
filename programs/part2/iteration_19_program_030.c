/* test_partition_types.c - Cover all partition type cases in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024
#define VERIFY_VALUE 523776  /* Sum of 0..1023 */

/* Helper to initialize array */
static void init_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = 0;
    }
}

/* Helper to verify array */
static int verify_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing partition types with n = %d\n", n);
    
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
    
    /* Initialize all arrays */
    init_array(a, n);
    init_array(b, n);
    init_array(c, n);
    init_array(d, n);
    init_array(e, n);
    init_array(f, n);
    init_array(g, n);
    init_array(h, n);
    
    /* Test 1: gang redundant (case 0) */
    printf("Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    
    /* Test 2: gang partitioned (case 1) */
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i * 2;
        }
    }
    
    /* Test 3: worker partitioned (case 2) */
    printf("Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 3;
        }
    }
    
    /* Test 4: gang+worker partitioned (case 3) */
    printf("Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 4;
        }
    }
    
    /* Test 5: vector partitioned (case 4) */
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i * 5;
        }
    }
    
    /* Test 6: gang+vector partitioned (case 5) */
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i * 6;
        }
    }
    
    /* Test 7: worker+vector partitioned (case 6) */
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            g[i] = i * 7;
        }
    }
    
    /* Test 8: fully partitioned (case 7) */
    printf("Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = i * 8;
        }
    }
    
    /* Verify results on host to prevent dead code elimination */
    printf("Verifying results...\n");
    
    int sum_a = verify_array(a, n);
    int sum_b = verify_array(b, n);
    int sum_c = verify_array(c, n);
    int sum_d = verify_array(d, n);
    int sum_e = verify_array(e, n);
    int sum_f = verify_array(f, n);
    int sum_g = verify_array(g, n);
    int sum_h = verify_array(h, n);
    
    printf("Sums: a=%d, b=%d, c=%d, d=%d, e=%d, f=%d, g=%d, h=%d\n",
           sum_a, sum_b, sum_c, sum_d, sum_e, sum_f, sum_g, sum_h);
    
    /* Final reduction across all arrays */
    int total_sum = sum_a + sum_b + sum_c + sum_d + sum_e + sum_f + sum_g + sum_h;
    printf("Total sum: %d\n", total_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
