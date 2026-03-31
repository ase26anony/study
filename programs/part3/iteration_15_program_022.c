/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int *restrict a, int *restrict b, 
                 int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple dependency types to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* TRUE (FLOW) DEPENDENCE: a[i] depends on a[i-1] (read-after-write across iterations) */
        /* This creates edges with type=TRUE_DEP, data_type=MEM_DEP */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* ANTI-DEPENDENCE: b[i] is written, then potentially read in next iteration */
        /* This creates edges with type=ANTI_DEP, data_type=MEM_DEP */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more edges */
        /* OUTPUT DEPENDENCE: c[i] written twice in same iteration */
        /* This creates edges with type=OUTPUT_DEP, data_type=MEM_DEP */
        c[i] = (c[i] << 2) | 1;
        c[i] = c[i] + i;  /* Output dependence on c[i] */
        
        /* REGISTER DEPENDENCE: Create true dependence through registers */
        /* This creates edges with type=TRUE_DEP, data_type=REG_DEP */
        int temp = d[i] * 3;
        d[i] = temp + a[i];  /* True dependence through 'temp' */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
int compute_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Loop with complex carried dependencies */
    for (i = 2; i < n; ++i) {
        /* Multiple interleaved dependencies */
        x[i] = x[i-1] + y[i-2];  /* True dependence on x[i-1], anti on y[i-2] */
        y[i] = x[i] * y[i-1];    /* True on x[i], anti on y[i-1] */
        
        /* Create register pressure and more dependencies */
        int r1 = x[i] << 1;
        int r2 = y[i] >> 2;
        x[i-1] = r1 + r2;        /* Anti-dependence on x[i-1] */
    }
    
    return x[n-1] * y[n-1];
}

int main(void) {
    int i, result1, result2;
    
    /* Dynamically allocate arrays to avoid stack overflow */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        x[i] = (i * 101) % 977;
        y[i] = (i * 113) % 971;
    }
    
    /* Call the loop functions multiple times to give compiler more chances */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, N);
    
    /* Mix results to prevent optimization */
    int final_result = result1 ^ result2;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
