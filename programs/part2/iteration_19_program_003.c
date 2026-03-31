/* test_omp_acc_partition_coverage.c
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_omp_acc_partition_coverage.c
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
    return (sum == expected_value);
}

int main(int argc, char *argv[]) {
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
    
    // Test 1: gang redundant (case 0)
    // Simple parallel region with num_gangs only
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
    
    // Test 2: gang partitioned (case 1)
    // Parallel loop with gang clause
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i * 2;
        }
    }
    
    // Test 3: worker partitioned (case 2)
    // Parallel region with num_workers
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 3;
        }
    }
    
    // Test 4: gang+worker partitioned (case 3)
    // Parallel region with both num_gangs and num_workers
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 4;
        }
    }
    
    // Test 5: vector partitioned (case 4)
    // Parallel loop with vector clause
    #pragma acc parallel loop vector vector_length(32) copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i * 5;
        }
    }
    
    // Test 6: gang+vector partitioned (case 5)
    // Parallel loop with gang and vector clauses
    #pragma acc parallel loop gang vector vector_length(32) copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i * 6;
        }
    }
    
    // Test 7: worker+vector partitioned (case 6)
    // Parallel region with num_workers and inner vector loop
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector vector_length(32)
            for (int j = 0; j < 10; j++) {
                g[i] += j;
            }
        }
    }
    
    // Test 8: fully partitioned (case 7)
    // Parallel region with all three levels
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector vector_length(32)
            for (int j = 0; j < 10; j++) {
                h[i] += j * 2;
            }
        }
    }
    
    // Additional test with reduction to ensure different code paths
    int reduction_result = 0;
    #pragma acc parallel num_gangs(4) copy(reduction_result)
    {
        #pragma acc loop gang reduction(+:reduction_result)
        for (int i = 0; i < n; i++) {
            reduction_result += 1;
        }
    }
    
    // Verify results
    int all_ok = 1;
    all_ok &= verify_array(a, n, n*(n-1)/2);
    all_ok &= verify_array(b, n, n*(n-1));
    all_ok &= verify_array(c, n, 3*n*(n-1)/2);
    all_ok &= verify_array(d, n, 2*n*(n-1));
    all_ok &= verify_array(e, n, 5*n*(n-1)/2);
    all_ok &= verify_array(f, n, 3*n*(n-1));
    all_ok &= verify_array(g, n, 45*n);  // sum(0..9) = 45
    all_ok &= verify_array(h, n, 90*n);  // 2*sum(0..9) = 90
    
    if (all_ok && reduction_result == n) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests failed!\n");
    }
    
    // Clean up
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    
    return 0;
}
