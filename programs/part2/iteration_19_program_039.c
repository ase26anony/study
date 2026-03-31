/* test_omp_oacc_partition_types.c */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_omp_oacc_partition_types.c */
/* Or with: gcc -O2 -fopenacc -o test_partition test_omp_oacc_partition_types.c */
/* Run with: ./test_partition 1000 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N_DEFAULT 1024

void gang_redundant(int n, int *a) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
    }
}

void gang_partitioned(int n, int *a, int *b) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + 1;
    }
}

void worker_partitioned(int n, int *a, int *b) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 2;
        }
    }
}

void gang_worker_partitioned(int n, int *a, int *b) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            b[i] = a[i] - 1;
        }
    }
}

void vector_partitioned(int n, int *a, int *b) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] / 2;
    }
}

void gang_vector_partitioned(int n, int *a, int *b) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + 100;
    }
}

void worker_vector_partitioned(int n, int *a, int *b) {
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop worker vector vector_length(16)
        for (int i = 0; i < n; i++) {
            b[i] = a[i] * 3;
        }
    }
}

void fully_partitioned(int n, int *a, int *b) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyout(b[0:n])
    {
        #pragma acc loop gang worker vector vector_length(8)
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + 500;
        }
    }
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            fprintf(stderr, "Error: n must be > 0\n");
            return 1;
        }
    }
    
    printf("Testing partition types with n = %d\n", n);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* Test each partition type sequentially */
    printf("1. Testing gang redundant...\n");
    gang_redundant(n, b);
    
    printf("2. Testing gang partitioned...\n");
    gang_partitioned(n, b, c);
    
    printf("3. Testing worker partitioned...\n");
    worker_partitioned(n, c, b);
    
    printf("4. Testing gang+worker partitioned...\n");
    gang_worker_partitioned(n, b, c);
    
    printf("5. Testing vector partitioned...\n");
    vector_partitioned(n, c, b);
    
    printf("6. Testing gang+vector partitioned...\n");
    gang_vector_partitioned(n, b, c);
    
    printf("7. Testing worker+vector partitioned...\n");
    worker_vector_partitioned(n, c, b);
    
    printf("8. Testing fully partitioned...\n");
    fully_partitioned(n, b, c);
    
    /* Final verification to prevent dead code elimination */
    long long sum = 0;
    #pragma acc serial copyin(c[0:n]) copy(sum)
    {
        for (int i = 0; i < n; i++) {
            sum += c[i];
        }
    }
    
    /* Also compute on host for verification */
    long long host_sum = 0;
    for (int i = 0; i < n; i++) {
        host_sum += c[i];
    }
    
    printf("Final sum (device): %lld\n", sum);
    printf("Final sum (host): %lld\n", host_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    printf("Test completed successfully\n");
    return 0;
}
