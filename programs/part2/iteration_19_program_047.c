/* test_omp_oacc_partition_coverage.c
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test test_omp_oacc_partition_coverage.c
 * Or with: gcc -O2 -fopenmp -foffload=amd -o test test_omp_oacc_partition_coverage.c
 * Run with: ./test 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#define TARGET_PRAGMA acc
#elif _OPENMP
#define TARGET_PRAGMA omp target teams distribute parallel for
#else
#error "Compile with -fopenacc or -fopenmp"
#endif

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Array size must be positive\n");
        return 1;
    }
    
    // Allocate and initialize arrays
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *e = (int*)malloc(n * sizeof(int));
    int *f = (int*)malloc(n * sizeof(int));
    int *g = (int*)malloc(n * sizeof(int));
    int *h = (int*)malloc(n * sizeof(int));
    
    assert(a && b && c && d && e && f && g && h);
    
    // Initialize arrays with different patterns
    for (int i = 0; i < n; i++) {
        a[i] = 0;
        b[i] = i;
        c[i] = i % 10;
        d[i] = i * 2;
        e[i] = i % 5;
        f[i] = i % 7;
        g[i] = i % 3;
        h[i] = i;
    }
    
    int sum = 0;
    
    // =================================================================
    // Region 1: gang redundant (case 0)
    // Simple parallel region with num_gangs only
    // =================================================================
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
    
    // =================================================================
    // Region 2: gang partitioned (case 1)
    // Parallel loop with gang clause
    // =================================================================
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = b[i] + 1;
        }
    }
    
    // =================================================================
    // Region 3: worker partitioned (case 2)
    // Parallel region with num_workers clause
    // =================================================================
    #pragma acc parallel num_workers(8) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = c[i] * 3;
        }
    }
    
    // =================================================================
    // Region 4: gang+worker partitioned (case 3)
    // Parallel region with both num_gangs and num_workers
    // =================================================================
    #pragma acc parallel num_gangs(4) num_workers(4) copy(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = d[i] / 2;
        }
    }
    
    // =================================================================
    // Region 5: vector partitioned (case 4)
    // Parallel loop with vector clause
    // =================================================================
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = e[i] + i;
        }
    }
    
    // =================================================================
    // Region 6: gang+vector partitioned (case 5)
    // Parallel loop with both gang and vector clauses
    // =================================================================
    #pragma acc parallel loop gang vector copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = f[i] * 2 - 1;
        }
    }
    
    // =================================================================
    // Region 7: worker+vector partitioned (case 6)
    // Parallel region with num_workers and inner vector loop
    // =================================================================
    #pragma acc parallel num_workers(4) copy(g[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                g[i] = g[i] + j;
            }
        }
    }
    
    // =================================================================
    // Region 8: fully partitioned (case 7)
    // Parallel region with gangs, workers, and vector length
    // =================================================================
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(32) copy(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = h[i] % 10 + 100;
        }
    }
    
    // =================================================================
    // Additional OpenMP target regions for extra coverage
    // =================================================================
    #ifdef _OPENMP
    int *omp_arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) omp_arr[i] = i;
    
    // OpenMP target teams with different distributions
    #pragma omp target teams distribute parallel for map(tofrom:omp_arr[0:n])
    for (int i = 0; i < n; i++) {
        omp_arr[i] = omp_arr[i] * 2;
    }
    
    #pragma omp target teams distribute parallel for simd map(tofrom:omp_arr[0:n])
    for (int i = 0; i < n; i++) {
        omp_arr[i] = omp_arr[i] + 1;
    }
    
    free(omp_arr);
    #endif
    
    // =================================================================
    // Verification and result computation
    // =================================================================
    
    // Compute sum of all arrays to prevent dead code elimination
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + e[i] + f[i] + g[i] + h[i];
    }
    
    // Also perform a reduction on device to trigger more code paths
    int reduction_sum = 0;
    #pragma acc parallel loop reduction(+:reduction_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        reduction_sum += a[i];
    }
    
    printf("Total sum: %d, Reduction sum: %d\n", sum, reduction_sum);
    
    // Cleanup
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
