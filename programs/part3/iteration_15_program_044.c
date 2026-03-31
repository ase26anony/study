/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent constant propagation */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int* restrict a, int* restrict b, 
                        int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operation to create more edges */
        c[i] = (b[i-1] << 2) + i;  /* Anti-dependence on b[i-1] */
        
        /* Output dependence: d is written twice in same iteration */
        d[i] = a[i] + b[i];
        d[i] = d[i] * 3;  /* Output dependence on d[i] */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int* restrict x, int* restrict y, 
                         int* restrict z, int n) {
    int i;
    
    /* Different loop with register-to-register dependencies */
    int acc = x[0];
    for (i = 1; i < n; ++i) {
        /* Chain of dependencies within iteration */
        int t1 = acc + y[i];      /* Flow dep on acc from prev iteration */
        int t2 = t1 * z[i];       /* Flow dep on t1 */
        int t3 = t2 - x[i-1];     /* Flow dep on t2, anti-dep on x[i-1] */
        acc = t3 >> 1;            /* Flow dep on t3 */
        x[i] = acc;               /* Output to memory */
        
        /* Create anti-dependence for next iteration */
        y[i] = x[i] + i;
    }
    
    return acc + x[n-1];
}

int main(void) {
    int i;
    int checksum1, checksum2;
    
    /* Use volatile for loop bound to prevent unrolling */
    volatile int loop_bound = N;
    int actual_n = loop_bound;
    
    /* Allocate and initialize arrays with deterministic values */
    int* a = (int*)malloc(actual_n * sizeof(int));
    int* b = (int*)malloc(actual_n * sizeof(int));
    int* c = (int*)malloc(actual_n * sizeof(int));
    int* d = (int*)malloc(actual_n * sizeof(int));
    
    int* x = (int*)malloc(actual_n * sizeof(int));
    int* y = (int*)malloc(actual_n * sizeof(int));
    int* z = (int*)malloc(actual_n * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (i = 0; i < actual_n; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        
        x[i] = (i * 101) % 977;
        y[i] = (i * 103) % 971;
        z[i] = (i * 107) % 967;
    }
    
    /* Execute loops to trigger DDG construction */
    checksum1 = compute_loop(a, b, c, d, actual_n);
    checksum2 = compute_loop2(x, y, z, actual_n);
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 2: %d\n", checksum2);
    printf("Total: %d\n", checksum1 + checksum2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
