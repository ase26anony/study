/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int value);
void use(int value) {
    /* Empty but prevents dead code elimination */
    volatile int sink = value;
    (void)sink;
}

/* Function to create side effects */
volatile int global_sink;

int main(void) {
    const int N = 100;
    int i, j, k;
    int result = 0;
    
    /* Initialize arrays */
    int a[N], b[N], c[N];
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. Loop with no-vector annotation (ivdep pragma) */
    /* This tells GCC there are no loop-carried dependencies */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep overrides */
        a[i] = a[i] + a[i-1] * 0;  /* Actually no real dependency */
        use(a[i]);
    }
    
    /* 2. Loop with vector annotation */
    /* Hint that this loop should be vectorized */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);
    }
    
    /* 3. Loop with parallel annotation */
    /* Hint that loop iterations are independent */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent computation */
        c[i] = b[i] * b[i];
        use(c[i]);
    }
    
    /* 4. Loop with maybe-infinite annotation (unroll pragma without count) */
    /* Inside conditional to test different control flow contexts */
    if (N > 10) {
        int temp = 1;
        #pragma GCC unroll
        for (i = 0; i < 8; i++) {  /* Small constant bound */
            temp *= (i + 1);
            use(temp);
        }
        result += temp;
    }
    
    /* 5. Nested loop with annotation on inner loop */
    int matrix[10][10];
    for (i = 0; i < 10; i++) {
        #pragma GCC ivdep
        for (j = 0; j < 10; j++) {
            matrix[i][j] = i * j;
            use(matrix[i][j]);
        }
    }
    
    /* 6. Another maybe-infinite annotation with variable bound */
    int m = 5;  /* Could be variable */
    int prod = 1;
    #pragma GCC unroll
    for (k = 0; k < m; k++) {
        prod *= (k + 2);
        use(prod);
    }
    result += prod;
    
    /* Compute final result to ensure all loops execute */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Add matrix and product results */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            sum += matrix[i][j];
        }
    }
    sum += result;
    
    /* Print result to verify execution */
    printf("Result: %d\n", sum);
    
    /* Prevent optimization of final result */
    global_sink = sum;
    
    return (sum > 0) ? 0 : 1;
}
