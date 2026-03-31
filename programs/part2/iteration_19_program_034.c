/* test_omp_acc_partition_coverage.c
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_coverage test_omp_acc_partition_coverage.c
 * Run with: ./test_coverage 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N_DEFAULT 1000

void verify_array(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("Array sum: %d (expected: %d)\n", sum, expected_sum);
    assert(sum == expected_sum);
}

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing with n = %d\n", n);
    
    // Allocate and initialize arrays
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *d = (int *)malloc(n * sizeof(int));
    int *e = (int *)malloc(n * sizeof(int));
    int *f = (int *)malloc(n * sizeof(int));
    int *g = (int *)malloc(n * sizeof(int));
    int *h = (int *)malloc(n * sizeof(int));
    
    // Initialize arrays to zero
    memset(a, 0, n * sizeof(int));
    memset(b, 0, n * sizeof(int));
    memset(c, 0, n * sizeof(int));
    memset(d, 0, n * sizeof(int));
    memset(e, 0, n * sizeof(int));
    memset(f, 0, n * sizeof(int));
    memset(g, 0, n * sizeof(int));
    memset(h, 0, n * sizeof(int));
    
    // =================================================================
    // Test Case 1: "gang redundant" (case 0)
    // Simple parallel region with num_gangs only
    // =================================================================
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    printf("Case 0 (gang redundant) completed\n");
    
    // =================================================================
    // Test Case 2: "gang partitioned" (case 1)
    // Parallel loop with gang clause
    // =================================================================
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    printf("Case 1 (gang partitioned) completed\n");
    
    // =================================================================
    // Test Case 3: "worker partitioned" (case 2)
    // Parallel region with num_workers clause
    // =================================================================
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 3;
        }
    }
    printf("Case 2 (worker partitioned) completed\n");
    
    // =================================================================
    // Test Case 4: "gang+worker partitioned" (case 3)
    // Parallel region with both num_gangs and num_workers
    // =================================================================
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 4;
        }
    }
    printf("Case 3 (gang+worker partitioned) completed\n");
    
    // =================================================================
    // Test Case 5: "vector partitioned" (case 4)
    // Parallel loop with vector clause
    // =================================================================
    #pragma acc parallel loop vector copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * 5;
    }
    printf("Case 4 (vector partitioned) completed\n");
    
    // =================================================================
    // Test Case 6: "gang+vector partitioned" (case 5)
    // Parallel loop with gang and vector clauses
    // =================================================================
    #pragma acc parallel loop gang vector copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i * 6;
    }
    printf("Case 5 (gang+vector partitioned) completed\n");
    
    // =================================================================
    // Test Case 7: "worker+vector partitioned" (case 6)
    // Parallel region with num_workers and inner vector loop
    // =================================================================
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                g[i] += i * 7 + j;
            }
        }
    }
    printf("Case 6 (worker+vector partitioned) completed\n");
    
    // =================================================================
    // Test Case 8: "fully partitioned" (case 7)
    // Parallel region with gangs, workers, and vector length
    // =================================================================
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(32) copyout(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = i * 8;
        }
    }
    printf("Case 7 (fully partitioned) completed\n");
    
    // =================================================================
    // Verification and cleanup
    // =================================================================
    
    // Verify each array
    printf("\nVerifying results:\n");
    verify_array(a, n, n * (n - 1) / 2);           // Sum of 0..n-1
    verify_array(b, n, n * (n - 1));               // Sum of 2*i
    verify_array(c, n, 3 * n * (n - 1) / 2);       // Sum of 3*i
    verify_array(d, n, 2 * n * (n - 1));           // Sum of 4*i
    verify_array(e, n, 5 * n * (n - 1) / 2);       // Sum of 5*i
    verify_array(f, n, 3 * n * (n - 1));           // Sum of 6*i
    verify_array(g, n, n * (4 * n * 7 + 6) / 2);   // Sum of (28*i + 6)
    verify_array(h, n, 4 * n * (n - 1));           // Sum of 8*i
    
    // Additional test with reduction to trigger more internal logic
    int total_sum = 0;
    #pragma acc parallel loop reduction(+:total_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        total_sum += a[i];
    }
    printf("Reduction test sum: %d\n", total_sum);
    
    // Free memory
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(g);
    free(h);
    
    printf("\nAll test cases completed successfully!\n");
    return 0;
}
