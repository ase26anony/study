/* test_omp_oacc_partition_coverage.c
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_omp_oacc_partition_coverage test_omp_oacc_partition_coverage.c
 * Run with: ./test_omp_oacc_partition_coverage 10000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N_DEFAULT 10000

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
    
    assert(a && b && c && d && e && f && g && h);
    
    // Initialize arrays to zero
    memset(a, 0, n * sizeof(int));
    memset(b, 0, n * sizeof(int));
    memset(c, 0, n * sizeof(int));
    memset(d, 0, n * sizeof(int));
    memset(e, 0, n * sizeof(int));
    memset(f, 0, n * sizeof(int));
    memset(g, 0, n * sizeof(int));
    memset(h, 0, n * sizeof(int));
    
    int total_sum = 0;
    
    // 1. Case 0: "gang redundant"
    // Simple reduction with gang-level redundancy
    printf("\n1. Testing gang redundant (case 0)...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i % 100;
        }
    }
    total_sum += n * 49; // Expected sum for i%100 pattern
    
    // 2. Case 1: "gang partitioned"
    printf("\n2. Testing gang partitioned (case 1)...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = (i * 2) % 100;
        }
    }
    total_sum += n * 99; // Expected sum for (i*2)%100 pattern
    
    // 3. Case 2: "worker partitioned"
    printf("\n3. Testing worker partitioned (case 2)...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = (i + 1) % 100;
        }
    }
    total_sum += n * 50; // Expected sum for (i+1)%100 pattern
    
    // 4. Case 3: "gang+worker partitioned"
    printf("\n4. Testing gang+worker partitioned (case 3)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = (i * 3) % 100;
        }
    }
    total_sum += n * 147; // Expected sum for (i*3)%100 pattern
    
    // 5. Case 4: "vector partitioned"
    printf("\n5. Testing vector partitioned (case 4)...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = (i * 4) % 100;
        }
    }
    total_sum += n * 196; // Expected sum for (i*4)%100 pattern
    
    // 6. Case 5: "gang+vector partitioned"
    printf("\n6. Testing gang+vector partitioned (case 5)...\n");
    #pragma acc parallel loop gang vector_length(32) copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = (i * 5) % 100;
        }
    }
    total_sum += n * 245; // Expected sum for (i*5)%100 pattern
    
    // 7. Case 6: "worker+vector partitioned"
    printf("\n7. Testing worker+vector partitioned (case 6)...\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                g[i] += (i + j) % 100;
            }
        }
    }
    // Complex pattern, we'll compute separately
    int g_sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            g_sum += (i + j) % 100;
        }
    }
    total_sum += g_sum;
    
    // 8. Case 7: "fully partitioned"
    printf("\n8. Testing fully partitioned (case 7)...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < 8; j++) {
                h[i] += (i * j) % 100;
            }
        }
    }
    // Complex pattern, we'll compute separately
    int h_sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 8; j++) {
            h_sum += (i * j) % 100;
        }
    }
    total_sum += h_sum;
    
    // Combine all results into array a for final verification
    printf("\nCombining results...\n");
    #pragma acc parallel loop copy(a[0:n], b[0:n], c[0:n], d[0:n], e[0:n], f[0:n], g[0:n], h[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    
    // Final verification on host
    printf("\nFinal verification:\n");
    verify_array(a, n, total_sum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(g);
    free(h);
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
