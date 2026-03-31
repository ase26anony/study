/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation */
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
        
        /* Additional operation to create more edges in the DDG */
        c[i] = (b[i-1] << 2) + i;  /* Anti-dependence on b[i-1] */
        
        /* Output dependence: d is written multiple times in same iteration */
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
        int t1 = acc + y[i];      /* Flow dep on acc from previous iteration */
        int t2 = t1 * z[i];       /* Flow dep on t1 */
        int t3 = t2 - x[i-1];     /* Flow dep on t2, anti-dep on x[i-1] */
        acc = t3 >> 1;            /* Flow dep on t3, output dep on acc */
        x[i] = acc;               /* Flow dep on acc */
        
        /* Cross-iteration anti-dependence */
        y[i] = z[i-1] + i;        /* Anti-dep on z[i-1] */
        
        /* Independent operation to create parallel paths */
        z[i] = (i * 37) & 0xFF;
    }
    
    return acc + x[n-1];
}

int main(void) {
    int i;
    
    /* Allocate arrays with restrict to help alias analysis */
    int* restrict a = (int*)malloc(N * sizeof(int));
    int* restrict b = (int*)malloc(N * sizeof(int));
    int* restrict c = (int*)malloc(N * sizeof(int));
    int* restrict d = (int*)malloc(N * sizeof(int));
    
    int* restrict x = (int*)malloc(N * sizeof(int));
    int* restrict y = (int*)malloc(N * sizeof(int));
    int* restrict z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1001;
        c[i] = (i * 71) % 1001;
        d[i] = (i * 97) % 1001;
        
        x[i] = (i * 131) % 1001;
        y[i] = (i * 173) % 1001;
        z[i] = (i * 199) % 1001;
    }
    
    /* Call loops multiple times to ensure execution */
    int sum1 = 0, sum2 = 0;
    for (int iter = 0; iter < 10; ++iter) {
        sum1 += compute_loop(a, b, c, d, N);
        sum2 += compute_loop2(x, y, z, N);
        
        /* Modify inputs slightly for next iteration */
        a[0] += iter;
        x[0] += iter;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1: %d\n", sum1);
    printf("Checksum 2: %d\n", sum2);
    printf("Total: %d\n", sum1 + sum2);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
