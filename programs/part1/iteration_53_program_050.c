/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function with various annotated loops */
void process_loops(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int result = 0;
    
    /* 1. annot_expr_no_vector_kind - #pragma GCC ivdep */
    #pragma GCC ivdep
    for (int i = 1; i < n; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] * 2 + b[i];
        use(a[i]);
    }
    
    /* 2. annot_expr_vector_kind - #pragma GCC vector */
    #pragma GCC vector
    for (int i = 0; i < n; i++) {
        /* Simple vectorizable operation */
        c[i] = a[i] * 3 + b[i];
        use(c[i]);
    }
    
    /* 3. annot_expr_parallel_kind - #pragma GCC parallel */
    int partial_sums[4] = {0};
    #pragma GCC parallel
    for (int i = 0; i < n; i++) {
        /* Independent accumulation - suitable for parallel */
        partial_sums[i % 4] += c[i] * 2;
    }
    
    /* Use partial sums */
    for (int i = 0; i < 4; i++) {
        result += partial_sums[i];
        use(partial_sums[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind - #pragma GCC unroll */
    int unroll_count = (n > 8) ? 8 : n;
    int product = 1;
    #pragma GCC unroll
    for (int i = 0; i < unroll_count; i++) {
        /* Loop with variable bound - maybe infinite for unrolling */
        product *= (a[i] + 1);
        use(product);
    }
    
    result += product;
    use(result);
}

/* Another function with nested annotated loops */
void nested_annotated_loops(int n, int m, int *restrict arr) {
    volatile int sum = 0;
    
    /* Outer loop with ivdep */
    #pragma GCC ivdep
    for (int i = 1; i < n; i++) {
        /* Inner loop with vector pragma */
        #pragma GCC vector
        for (int j = 0; j < m; j++) {
            arr[i * m + j] = arr[(i-1) * m + j] + i * j;
            sum += arr[i * m + j];
        }
        use(sum);
    }
}

/* Function with conditional annotated loops */
void conditional_annotated_loop(int flag, int n, int *restrict data) {
    if (flag > 0) {
        /* Loop with parallel pragma inside conditional */
        #pragma GCC parallel
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * data[i] + i;
            use(data[i]);
        }
    } else {
        /* Loop with unroll pragma in else branch */
        int limit = (n > 16) ? 16 : n;
        #pragma GCC unroll
        for (int i = 0; i < limit; i++) {
            data[i] = data[i] / 2;
            use(data[i]);
        }
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *arr = (int*)malloc(N * M * sizeof(int));
    
    if (!a || !b || !c || !arr) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
        c[i] = 0;
    }
    
    for (int i = 0; i < N * M; i++) {
        arr[i] = i % 100;
    }
    
    /* Execute all functions with annotated loops */
    process_loops(N, a, b, c);
    nested_annotated_loops(20, M, arr);
    conditional_annotated_loop(1, N, a);
    conditional_annotated_loop(0, N, b);
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    
    for (int i = 0; i < N * M; i += 97) {
        checksum += arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
