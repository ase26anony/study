/* test_partition_types.c - Coverage for omp-oacc-neuter-broadcast.cc partition type strings */
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
    /* Case 0: gang redundant - simple parallel region with num_gangs */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
}

void test_gang_partitioned(int n, int *a) {
    /* Case 1: gang partitioned - parallel loop with gang clause */
    #pragma acc parallel loop gang copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] += i * 2;
    }
}

void test_worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned - parallel region with num_workers */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] -= i;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a) {
    /* Case 3: gang+worker partitioned - both num_gangs and num_workers */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] += 1;
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    /* Case 4: vector partitioned - parallel loop with vector clause */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] *= 2;
    }
}

void test_gang_vector_partitioned(int n, int *a) {
    /* Case 5: gang+vector partitioned - both gang and vector clauses */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] /= 2;
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned - worker region with vector loop */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i += 4) {
            #pragma acc loop vector vector_length(32)
            for (int j = 0; j < 4 && (i + j) < n; j++) {
                a[i + j] += j;
            }
        }
    }
}

void test_fully_partitioned(int n, int *a) {
    /* Case 7: fully partitioned - gang, worker, and vector all specified */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            int start = g * (n / 2);
            int end = (g + 1) * (n / 2);
            #pragma acc loop worker
            for (int i = start; i < end; i += 4) {
                #pragma acc loop vector vector_length(32)
                for (int j = 0; j < 4 && (i + j) < end; j++) {
                    a[i + j] = (i + j) * 3;
                }
            }
        }
    }
}

int verify_result(int n, int *a) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            fprintf(stderr, "Error: n must be positive\n");
            return 1;
        }
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = 0;
        b[i] = i;
    }
    
    /* Test each partition type with different constructs */
    
    /* OpenACC tests - these should trigger the partition type logic */
    printf("Testing gang redundant...\n");
    test_gang_redundant(n, a);
    
    printf("Testing gang partitioned...\n");
    test_gang_partitioned(n, a);
    
    printf("Testing worker partitioned...\n");
    test_worker_partitioned(n, a);
    
    printf("Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, a);
    
    printf("Testing vector partitioned...\n");
    test_vector_partitioned(n, a);
    
    printf("Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, a);
    
    printf("Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, a);
    
    printf("Testing fully partitioned...\n");
    test_fully_partitioned(n, a);
    
    /* Also test with OpenMP target offload for additional coverage */
    #ifdef _OPENMP
    printf("Testing with OpenMP target offload...\n");
    #pragma omp target teams distribute parallel for map(tofrom: b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + b[i];
    }
    #endif
    
    /* Verify results to prevent optimization */
    int final_sum = verify_result(n, a);
    printf("Final array sum: %d\n", final_sum);
    
    /* Additional verification with reduction */
    int reduction_sum = 0;
    #pragma acc parallel loop reduction(+:reduction_sum) copyin(a[0:n])
    for (int i = 0; i < n; i++) {
        reduction_sum += a[i];
    }
    printf("Reduction sum: %d\n", reduction_sum);
    
    /* Clean up */
    free(a);
    free(b);
    
    printf("All partition type tests completed\n");
    return 0;
}
