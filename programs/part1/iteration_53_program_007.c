/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create side effects */
int dummy_side_effect = 0;

int main(void) {
    const int N = 100;
    int i, j, k;
    int result = 0;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    volatile int vol_sink;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. Loop with #pragma GCC ivdep for annot_expr_no_vector_kind */
    /* This pragma tells GCC to ignore vector dependencies */
    printf("Starting ivdep loop...\n");
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep overrides */
        a[i] = a[i-1] + i;
        use(a[i]);  /* Prevent dead code elimination */
    }
    vol_sink = a[N-1];
    
    /* 2. Loop with #pragma GCC vector for annot_expr_vector_kind */
    /* This pragma hints that vectorization is beneficial */
    printf("Starting vector loop...\n");
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);
    }
    vol_sink = b[N/2];
    
    /* 3. Nested loop with outer pragma for different context */
    printf("Starting nested loops...\n");
    for (j = 0; j < 10; j++) {
        /* Inner loop with #pragma GCC parallel for annot_expr_parallel_kind */
        /* This pragma hints that loop iterations are independent */
        #pragma GCC parallel
        for (i = 0; i < N; i++) {
            c[i] += a[i] + b[i] + j;
            /* Independent accumulation - safe for parallelization */
            dummy_side_effect += (c[i] & 1);
        }
    }
    vol_sink = c[N-1];
    
    /* 4. Loop with #pragma GCC unroll for annot_expr_maybe_infinite_kind */
    /* No count specified, so potentially infinite unrolling */
    printf("Starting unroll loop...\n");
    int limit = 20;  /* Variable limit to prevent compile-time unrolling */
    #pragma GCC unroll
    for (k = 0; k < limit; k++) {
        result += k * k;
        use(result);
        /* Conditional to prevent infinite loop optimization */
        if (k == limit - 1) break;
    }
    vol_sink = result;
    
    /* 5. Loop inside conditional block */
    printf("Starting conditional loop...\n");
    if (result > 0) {
        /* Mixed pragma usage in different control flow */
        #pragma GCC ivdep
        for (i = 0; i < N/2; i++) {
            a[i] = b[i] * c[i];
            use(a[i]);
        }
    }
    
    /* 6. While loop with pragma */
    printf("Starting while loop...\n");
    int counter = 10;
    #pragma GCC unroll
    while (counter-- > 0) {
        result += counter;
        use(result);
    }
    
    /* Final computation using all results */
    int final_sum = 0;
    for (i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i];
    }
    final_sum += result + dummy_side_effect;
    
    printf("Final result: %d\n", final_sum);
    printf("All annotated loops executed successfully.\n");
    
    return final_sum > 0 ? 0 : 1;
}
