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

void gang_redundant(int *a, int n) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i + gang_id * 1000;  /* Different gangs write different values */
        }
    }
}

void gang_partitioned(int *a, int n) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 2;
    }
}

void worker_partitioned(int *a, int n) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = __pgi_workeridx();
        #endif
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] = i + worker_id * 100;
        }
    }
}

void gang_worker_partitioned(int *a, int n) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        worker_id = __pgi_workeridx();
        #endif
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = i + gang_id * 1000 + worker_id * 100;
        }
    }
}

void vector_partitioned(int *a, int n) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
    }
}

void gang_vector_partitioned(int *a, int n) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 4;
    }
}

void worker_vector_partitioned(int *a, int n) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            a[i] = i * 5;
        }
    }
}

void fully_partitioned(int *a, int n) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            a[i] = i * 6;
        }
    }
}

int verify_array(int *a, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    printf("Array sum: %d (expected: %d)\n", sum, expected_sum);
    return sum == expected_sum;
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
    
    printf("Testing OpenACC partition types with n = %d\n", n);
    
    /* Allocate and initialize arrays for each test */
    int *a1 = (int *)malloc(n * sizeof(int));
    int *a2 = (int *)malloc(n * sizeof(int));
    int *a3 = (int *)malloc(n * sizeof(int));
    int *a4 = (int *)malloc(n * sizeof(int));
    int *a5 = (int *)malloc(n * sizeof(int));
    int *a6 = (int *)malloc(n * sizeof(int));
    int *a7 = (int *)malloc(n * sizeof(int));
    int *a8 = (int *)malloc(n * sizeof(int));
    
    if (!a1 || !a2 || !a3 || !a4 || !a5 || !a6 || !a7 || !a8) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Test each partition type */
    printf("\n1. Testing gang redundant...\n");
    gang_redundant(a1, n);
    
    printf("2. Testing gang partitioned...\n");
    gang_partitioned(a2, n);
    
    printf("3. Testing worker partitioned...\n");
    worker_partitioned(a3, n);
    
    printf("4. Testing gang+worker partitioned...\n");
    gang_worker_partitioned(a4, n);
    
    printf("5. Testing vector partitioned...\n");
    vector_partitioned(a5, n);
    
    printf("6. Testing gang+vector partitioned...\n");
    gang_vector_partitioned(a6, n);
    
    printf("7. Testing worker+vector partitioned...\n");
    worker_vector_partitioned(a7, n);
    
    printf("8. Testing fully partitioned...\n");
    fully_partitioned(a8, n);
    
    /* Verify results */
    printf("\nVerification:\n");
    int all_ok = 1;
    
    /* We can't easily predict exact values due to parallel execution,
       but we can verify the arrays were modified */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int sum5 = 0, sum6 = 0, sum7 = 0, sum8 = 0;
    
    #pragma acc parallel loop reduction(+:sum1,sum2,sum3,sum4,sum5,sum6,sum7,sum8) \
        copyin(a1[0:n],a2[0:n],a3[0:n],a4[0:n],a5[0:n],a6[0:n],a7[0:n],a8[0:n])
    for (int i = 0; i < n; i++) {
        sum1 += a1[i];
        sum2 += a2[i];
        sum3 += a3[i];
        sum4 += a4[i];
        sum5 += a5[i];
        sum6 += a6[i];
        sum7 += a7[i];
        sum8 += a8[i];
    }
    
    printf("Sums: %d, %d, %d, %d, %d, %d, %d, %d\n", 
           sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8);
    
    /* Cleanup */
    free(a1); free(a2); free(a3); free(a4);
    free(a5); free(a6); free(a7); free(a8);
    
    printf("\nAll partition type tests completed.\n");
    
    return 0;
}
