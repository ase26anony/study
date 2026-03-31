/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* External function to create side effects */
extern void external_side_effect(int);

int main(void) {
    const int N = 100;
    int i, j, k;
    int sum = 0;
    int product = 1;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. Loop with no-vector annotation (ivdep pragma) */
    /* This tells GCC to ignore potential vector dependencies */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i] + a[i-1] * 2;
    }
    use(a[N-1]); /* Prevent dead code elimination */
    
    /* 2. Loop with vector annotation */
    /* Hint that this loop is a good candidate for vectorization */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + 7;
    }
    use(b[N/2]);
    
    /* 3. Loop with parallel annotation */
    /* Hint that loop iterations are independent and can be parallelized */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent computation - each iteration writes to different location */
        c[i] = b[i] * 2 - a[i];
        external_side_effect(c[i]); /* External call prevents optimization */
    }
    
    /* 4. Loop with maybe-infinite annotation (unroll pragma without count) */
    /* Suggests unrolling without specifying exact count */
    {
        int limit = 8; /* Variable limit to prevent complete unrolling at compile time */
        #pragma GCC unroll
        for (i = 0; i < limit; i++) {
            product *= (a[i] + 1);
        }
    }
    use(product);
    
    /* Nested loop with annotation */
    {
        int matrix[10][10];
        
        /* Initialize matrix */
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                matrix[i][j] = i * 10 + j;
            }
        }
        
        /* Outer loop with ivdep annotation */
        #pragma GCC ivdep
        for (i = 1; i < 9; i++) {
            #pragma GCC vector
            for (j = 1; j < 9; j++) {
                /* Stencil computation - has dependencies */
                matrix[i][j] = (matrix[i-1][j] + matrix[i+1][j] +
                              matrix[i][j-1] + matrix[i][j+1]) / 4;
            }
        }
        use(matrix[5][5]);
    }
    
    /* Conditional block with annotated loop */
    if (N > 50) {
        int temp[N];
        
        #pragma GCC parallel
        for (i = 0; i < N; i += 2) {
            temp[i] = a[i] * b[i];
        }
        use(temp[N-2]);
    }
    
    /* Compute final result from all loops */
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: sum = %d, product = %d\n", sum, product);
    
    /* Verify computations */
    if (sum > 0 && product > 0) {
        printf("All annotated loops executed successfully.\n");
    }
    
    return 0;
}

/* Definition of external function */
void external_side_effect(int x) {
    volatile int sink = x;
    (void)sink;
}
