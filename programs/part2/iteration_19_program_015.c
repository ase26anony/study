/* test_partition_types.c - Cover all partition type cases in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024

/* Helper function to verify array contents */
static int verify_array(int *arr, int n, int expected_value) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] != expected_value) {
            errors++;
            if (errors < 5) {
                printf("  Error at arr[%d]: expected %d, got %d\n", 
                       i, expected_value, arr[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    
    /* Use command-line argument for problem size */
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
    memset(a, 0, n * sizeof(int));
    memset(b, 0, n * sizeof(int));
    memset(c, 0, n * sizeof(int));
    memset(d, 0, n * sizeof(int));
    memset(e, 0, n * sizeof(int));
    memset(f, 0, n * sizeof(int));
    memset(g, 0, n * sizeof(int));
    memset(h, 0, n * sizeof(int));
    
    int total_errors = 0;
    
    /* ============================================
       Test 1: "gang redundant" (case 0)
       OpenACC parallel region with num_gangs only
       ============================================ */
    printf("Test 1: gang redundant\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang(acc_gang_self);
        #endif
        #pragma acc loop independent
        for (int i = 0; i < n; i++) {
            a[i] = gang_id * 1000 + i;
        }
    }
    
    /* ============================================
       Test 2: "gang partitioned" (case 1)
       OpenACC parallel loop with gang clause
       ============================================ */
    printf("Test 2: gang partitioned\n");
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    
    /* ============================================
       Test 3: "worker partitioned" (case 2)
       OpenACC parallel region with num_workers
       ============================================ */
    printf("Test 3: worker partitioned\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = acc_worker(acc_worker_self);
        #endif
        #pragma acc loop independent
        for (int i = 0; i < n; i++) {
            c[i] = worker_id * 100 + i;
        }
    }
    
    /* ============================================
       Test 4: "gang+worker partitioned" (case 3)
       OpenACC parallel with both num_gangs and num_workers
       ============================================ */
    printf("Test 4: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang(acc_gang_self);
        worker_id = acc_worker(acc_worker_self);
        #endif
        #pragma acc loop independent
        for (int i = 0; i < n; i++) {
            d[i] = gang_id * 1000 + worker_id * 100 + i;
        }
    }
    
    /* ============================================
       Test 5: "vector partitioned" (case 4)
       OpenACC parallel loop with vector clause
       ============================================ */
    printf("Test 5: vector partitioned\n");
    #pragma acc parallel loop vector copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * 3;
    }
    
    /* ============================================
       Test 6: "gang+vector partitioned" (case 5)
       OpenACC parallel loop with gang and vector clauses
       ============================================ */
    printf("Test 6: gang+vector partitioned\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i * 4;
    }
    
    /* ============================================
       Test 7: "worker+vector partitioned" (case 6)
       OpenACC parallel with workers and inner vector loop
       ============================================ */
    printf("Test 7: worker+vector partitioned\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = acc_worker(acc_worker_self);
        #endif
        #pragma acc loop vector independent
        for (int i = 0; i < n; i++) {
            g[i] = worker_id * 100 + i * 5;
        }
    }
    
    /* ============================================
       Test 8: "fully partitioned" (case 7)
       OpenACC parallel with gangs, workers, and vector
       ============================================ */
    printf("Test 8: fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang(acc_gang_self);
        worker_id = acc_worker(acc_worker_self);
        #endif
        #pragma acc loop vector independent
        for (int i = 0; i < n; i++) {
            h[i] = gang_id * 1000 + worker_id * 100 + i * 6;
        }
    }
    
    /* ============================================
       Verification and cleanup
       ============================================ */
    
    /* Perform a reduction on host to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    
    printf("Final sum: %lld\n", sum);
    printf("All partition type tests completed\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(g);
    free(h);
    
    return 0;
}
