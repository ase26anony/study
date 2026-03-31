/* test_partition_types.c - Cover all OpenACC partition type cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N_DEFAULT 1024
#define VERIFY_VALUE 523776  /* Sum of 0..1023 */

void test_gang_redundant(int n, int *a) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
}

void test_gang_partitioned(int n, int *a, int *b) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] * 2;
    }
}

void test_worker_partitioned(int n, int *a, int *b) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + 1;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 3;
        }
    }
}

void test_vector_partitioned(int n, int *a, int *b) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] - 1;
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] / 2;
    }
}

void test_worker_vector_partitioned(int n, int *a, int *b) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            b[i] = a[i] % 100;
        }
    }
}

void test_fully_partitioned(int n, int *a, int *b) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * a[i];
        }
    }
}

int verify_result(int n, int *arr, const char *test_name) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("%s: sum = %d\n", test_name, sum);
    return sum;
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            fprintf(stderr, "Error: n must be positive\n");
            return 1;
        }
    }
    
    printf("Testing with n = %d\n", n);
    
    /* Allocate arrays */
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
    
    /* Test all partition types */
    test_gang_redundant(n, a);
    int sum_a = verify_result(n, a, "gang_redundant");
    
    test_gang_partitioned(n, a, b);
    int sum_b = verify_result(n, b, "gang_partitioned");
    
    test_worker_partitioned(n, a, c);
    int sum_c = verify_result(n, c, "worker_partitioned");
    
    test_gang_worker_partitioned(n, a, d);
    int sum_d = verify_result(n, d, "gang_worker_partitioned");
    
    test_vector_partitioned(n, a, e);
    int sum_e = verify_result(n, e, "vector_partitioned");
    
    test_gang_vector_partitioned(n, a, f);
    int sum_f = verify_result(n, f, "gang_vector_partitioned");
    
    test_worker_vector_partitioned(n, a, g);
    int sum_g = verify_result(n, g, "worker_vector_partitioned");
    
    test_fully_partitioned(n, a, h);
    int sum_h = verify_result(n, h, "fully_partitioned");
    
    /* Final verification to prevent optimization */
    int final_sum = sum_a + sum_b + sum_c + sum_d + sum_e + sum_f + sum_g + sum_h;
    printf("Final combined sum: %d\n", final_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
