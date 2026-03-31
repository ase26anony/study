/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create side effects */
static int dummy = 0;
void side_effect(void) {
    dummy++;
}

int main(void) {
    const int N = 100;
    int i, j, k;
    int result = 0;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
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
        a[i] = a[i-1] + i;
        side_effect();
    }
    use(a[N-1]);
    
    /* 2. Loop with vector annotation */
    /* Hint that this loop should be vectorized */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        side_effect();
    }
    use(b[N/2]);
    
    /* 3. Loop with parallel annotation */
    /* Hint that loop iterations are independent */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent computation - could be parallelized */
        c[i] = b[i] * b[i];
        side_effect();
    }
    use(c[N-1]);
    
    /* 4. Loop with maybe-infinite annotation (unroll pragma without count) */
    /* This suggests the loop could be unrolled, potentially infinitely */
    int limit = 20;  /* Variable bound */
    int sum = 0;
    #pragma GCC unroll
    for (i = 0; i < limit; i++) {
        sum += a[i % N];
        side_effect();
    }
    use(sum);
    
    /* 5. Nested loop with annotation on inner loop */
    int matrix[10][10];
    for (i = 0; i < 10; i++) {
        #pragma GCC ivdep
        for (j = 0; j < 10; j++) {
            matrix[i][j] = i * j;
            side_effect();
        }
    }
    use(matrix[5][5]);
    
    /* 6. Annotated loop inside conditional */
    if (N > 50) {
        #pragma GCC vector
        for (i = 0; i < 50; i++) {
            a[i] += b[i];
            side_effect();
        }
        use(a[25]);
    }
    
    /* 7. While loop with annotation */
    i = 0;
    #pragma GCC unroll
    while (i < 10) {
        result += i * i;
        side_effect();
        i++;
    }
    use(result);
    
    /* Final computation using all results */
    int final_result = 0;
    for (i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    final_result += sum + result + matrix[5][5];
    
    printf("Final result: %d\n", final_result);
    printf("Dummy counter: %d\n", dummy);
    
    return 0;
}
