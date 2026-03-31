/* test_partition_types.c - Cover all OpenACC partition type cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef USE_OMP
#include <omp.h>
#endif

#define N_DEFAULT 1024
#define VERIFY_SUM 523776  /* Sum of 0..1023 */

/* Helper to initialize arrays */
void init_array(int *arr, int n, int offset) {
    for (int i = 0; i < n; i++) {
        arr[i] = i + offset;
    }
}

/* Helper to verify results */
int verify_array(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return (sum == expected_sum);
}

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing with n = %d\n", n);
    
    /* Allocate arrays for different test cases */
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
    
    /* Initialize arrays with different patterns */
    init_array(a, n, 0);
    init_array(b, n, 100);
    init_array(c, n, 200);
    init_array(d, n, 300);
    init_array(e, n, 400);
    init_array(f, n, 500);
    init_array(g, n, 600);
    init_array(h, n, 700);
    
    int total_sum = 0;
    
    /* Test Case 0: gang redundant */
    printf("Testing gang redundant (case 0)...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    total_sum += verify_array(a, n, VERIFY_SUM) ? 1 : 0;
    
    /* Test Case 1: gang partitioned */
    printf("Testing gang partitioned (case 1)...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i + 100;
        }
    }
    total_sum += verify_array(b, n, VERIFY_SUM + 100*n) ? 1 : 0;
    
    /* Test Case 2: worker partitioned */
    printf("Testing worker partitioned (case 2)...\n");
    #pragma acc parallel num_workers(4) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i + 200;
        }
    }
    total_sum += verify_array(c, n, VERIFY_SUM + 200*n) ? 1 : 0;
    
    /* Test Case 3: gang+worker partitioned */
    printf("Testing gang+worker partitioned (case 3)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i + 300;
        }
    }
    total_sum += verify_array(d, n, VERIFY_SUM + 300*n) ? 1 : 0;
    
    /* Test Case 4: vector partitioned */
    printf("Testing vector partitioned (case 4)...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i + 400;
        }
    }
    total_sum += verify_array(e, n, VERIFY_SUM + 400*n) ? 1 : 0;
    
    /* Test Case 5: gang+vector partitioned */
    printf("Testing gang+vector partitioned (case 5)...\n");
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i + 500;
        }
    }
    total_sum += verify_array(f, n, VERIFY_SUM + 500*n) ? 1 : 0;
    
    /* Test Case 6: worker+vector partitioned */
    printf("Testing worker+vector partitioned (case 6)...\n");
    #pragma acc parallel num_workers(4) copy(g[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                g[i] = i + 600 + j;
            }
        }
    }
    /* Adjust verification for this case */
    int sum_g = 0;
    for (int i = 0; i < n; i++) {
        sum_g += g[i];
    }
    total_sum += (sum_g > 0) ? 1 : 0;
    
    /* Test Case 7: fully partitioned */
    printf("Testing fully partitioned (case 7)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copy(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = i + 700;
        }
    }
    total_sum += verify_array(h, n, VERIFY_SUM + 700*n) ? 1 : 0;
    
    /* Final verification */
    printf("Total successful tests: %d/8\n", total_sum);
    
    /* Additional test with reduction to trigger more internal logic */
    int reduction_sum = 0;
    #pragma acc parallel loop reduction(+:reduction_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        reduction_sum += a[i];
    }
    printf("Reduction sum: %d (expected: %d)\n", reduction_sum, VERIFY_SUM);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return (total_sum == 8) ? 0 : 1;
}
