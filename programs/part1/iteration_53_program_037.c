/* test-annotated-loops.c */
/* Compile with: gcc -O2 -ftree-vectorize -fdump-tree-all -c test-annotated-loops.c */

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
    int i, j;
    int sum = 0;
    int product = 1;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    int partial_sums[4] = {0}; /* For parallel simulation */
    
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
        /* Potential dependency that ivdep tells GCC to ignore */
        a[i] = a[i-1] + a[i] * 2;
    }
    use(a[N-1]); /* Prevent dead code elimination */
    
    /* 2. Loop with #pragma GCC vector for annot_expr_vector_kind */
    /* This pragma hints that vectorization is desirable */
    printf("Starting vector loop...\n");
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + 7;
    }
    use(b[N/2]);
    
    /* 3. Nested loop with #pragma GCC parallel for annot_expr_parallel_kind */
    /* This pragma hints that loop iterations are independent */
    printf("Starting parallel loop...\n");
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        int local_sum = 0;
        /* Independent inner loop */
        for (j = 0; j < 10; j++) {
            local_sum += a[i] + j;
        }
        c[i] = local_sum;
        /* Simulate thread-local accumulation */
        partial_sums[i % 4] += c[i];
    }
    
    /* Use partial sums to prevent elimination */
    for (i = 0; i < 4; i++) {
        external_side_effect(partial_sums[i]);
    }
    
    /* 4. Loop with #pragma GCC unroll for annot_expr_maybe_infinite_kind */
    /* Without a count, this suggests potentially infinite unrolling */
    printf("Starting unroll loop...\n");
    {
        int limit = 8; /* Variable to prevent compile-time unrolling */
        #pragma GCC unroll
        for (i = 0; i < limit; i++) {
            product *= (i + 2);
            /* Call external function to create side effect */
            external_side_effect(product);
        }
    }
    
    /* 5. Mixed: Annotated loop inside conditional */
    printf("Starting conditional annotated loop...\n");
    if (N > 50) {
        #pragma GCC ivdep
        for (i = 0; i < 20; i++) {
            b[i] += a[i] * 2;
        }
        use(b[10]);
    }
    
    /* 6. Another vector loop with different structure */
    printf("Starting second vector loop...\n");
    #pragma GCC vector
    for (i = 0; i < N; i += 2) {
        /* Non-unit stride to test printer */
        b[i] = a[i] * a[i+1];
    }
    use(b[30]);
    
    /* Final computation and output */
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Final checksum: sum = %d, product = %d\n", sum, product);
    
    /* Verify with a simple check */
    if (sum > 0 && product > 0) {
        printf("All loops executed successfully.\n");
    } else {
        printf("Unexpected results.\n");
    }
    
    return 0;
}

/* Define the external function to avoid linkage errors */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
