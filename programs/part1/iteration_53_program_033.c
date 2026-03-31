/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create data dependencies */
int unpredictable(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

int main(void) {
    const int N = 100;
    int i, j, k;
    int result = 0;
    volatile int sink;
    
    /* Initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    printf("Testing ivdep pragma (no-vector)...\n");
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i] + a[unpredictable(i) % N];
    }
    sink = a[N-1];  /* Prevent dead code elimination */
    use(sink);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    printf("Testing vector pragma...\n");
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
    }
    sink = b[N/2];
    use(sink);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    printf("Testing parallel pragma...\n");
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent computation - suitable for parallelization */
        c[i] = a[i] * b[i];
        result += c[i];  /* Reduction, but pragma hints at parallel */
    }
    use(result);
    
    /* Reset result for next test */
    result = 0;
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    printf("Testing unroll pragma (maybe-infinite)...\n");
    int limit = 8;  /* Small enough for potential unrolling */
    #pragma GCC unroll
    for (i = 0; i < limit; i++) {
        result += a[i] * i;
    }
    use(result);
    
    /* Nested loop with pragma */
    printf("Testing nested loop with pragma...\n");
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            b[i*10 + j] = a[i*10 + j] + i * j;
        }
    }
    sink = b[50];
    use(sink);
    
    /* Loop in conditional context */
    printf("Testing loop in conditional context...\n");
    if (N > 50) {
        #pragma GCC vector
        for (i = 0; i < 20; i++) {
            a[i] = b[i] * 3;
        }
        sink = a[10];
        use(sink);
    }
    
    /* While loop with pragma */
    printf("Testing while loop with pragma...\n");
    i = 0;
    #pragma GCC unroll
    while (i < 10) {
        result += i * 2;
        i++;
    }
    use(result);
    
    /* Complex loop with multiple pragmas in sequence */
    printf("Testing multiple pragmas in sequence...\n");
    
    #pragma GCC ivdep
    for (i = 1; i < N-1; i++) {
        a[i] = (a[i-1] + a[i] + a[i+1]) / 3;
    }
    
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * a[i];
    }
    
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Final computation and output */
    int final_sum = 0;
    for (i = 0; i < N; i++) {
        final_sum += c[i];
    }
    
    printf("Final result: %d\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return final_sum > 0 ? 0 : 1;
}
