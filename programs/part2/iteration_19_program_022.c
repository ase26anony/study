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

void test_gang_redundant(int n, int *a) {
    /* Case 0: gang redundant */
    #pragma acc parallel num_gangs(4) copyout(a[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            a[i] = i;
        }
    }
}

void test_gang_partitioned(int n, int *a, int *b) {
    /* Case 1: gang partitioned */
    #pragma acc parallel loop gang copy(a[0:n]) copyin(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * 2;
        }
    }
}

void test_worker_partitioned(int n, int *a) {
    /* Case 2: worker partitioned */
    #pragma acc parallel num_workers(4) copy(a[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            a[i] += 1;
        }
    }
}

void test_gang_worker_partitioned(int n, int *a, int *b) {
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyin(b[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            a[i] = b[i] + a[i];
        }
    }
}

void test_vector_partitioned(int n, int *a) {
    /* Case 4: vector partitioned */
    #pragma acc parallel loop vector vector_length(32) copy(a[0:n])
    {
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 3;
        }
    }
}

void test_gang_vector_partitioned(int n, int *a, int *b) {
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel loop gang vector vector_length(32) copy(a[0:n]) copyin(b[0:n])
    {
        for (int i = 0; i < n; i++) {
            a[i] = b[i] - a[i];
        }
    }
}

void test_worker_vector_partitioned(int n, int *a) {
    /* Case 6: worker+vector partitioned */
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

void test_fully_partitioned(int n, int *a, int *b, int *c) {
    /* Case 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(a[0:n]) copyin(b[0:n], c[0:n])
    {
        #pragma acc loop gang worker vector vector_length(32)
        for (int i = 0; i < n; i++) {
            a[i] = b[i] + c[i] + a[i];
        }
    }
}

int verify_result(int n, int *a, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    printf("Verification sum: %d (expected: %d)\n", sum, expected_sum);
    return sum == expected_sum;
}

int main(int argc, char *argv[]) {
    int n = N_DEFAULT;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            n = N_DEFAULT;
        }
    }
    
    printf("Testing OpenACC partition types with n = %d\n", n);
    
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
        a[i] = 0;
        b[i] = i;
        c[i] = i * 2;
    }
    
    /* Test all partition types sequentially */
    printf("1. Testing gang redundant...\n");
    test_gang_redundant(n, a);
    
    printf("2. Testing gang partitioned...\n");
    test_gang_partitioned(n, a, b);
    
    printf("3. Testing worker partitioned...\n");
    test_worker_partitioned(n, a);
    
    printf("4. Testing gang+worker partitioned...\n");
    test_gang_worker_partitioned(n, a, b);
    
    printf("5. Testing vector partitioned...\n");
    test_vector_partitioned(n, a);
    
    printf("6. Testing gang+vector partitioned...\n");
    test_gang_vector_partitioned(n, a, b);
    
    printf("7. Testing worker+vector partitioned...\n");
    test_worker_vector_partitioned(n, a);
    
    printf("8. Testing fully partitioned...\n");
    test_fully_partitioned(n, a, b, c);
    
    /* Final verification */
    int expected_final = VERIFY_VALUE;
    if (n != N_DEFAULT) {
        /* Recalculate expected sum for different n */
        expected_final = 0;
        for (int i = 0; i < n; i++) {
            /* This matches the transformations applied in the tests */
            int val = i;                     /* test_gang_redundant */
            val = i * 2;                     /* test_gang_partitioned */
            val += 1;                        /* test_worker_partitioned */
            val = i + val;                   /* test_gang_worker_partitioned */
            val = val * 3;                   /* test_vector_partitioned */
            val = i - val;                   /* test_gang_vector_partitioned */
            val += (i % 4);                  /* test_worker_vector_partitioned */
            val = i + (i * 2) + val;         /* test_fully_partitioned */
            expected_final += val;
        }
    }
    
    int success = verify_result(n, a, expected_final);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    if (success) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Verification failed!\n");
        return 1;
    }
}
