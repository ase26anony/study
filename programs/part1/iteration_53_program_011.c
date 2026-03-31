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
int maybe_dependent(int i, int j) {
    return (i * 1103515245 + 12345) % j;
}

int main(void) {
    const int N = 100;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    volatile int sink;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. Loop with #pragma GCC ivdep for annot_expr_no_vector_kind */
    /* This pragma tells GCC to ignore vector dependencies */
    #pragma GCC ivdep
    for (int i = 1; i < N; i++) {
        /* Potential dependency broken by ivdep */
        a[i] = a[i-1] + a[i] + maybe_dependent(i, 7);
    }
    sink = a[N-1];
    use(sink);
    
    /* 2. Loop with #pragma GCC vector for annot_expr_vector_kind */
    /* This pragma hints that vectorization is beneficial */
    #pragma GCC vector
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2 + i;
    }
    sink = b[N/2];
    use(sink);
    
    /* 3. Loop with #pragma GCC parallel for annot_expr_parallel_kind */
    /* This pragma hints that loop iterations are independent */
    #pragma GCC parallel
    for (int i = 0; i < N; i++) {
        /* Independent computation - could be parallelized */
        c[i] = a[i] + b[i] * 3;
        sum += c[i];  /* Reduction, but pragma suggests parallelizable */
    }
    use(sum);
    
    /* 4. Nested loops with mixed pragmas */
    {
        int temp[N];
        #pragma GCC ivdep
        for (int i = 0; i < N; i++) {
            temp[i] = 0;
            #pragma GCC vector
            for (int j = 0; j < 10; j++) {
                temp[i] += a[(i + j) % N] * j;
            }
        }
        sink = temp[N-1];
        use(sink);
    }
    
    /* 5. Loop with #pragma GCC unroll for annot_expr_maybe_infinite_kind */
    /* No count specified - implies "maybe infinite" */
    {
        int count = 8;  /* Variable to prevent compile-time unrolling */
        #pragma GCC unroll
        for (int i = 0; i < count; i++) {
            prod *= (a[i % N] + 1);
            /* Call to prevent dead code elimination */
            use(prod);
        }
    }
    
    /* 6. Loop in conditional context with parallel pragma */
    if (sum > 0) {
        #pragma GCC parallel
        for (int i = 0; i < N/2; i++) {
            b[i] = c[i] * 2 - a[i];
            use(b[i]);
        }
    }
    
    /* 7. While loop with ivdep pragma */
    {
        int i = 0;
        #pragma GCC ivdep
        while (i < N) {
            a[i] = a[i] * a[i] % 1000;
            i += 1 + (i % 3);  /* Non-unit stride */
        }
        sink = a[0];
        use(sink);
    }
    
    /* Final computation and output */
    int final_result = sum + prod + sink;
    for (int i = 0; i < N; i++) {
        final_result += a[i] % 256;
        final_result += b[i] % 256;
        final_result += c[i] % 256;
    }
    
    printf("Result: %d\n", final_result);
    return 0;
}
