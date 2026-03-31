/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int* restrict a, int* restrict b, 
                 int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written here, read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more dependencies */
        c[i] = b[i-1] + i;      /* Anti-dependence on b[i-1] */
        d[i] = a[i] << 2;       /* Output dependence on a[i] within same iteration */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
int compute_loop2(int* restrict x, int* restrict y, 
                  int* restrict z, int n) {
    int i;
    
    /* Different loop with register-to-register dependencies */
    int acc = x[0];
    for (i = 1; i < n; ++i) {
        /* Multiple true dependencies creating a chain */
        acc = acc + y[i];
        x[i] = acc * z[i];
        y[i] = x[i-1] - y[i-1];  /* True and anti dependencies */
        z[i] = (z[i-1] << 1) | 1; /* True dependence */
    }
    
    return acc + x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i;
    int result1, result2;
    
    /* Dynamically allocate arrays to avoid stack overflow */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    int* d = (int*)malloc(N * sizeof(int));
    
    int* x = (int*)malloc(N * sizeof(int));
    int* y = (int*)malloc(N * sizeof(int));
    int* z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
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
        y[i] = (i * 103) % 971;
        z[i] = (i * 107) % 967;
    }
    
    /* Call the loops multiple times to ensure execution */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Mix results to prevent optimization */
    int final_result = result1 ^ result2;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Clean up */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
