/* test_omp_oacc_partition_coverage.c */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_coverage test_omp_oacc_partition_coverage.c */
/* Or with: gcc -O2 -fopenmp -foffload=amd -o test_coverage test_omp_oacc_partition_coverage.c -lm */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef _OPENACC
#define PARALLEL_DIRECTIVE acc
#elif _OPENMP
#define PARALLEL_DIRECTIVE omp target teams distribute parallel for
#else
#error "Neither OpenACC nor OpenMP is enabled"
#endif

#define ARRAY_SIZE 1024

/* Function to verify array contents */
int verify_array(int *arr, int n, int expected_value) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] != expected_value) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: expected %d, got %d\n", 
                       i, expected_value, arr[i]);
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    int n = ARRAY_SIZE;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            n = ARRAY_SIZE;
        }
    }
    
    printf("Testing partition types with array size: %d\n", n);
    
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
    
    /* Test 1: gang redundant (case 0) */
    printf("\n1. Testing gang redundant...\n");
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
    total_errors += verify_array(a, n, 0);  /* All should be 0 due to gang redundant */
    
    /* Test 2: gang partitioned (case 1) */
    printf("\n2. Testing gang partitioned...\n");
    #pragma acc parallel loop gang copy(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            b[i] = i + 1;
        }
    }
    total_errors += verify_array(b, n, 0);  /* Check for errors */
    
    /* Test 3: worker partitioned (case 2) */
    printf("\n3. Testing worker partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(c[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            c[i] = i * 3;
        }
    }
    total_errors += verify_array(c, n, 0);
    
    /* Test 4: gang+worker partitioned (case 3) */
    printf("\n4. Testing gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(d[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            d[i] = i * 4;
        }
    }
    total_errors += verify_array(d, n, 0);
    
    /* Test 5: vector partitioned (case 4) */
    printf("\n5. Testing vector partitioned...\n");
    #pragma acc parallel loop vector copy(e[0:n])
    {
        for (int i = 0; i < n; i++) {
            e[i] = i * 5;
        }
    }
    total_errors += verify_array(e, n, 0);
    
    /* Test 6: gang+vector partitioned (case 5) */
    printf("\n6. Testing gang+vector partitioned...\n");
    #pragma acc parallel loop gang vector_length(32) copy(f[0:n])
    {
        for (int i = 0; i < n; i++) {
            f[i] = i * 6;
        }
    }
    total_errors += verify_array(f, n, 0);
    
    /* Test 7: worker+vector partitioned (case 6) */
    printf("\n7. Testing worker+vector partitioned...\n");
    #pragma acc parallel num_workers(4) copyout(g[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            g[i] = i * 7;
        }
    }
    total_errors += verify_array(g, n, 0);
    
    /* Test 8: fully partitioned (case 7) */
    printf("\n8. Testing fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) copyout(h[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            h[i] = i * 8;
        }
    }
    total_errors += verify_array(h, n, 0);
    
    /* Additional test with reduction to trigger more internal logic */
    printf("\n9. Testing with reduction...\n");
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) copyin(a[0:n])
    {
        for (int i = 0; i < n; i++) {
            sum += a[i];
        }
    }
    printf("  Reduction sum: %d\n", sum);
    
    /* Final verification with complex computation */
    printf("\n10. Final complex computation...\n");
    double *result = (double *)malloc(n * sizeof(double));
    #pragma acc parallel loop gang vector_length(64) copyout(result[0:n])
    {
        for (int i = 0; i < n; i++) {
            result[i] = sin(i * 0.1) * cos(i * 0.05);
        }
    }
    
    /* Compute final checksum */
    double checksum = 0.0;
    for (int i = 0; i < n; i++) {
        checksum += result[i];
    }
    printf("  Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(g); free(h);
    free(result);
    
    printf("\nTotal errors detected: %d\n", total_errors);
    printf("Test %s\n", total_errors == 0 ? "PASSED" : "FAILED");
    
    return total_errors > 0 ? 1 : 0;
}
