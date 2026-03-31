/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static volatile int sink;
void use(int val) { sink = val; }

int main(void) {
    const int N = 100;
    int i, j, k;
    int sum = 0;
    
    /* Initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. Loop with no-vector annotation (ivdep pragma) */
    /* This tells GCC to ignore assumed vector dependencies */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency broken by ivdep */
        a[i] = a[i-1] + a[i];
    }
    use(a[N-1]);  /* Prevent dead code elimination */
    
    /* 2. Loop with vector annotation */
    /* Hint that this loop is a good candidate for vectorization */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
    }
    use(b[N/2]);
    
    /* 3. Loop with parallel annotation */
    /* Hint that loop iterations are independent */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent accumulation */
        c[i] = a[i] + b[i];
    }
    use(c[N-1]);
    
    /* 4. Loop with maybe-infinite annotation (unroll pragma without count) */
    /* Inside conditional to test different control flow contexts */
    if (N > 10) {
        int prod = 1;
        int limit = 8;  /* Variable bound for unroll */
        
        #pragma GCC unroll
        for (i = 1; i <= limit; i++) {
            prod *= i;
        }
        use(prod);
        sum += prod;
    }
    
    /* 5. Nested loop with annotation on inner loop */
    int matrix[10][10];
    for (i = 0; i < 10; i++) {
        #pragma GCC ivdep
        for (j = 0; j < 10; j++) {
            matrix[i][j] = i * j;
        }
    }
    use(matrix[5][5]);
    
    /* 6. While loop with annotation */
    i = 0;
    #pragma GCC vector
    while (i < 20) {
        b[i] += i;
        i++;
    }
    use(b[10]);
    
    /* Calculate final checksum */
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Final checksum: %d\n", sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return (sum > 0) ? 0 : 1;
}
