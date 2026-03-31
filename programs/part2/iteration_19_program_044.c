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
static void init_array(int *arr, int n, int start_val) {
    for (int i = 0; i < n; i++) {
        arr[i] = start_val + i;
    }
}

/* Helper to verify array sum */
static int verify_array_sum(int *arr, int n) {
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
    
    /* Allocate arrays for different test cases */
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
    
    /* Initialize arrays with different patterns */
    init_array(a, n, 0);
    init_array(b, n, 1000);
    init_array(c, n, 2000);
    init_array(d, n, 3000);
    init_array(e, n, 4000);
    init_array(f, n, 5000);
    init_array(g, n, 6000);
    init_array(h, n, 7000);
    
    int total_sum = 0;
    
    /* Test Case 1: gang redundant (case 0) */
    printf("Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;  /* Simple assignment */
        }
    }
    total_sum += verify_array_sum(a, n);
    
    /* Test Case 2: gang partitioned (case 1) */
    printf("Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i * 2;
        }
    }
    total_sum += verify_array_sum(b, n);
    
    /* Test Case 3: worker partitioned (case 2) */
    printf("Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i + 1;
        }
    }
    total_sum += verify_array_sum(c, n);
    
    /* Test Case 4: gang+worker partitioned (case 3) */
    printf("Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 3;
        }
    }
    total_sum += verify_array_sum(d, n);
    
    /* Test Case 5: vector partitioned (case 4) */
    printf("Testing vector partitioned...\n");
    #pragma acc parallel loop vector vector_length(32) copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i * 4;
        }
    }
    total_sum += verify_array_sum(e, n);
    
    /* Test Case 6: gang+vector partitioned (case 5) */
    printf("Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector vector_length(32) copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i * 5;
        }
    }
    total_sum += verify_array_sum(f, n);
    
    /* Test Case 7: worker+vector partitioned (case 6) */
    printf("Testing worker+vector partitioned...\n");
    #pragma acc parallel num_workers(4) copy(g[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            g[i] = i * 6;
        }
    }
    total_sum += verify_array_sum(g, n);
    
    /* Test Case 8: fully partitioned (case 7) */
    printf("Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(h[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            h[i] = i * 7;
        }
    }
    total_sum += verify_array_sum(h, n);
    
    /* Final verification to prevent dead code elimination */
    printf("Total sum across all arrays: %d\n", total_sum);
    
    /* Additional test with reduction to trigger more internal logic */
    int reduction_sum = 0;
    #pragma acc parallel loop reduction(+:reduction_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        reduction_sum += a[i];
    }
    printf("Reduction sum: %d (expected: %d)\n", reduction_sum, VERIFY_VALUE);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
