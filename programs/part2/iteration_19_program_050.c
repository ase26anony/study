/* test_partition_types.c - Coverage test for omp-oacc-neuter-broadcast.cc partition types */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _OPENACC
#define USE_OPENACC 1
#else
#define USE_OPENACC 0
#endif

#ifdef _OPENMP
#define USE_OPENMP 1
#else
#define USE_OPENMP 0
#endif

/* Helper to ensure computations aren't optimized away */
static int volatile prevent_opt = 0;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 0) n = 1000;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
        c[i] = 0;
        d[i] = 0;
    }
    
    int sum = 0;
    
    /* Region 1: gang redundant (case 0) */
    /* Simple reduction across gangs */
    #pragma acc parallel num_gangs(4) copyin(a[0:n]) copyout(c[0:n])
    {
        int gang_id = 0;
        #if USE_OPENACC
        gang_id = __pgi_gangidx();
        #endif
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + gang_id;  /* Gang-redundant computation */
        }
    }
    
    /* Region 2: gang partitioned (case 1) */
    /* Each gang works on its own partition */
    #pragma acc parallel loop gang copy(a[0:n], b[0:n], d[0:n])
    for (int i = 0; i < n; i++) {
        d[i] = a[i] * b[i];  /* Gang-partitioned computation */
    }
    
    /* Region 3: worker partitioned (case 2) */
    /* Workers within gangs do partitioned work */
    #pragma acc parallel num_workers(4) copyin(a[0:n], b[0:n]) copy(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] += b[i];  /* Worker-partitioned update */
        }
    }
    
    /* Region 4: gang+worker partitioned (case 3) */
    /* Both gangs and workers partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n], d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = a[i] * 2;  /* Gang+worker partitioned */
        }
    }
    
    /* Region 5: vector partitioned (case 4) */
    /* Vector lanes work independently */
    #pragma acc parallel loop vector copy(a[0:n], b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + 1;  /* Vector-partitioned */
    }
    
    /* Region 6: gang+vector partitioned (case 5) */
    /* Gangs and vectors both partitioned */
    #pragma acc parallel loop gang vector copy(a[0:n], c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * 3;  /* Gang+vector partitioned */
    }
    
    /* Region 7: worker+vector partitioned (case 6) */
    /* Workers and vectors partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n], d[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            d[i] = a[i] + 5;  /* Worker+vector partitioned */
        }
    }
    
    /* Region 8: fully partitioned (case 7) */
    /* Gangs, workers, and vectors all partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n], b[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * a[i];  /* Fully partitioned */
        }
    }
    
    /* Also test OpenMP target offload for additional coverage */
    #if USE_OPENMP
    /* OpenMP version covering similar partition types */
    int *omp_arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) omp_arr[i] = i;
    
    /* OpenMP target with teams distribute (gang-like) */
    #pragma omp target teams distribute parallel for map(tofrom:omp_arr[0:n])
    for (int i = 0; i < n; i++) {
        omp_arr[i] += 100;
    }
    
    /* OpenMP target with nested parallelism */
    #pragma omp target teams distribute parallel for simd map(tofrom:omp_arr[0:n])
    for (int i = 0; i < n; i++) {
        omp_arr[i] *= 2;
    }
    
    free(omp_arr);
    #endif
    
    /* Final verification to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
        prevent_opt = sum;  /* Volatile write to prevent optimization */
    }
    
    printf("Final sum: %d (prevent_opt=%d)\n", sum, prevent_opt);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
