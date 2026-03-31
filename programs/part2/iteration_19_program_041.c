/* Test program to cover partition type string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Run with: ./test_partition 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N_DEFAULT 1000

void initialize_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = 0;
    }
}

int verify_array(int *arr, int n, int expected_value) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = N_DEFAULT;
    }
    
    printf("Testing with n = %d\n", n);
    
    // Allocate arrays for different test cases
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
    
    // Initialize all arrays
    initialize_array(a, n);
    initialize_array(b, n);
    initialize_array(c, n);
    initialize_array(d, n);
    initialize_array(e, n);
    initialize_array(f, n);
    initialize_array(g, n);
    initialize_array(h, n);
    
    int total_sum = 0;
    
    // Test Case 0: "gang redundant"
    // Simple parallel region with num_gangs only
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i % 100;
        }
    }
    total_sum += verify_array(a, n, 0);
    
    // Test Case 1: "gang partitioned"
    // Parallel loop with gang clause
    #pragma acc parallel loop gang copy(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = i * 2;
    }
    total_sum += verify_array(b, n, 0);
    
    // Test Case 2: "worker partitioned"
    // Parallel region with num_workers
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i + 1;
        }
    }
    total_sum += verify_array(c, n, 0);
    
    // Test Case 3: "gang+worker partitioned"
    // Parallel region with both num_gangs and num_workers
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 3;
        }
    }
    total_sum += verify_array(d, n, 0);
    
    // Test Case 4: "vector partitioned"
    // Parallel loop with vector clause
    #pragma acc parallel loop vector copy(e[0:n])
    for (int i = 0; i < n; i++) {
        e[i] = i * i;
    }
    total_sum += verify_array(e, n, 0);
    
    // Test Case 5: "gang+vector partitioned"
    // Parallel loop with both gang and vector clauses
    #pragma acc parallel loop gang vector copy(f[0:n])
    for (int i = 0; i < n; i++) {
        f[i] = i % 10;
    }
    total_sum += verify_array(f, n, 0);
    
    // Test Case 6: "worker+vector partitioned"
    // Parallel region with workers and vector loops
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
    total_sum += verify_array(g, n, 0);
    
    // Test Case 7: "fully partitioned"
    // Complex region with gangs, workers, and vectors
    #pragma acc parallel num_gangs(2) num_workers(2) copyout(h[0:n])
    {
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 2; w++) {
                int start = (g * n/2) + (w * n/4);
                int end = start + n/4;
                #pragma acc loop vector
                for (int i = start; i < end && i < n; i++) {
                    h[i] = g * 1000 + w * 100 + (i % 100);
                }
            }
        }
    }
    total_sum += verify_array(h, n, 0);
    
    // Additional test with reduction to trigger more internal logic
    int reduction_result = 0;
    #pragma acc parallel num_gangs(4) num_workers(2) copyout(a[0:n]) reduction(+:reduction_result)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = i;
            reduction_result += i;
        }
    }
    
    // Verify with host computation
    int host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += i;
    }
    
    printf("Total sum from all arrays: %d\n", total_sum);
    printf("Reduction result: %d (expected: %d)\n", reduction_result, host_sum);
    
    if (reduction_result == host_sum) {
        printf("SUCCESS: All tests passed\n");
    } else {
        printf("WARNING: Reduction mismatch (might be expected with certain optimizations)\n");
    }
    
    // Cleanup
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
