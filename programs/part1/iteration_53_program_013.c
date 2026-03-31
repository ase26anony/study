/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);

/* Volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function to create loops with various pragmas */
void test_loops(int n, int *arr1, int *arr2, int *arr3) {
    int i, j;
    
    /* 1. Loop with no-vector annotation (ivdep pragma) */
    #pragma GCC ivdep
    for (i = 0; i < n; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        arr1[i] = arr1[i] + arr1[i-1];
        sink = arr1[i]; /* Prevent optimization */
    }
    
    /* 2. Loop with vector annotation */
    #pragma GCC vector
    for (i = 0; i < n; i++) {
        /* Simple vectorizable computation */
        arr2[i] = arr1[i] * 2 + 3;
        sink = arr2[i];
    }
    
    /* 3. Loop with parallel annotation */
    #pragma GCC parallel
    for (i = 0; i < n; i++) {
        /* Independent computation suitable for parallelization */
        int local_sum = 0;
        for (j = 0; j < 10; j++) {
            local_sum += arr1[i] * j;
        }
        arr3[i] = local_sum;
        sink = arr3[i];
    }
    
    /* 4. Loop with maybe-infinite annotation (unroll pragma) */
    #pragma GCC unroll
    for (i = 0; i < 8; i++) {  /* Small constant bound */
        /* Computation that could be unrolled */
        arr1[i] = arr1[i] * arr1[i];
        sink = arr1[i];
    }
}

/* Another function with nested loops and pragmas in different contexts */
void test_mixed_contexts(int n, int *a, int *b) {
    int i, j;
    
    /* Loop with ivdep inside conditional */
    if (n > 0) {
        #pragma GCC ivdep
        for (i = 1; i < n; i++) {
            a[i] = a[i-1] + b[i];
            use(a[i]);
        }
    }
    
    /* Loop with vector pragma followed by other statements */
    #pragma GCC vector
    for (i = 0; i < n; i++) {
        b[i] = a[i] * 3;
        use(b[i]);
    }
    
    /* Nested loops with parallel pragma on outer loop */
    #pragma GCC parallel
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            a[i] += b[j];
        }
        use(a[i]);
    }
    
    /* Loop with unroll pragma and variable bound */
    int m = n < 16 ? n : 16;
    #pragma GCC unroll
    for (i = 0; i < m; i++) {
        a[i] = a[i] * 2 + 1;
        use(a[i]);
    }
}

/* Dummy function definition */
void use(int val) {
    sink = val;
}

int main(void) {
    const int N = 100;
    int *arr1 = malloc(N * sizeof(int));
    int *arr2 = malloc(N * sizeof(int));
    int *arr3 = malloc(N * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = 0;
        arr3[i] = 0;
    }
    
    /* Test loops with various pragmas */
    test_loops(N, arr1, arr2, arr3);
    
    /* Test mixed contexts */
    test_mixed_contexts(50, arr1, arr2);
    
    /* Compute checksum to verify execution */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
