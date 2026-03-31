/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int x) {
    volatile int sink = x;
    (void)sink;
}

/* External function to create side effects */
extern void external_side_effect(int);

int main(void) {
    const int N = 100;
    int i, j;
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
    
    /* 1. Loop with #pragma GCC ivdep for annot_expr_no_vector_kind */
    /* This pragma tells GCC to ignore vector dependencies */
    printf("Starting ivdep loop...\n");
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] + a[i];
        use(a[i]);  /* Prevent optimization */
    }
    
    /* 2. Loop with #pragma GCC vector for annot_expr_vector_kind */
    /* This pragma hints that vectorization is beneficial */
    printf("Starting vector loop...\n");
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);  /* Prevent optimization */
    }
    
    /* 3. Nested loop with #pragma GCC parallel for annot_expr_parallel_kind */
    /* This pragma hints that loop iterations are parallelizable */
    printf("Starting parallel loop...\n");
    #pragma GCC parallel
    for (i = 0; i < 10; i++) {
        int partial_sum = 0;
        for (j = 0; j < 10; j++) {
            /* Independent computation suitable for parallelization */
            partial_sum += a[i*10 + j] * b[i*10 + j];
        }
        c[i] = partial_sum;
        use(c[i]);  /* Prevent optimization */
    }
    
    /* 4. Loop with #pragma GCC unroll for annot_expr_maybe_infinite_kind */
    /* Without count, implies potential infinite unrolling */
    printf("Starting unroll loop...\n");
    int unroll_count = 8;  /* Variable to prevent compile-time unrolling */
    #pragma GCC unroll
    for (i = 0; i < unroll_count; i++) {
        product *= (a[i] + 1);
        use(product);  /* Prevent optimization */
    }
    
    /* 5. Mixed: Loop with pragma inside conditional block */
    printf("Starting conditional loop...\n");
    if (N > 50) {
        #pragma GCC ivdep
        for (i = 0; i < 20; i++) {
            a[i] = a[i] * 3 - 2;
            external_side_effect(a[i]);  /* External call prevents optimization */
        }
    }
    
    /* 6. Another vector loop with different computation pattern */
    printf("Starting second vector loop...\n");
    #pragma GCC vector
    for (i = 0; i < N; i += 2) {
        b[i] = a[i] * a[i+1];
        use(b[i]);
    }
    
    /* Calculate final result using all computed values */
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i];
        if (i < 10) sum += c[i];
    }
    sum += product;
    
    printf("Final checksum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}

/* Define the external function to avoid linkage errors */
void external_side_effect(int x) {
    volatile static int storage = 0;
    storage += x;
    (void)storage;
}
