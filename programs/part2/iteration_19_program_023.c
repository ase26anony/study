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
           sum, expected_value * n / 2);
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Allocate and initialize arrays for different test cases */
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
    
    /* Test Case 0: gang redundant */
    printf("\n1. Testing gang redundant (case 0)...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang_id();
        #endif
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            a[i] = gang_id + i;
        }
    }
    verify_array(a, n, 1);
    
    /* Test Case 1: gang partitioned */
    printf("\n2. Testing gang partitioned (case 1)...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    verify_array(b, n, n-1);
    
    /* Test Case 2: worker partitioned */
    printf("\n3. Testing worker partitioned (case 2)...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = acc_worker_id();
        #endif
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            c[i] = worker_id + i * 3;
        }
    }
    verify_array(c, n, 1);
    
    /* Test Case 3: gang+worker partitioned */
    printf("\n4. Testing gang+worker partitioned (case 3)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang_id();
        worker_id = acc_worker_id();
        #endif
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            d[i] = gang_id * 10 + worker_id + i;
        }
    }
    verify_array(d, n, 5);
    
    /* Test Case 4: vector partitioned */
    printf("\n5. Testing vector partitioned (case 4)...\n");
    #pragma acc parallel loop vector vector_length(32) copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * 5;
    }
    verify_array(e, n, (n-1)*5/2);
    
    /* Test Case 5: gang+vector partitioned */
    printf("\n6. Testing gang+vector partitioned (case 5)...\n");
    #pragma acc parallel loop gang vector vector_length(32) copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i * 7;
    }
    verify_array(f, n, (n-1)*7/2);
    
    /* Test Case 6: worker+vector partitioned */
    printf("\n7. Testing worker+vector partitioned (case 6)...\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = acc_worker_id();
        #endif
        #pragma acc loop vector vector_length(32)
        for (int i = 0; i < n; i++) {
            g[i] = worker_id * 100 + i * 11;
        }
    }
    verify_array(g, n, 150);
    
    /* Test Case 7: fully partitioned */
    printf("\n8. Testing fully partitioned (case 7)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = acc_gang_id();
        worker_id = acc_worker_id();
        #endif
        #pragma acc loop vector vector_length(32)
        for (int i = 0; i < n; i++) {
            h[i] = gang_id * 1000 + worker_id * 100 + i * 13;
        }
    }
    verify_array(h, n, 550);
    
    /* Final combined result to prevent optimization */
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
