/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    /* Empty but prevents dead code elimination */
    static volatile int sink;
    sink = val;
}

/* Function with various pragma-annotated loops */
void process_loops(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int vsink;
    int i, j;
    
    /* 1. annot_expr_no_vector_kind - #pragma GCC ivdep */
    #pragma GCC ivdep
    for (i = 0; i < n; i++) {
        /* Potentially dependent access pattern */
        if (i > 0) {
            a[i] = a[i-1] + b[i];
        } else {
            a[i] = b[i];
        }
        vsink = a[i]; /* Prevent optimization */
    }
    
    /* 2. annot_expr_vector_kind - #pragma GCC vector */
    #pragma GCC vector
    for (i = 0; i < n; i++) {
        /* Simple vectorizable computation */
        c[i] = a[i] * 2 + b[i];
        vsink = c[i];
    }
    
    /* 3. annot_expr_parallel_kind - #pragma GCC parallel */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (i = 0; i < n; i++) {
        /* Independent accumulation into thread-local buckets */
        partial_sums[i % 4] += a[i] * b[i];
        vsink = partial_sums[i % 4];
    }
    
    /* 4. annot_expr_maybe_infinite_kind - #pragma GCC unroll */
    int unroll_bound = (n > 8) ? 8 : n;
    int product = 1;
    #pragma GCC unroll
    for (i = 0; i < unroll_bound; i++) {
        product *= (a[i] + 1);
        vsink = product;
    }
    
    /* Nested loop with pragma */
    #pragma GCC ivdep
    for (i = 0; i < n; i++) {
        for (j = 0; j < 4; j++) {
            a[i] += j;
            vsink = a[i];
        }
    }
}

/* Another function with pragmas in different control flow contexts */
void conditional_pragmas(int n, int *arr) {
    volatile int sink;
    
    if (n > 10) {
        /* Pragma inside conditional */
        #pragma GCC vector
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 3;
            sink = arr[i];
        }
    } else {
        /* Different pragma in else branch */
        #pragma GCC ivdep
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] / 2;
            sink = arr[i];
        }
    }
    
    /* While loop with pragma */
    int count = n;
    #pragma GCC unroll
    while (count-- > 0) {
        arr[count] += count;
        sink = arr[count];
    }
}

int main(void) {
    const int N = 100;
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(N * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    /* Execute functions with pragma-annotated loops */
    process_loops(N, a, b, c);
    conditional_pragmas(N, a);
    
    /* Compute checksum to verify execution */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i];
        use(a[i]); /* Prevent optimization */
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
